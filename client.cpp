#include "client.h"
//#include <QOve
#include <QJSEngine>
#include <QJSValueList>

#include <QDebug>

namespace SocketIO {


Client::Client(int eioVersion, QObject *parent) :
    QObject(parent)
{
    _engineIoVerion = eioVersion;
    _state = Disconnected;
    _ws = new QWebSocket();
    _accessManager = new QNetworkAccessManager(this);

    _pulseTimer = new QTimer(this);
    _pulseTimeout = 20000;
    _pulseTimer->setInterval(_pulseTimeout);
    _pulseTimer->stop();

    _timeoutTimer = new QTimer(this);
    _connectionTimeout = 30000;
    _timeoutTimer->setInterval(_connectionTimeout);
    _timeoutTimer->stop();

    qRegisterMetaType<QAbstractSocket::SocketError>();
    qRegisterMetaType<QAbstractSocket::SocketState>();


    connect(_accessManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(handshakeReplyFinished(QNetworkReply *)));

    connect(_ws,QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),this,&Client::onSocketError);
    connect(_ws,SIGNAL(textMessageReceived(QString)),this, SLOT(onTextMessage(QString)));
    connect(_ws,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onWsStateChanged(QAbstractSocket::SocketState)));
    connect(_ws,SIGNAL(connected()),this,SLOT(onWsConnected()));
    connect(_ws,SIGNAL(disconnected()),this,SLOT(onWsDisconnected()));
    connect(_ws,SIGNAL(binaryMessageReceived(QByteArray)),this,SLOT(onBinaryMessage(QByteArray)));

    connect(_timeoutTimer,&QTimer::timeout,this,&Client::connectionTimedOut);
    connect(_pulseTimer,SIGNAL(timeout()),this,SLOT(sendPulse()));

    of("/");
}

Client::~Client()
{
    foreach(QString nsp, _nsps.keys())
        _nsps.value(nsp)->deleteLater();
    _pulseTimer->stop();
    _pulseTimer->deleteLater();
    _accessManager->deleteLater();
    _ws->deleteLater();
}

QString Client::sessionID()
{
    return _sessionID;
}

QString Client::getState()
{
    switch (_state) {
    case State::Opened: return "opened";
    case State::Opening: return "opening";
    case State::Connected: return "connected";
    case State::Connecting: return "connecting";
    case State::Disconnecting: return "disconnecting";
    default: return "disconnected";
    }
}

void Client::on(QString ev, QJSValue cb)
{
    of("/")->on(ev,cb);
}

void Client::off(QString ev)
{
    of("/")->off(ev);
}

void Client::once(QString ev, QJSValue cb)
{
    of("/")->once(ev,cb);
}

void Client::eemit(QString ev, QJSValue data)
{
    of("/")->eemit(ev,data);
}

Nsp *Client::of(QString name)
{
    Nsp * ret = _nsps.value(name,nullptr);
    if (ret)
        return ret;
    ret = new Nsp(name,this);
    _nsps.insert(name,ret);
    return ret;
}

void Client::open(QUrl url)
{
    _url = url;
    QUrl requestUrl = handshakeUrl();
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html"));
    request.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("*/*"));
    request.setRawHeader(QByteArrayLiteral("Connection"), QByteArrayLiteral("close"));
    _accessManager->get(request);
    _state = Opening;
    emit stateChanged(_state);
}

void Client::close()
{
    _ws->close();
    _state = Disconnected;
    emit stateChanged(_state);
    emit disconnected();
}

void Client::restart()
{
    qDebug() << "restarting";
    close();
    open(_url);
}

void Client::onSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "WebSocket error! " << error;
    restart();
}

void Client::onWsConnected()
{
//    qDebug() << "Websocket disconnected!";
    _state = Connecting;
    emit stateChanged(_state);
    sendPulse("probe");
}

void Client::onWsDisconnected()
{
//    qDebug() << "Websocket disconnected!";
    _state = Disconnected;
    emit stateChanged(_state);
    emit disconnected();
    _pulseTimer->stop();
    _timeoutTimer->stop();
}

void Client::onWsStateChanged(QAbstractSocket::SocketState st)
{
    qDebug() << "Websocket state changed!" << st;

}

void Client::onTextMessage(QString textMessage)
{
//    qDebug() << "Websocket text message!!!" << textMessage;
    Packet * p = _protocol.decrypt(textMessage);
    if (p->frameType()==FrameType::Pong&&p->defaultNsp())
    {
        if (p->rawData()=="probe")
        {
            //probe success message, send upgrade frame
            sendUpgrade();
            return;
        }
        resetTimoutTimer();
    }

    if (p->frameType()==FrameType::Message&&p->type()==PacketType::Connect&&p->defaultNsp())
    {
        _state = Connected;
        emit stateChanged(_state);
        emit connected();
        resetPulseTimer();
    }
    Nsp * nsp = of(p->nsp());
    nsp->inPacket(p);
}

void Client::onBinaryMessage(QByteArray msg)
{
    qDebug() << "Websocket binary message!!!" << msg;

}

void Client::sendPulse(QString rawData)
{
//    qDebug() << "send pulse";
    Packet p(FrameType::Ping);
    if (rawData!="")
        p.setRawData(rawData);
    sendPacket(&p);
}

void Client::sendUpgrade()
{
//    qDebug() << "send upgrade";
    Packet p(FrameType::Upgrade);
    sendPacket(&p);
}

void Client::sendConnect(QString nsp)
{
    if (nsp==""||nsp=="/")
        return;
    Packet p(FrameType::Message);
    p.setNsp(nsp);
    p.setType(PacketType::Connect);
    sendPacket(&p);
}

void Client::sendPacket(Packet *p)
{
    if (_state!=Connected&&_state!=Connecting)
    {
        p->deleteLater();
        return;
    }
    if (p->isBinary())
        _ws->sendBinaryMessage(_protocol.encryptBinary(p));
    else
        _ws->sendTextMessage(_protocol.encryptString(p));
    p->deleteLater();
}

void Client::handshakeReplyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
        qDebug() << "Error[" << status << "] " << reply->readAll();
        return;
    }
    QByteArray ba = reply->readAll();
    HandShake hs;
    if (!_protocol.decryptHandShake(ba,&hs))
    {
        qDebug() << "Error[handshake decryption] ";
        return;
    }

    qDebug() << "handshake! sid:" << hs.sid << "timeout:" << hs.pingTimeout << " interval: " << hs.pingInterval;
    _sessionID = hs.sid;
    _connectionTimeout = hs.pingInterval;
    _pulseTimeout = hs.pingInterval;

    _state = Opened;
    emit stateChanged(_state);
    _ws->open(wsUrl());

}


QUrl Client::handshakeUrl()
{
    QString scheme = "http";
    if (_url.scheme()=="https")
        scheme = "https";
    QString dt = QString::number(QDateTime::currentMSecsSinceEpoch());
    QUrl ret(QString("%1://%2:%3/socket.io/?EIO=%4&transport=polling&t=%5")
        .arg(scheme)
        .arg(_url.host())
        .arg(QString::number(_url.port(80)))
        .arg(QString::number(_engineIoVerion))
        .arg(dt));
//    .arg(QString(dt.toUtf8().toBase64())));
    qDebug() << "HS url: " << ret;
    return ret;
}

QUrl Client::wsUrl()
{
    QUrl ret(QString("%1://%2:%3/socket.io/?EIO=%4&transport=websocket&sid=%5")
       .arg(_url.scheme()=="https"?"wss":"ws")
       .arg(_url.host())
       .arg(QString::number(_url.port(80)))
       .arg(QString::number(_engineIoVerion))
       .arg(_sessionID));
    qDebug() << "WS url: " << ret;
    return ret;
}

void Client::resetPulseTimer()
{
    _pulseTimer->stop();
    _pulseTimer->setInterval(_pulseTimeout/2);
    _pulseTimer->start();
}

void Client::resetTimoutTimer()
{
    _timeoutTimer->stop();
    _timeoutTimer->setInterval(_connectionTimeout);
    _timeoutTimer->start();
}

void Client::connectionTimedOut()
{
    qDebug() << "connection timed out";
    restart();
}

}

#ifndef SIO_CLIENT_H
#define SIO_CLIENT_H

#include <QObject>
#include <QUrl>
#include <QJsonDocument>
#include <QWebSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QJSValue>
#include <QMap>

#include "protocol.h"
#include "nsp.h"

namespace SocketIO {


class Client : public QObject
{
    friend class Nsp;
    Q_OBJECT
public:
    enum State {
        Disconnected = 0,
        Opening,
        Opened,
        Connecting,
        Connected,
        Disconnecting
    };

    explicit Client(int eioVersion = 4, QObject *parent = nullptr);
    ~Client();

    QString sessionID();

    Q_PROPERTY(QString state READ getState)
    QString getState();

    Q_INVOKABLE void on(QString ev, QJSValue cb);
    Q_INVOKABLE void off(QString ev);
    Q_INVOKABLE void once(QString ev, QJSValue cb);
    Q_INVOKABLE void eemit(QString ev, QJSValue data);
    Q_INVOKABLE Nsp * of(QString name);
signals:
    void stateChanged(State);
    void connected();
    void disconnected();

public slots:
    void open(QUrl url);
    void close();
    void restart();
private slots:
    void handshakeReplyFinished(QNetworkReply *reply);
    void onSocketError(QAbstractSocket::SocketError error);

    void onWsConnected();
    void onWsDisconnected();
    void onWsStateChanged(QAbstractSocket::SocketState st);

    void onTextMessage(QString textMessage);
    void onBinaryMessage(QByteArray msg);

    void sendPulse(QString rawData = "");
    void sendUpgrade();
    void sendConnect(QString nsp="/");
    void sendPacket(Packet * p);
//    void parseTextMessage(QString msg);
private:
    QUrl handshakeUrl();
    QUrl wsUrl();

    void resetPulseTimer();
    void resetTimoutTimer();
    void connectionTimedOut();
private:
    State _state;

    QUrl    _url;
    QString _sessionID;

    int _engineIoVerion;

    QWebSocket * _ws;
    QNetworkAccessManager * _accessManager;

    Protocol _protocol;

    qint32 _connectionTimeout;
    qint32 _pulseTimeout;
    QTimer * _pulseTimer;
    QTimer * _timeoutTimer;

    QMap<QString,Nsp*> _nsps;
};

}


#endif // SOCKETIOCLIENT_H

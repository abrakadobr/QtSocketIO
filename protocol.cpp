#include "protocol.h"
#include <QDebug>
#include <QJsonDocument>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>

namespace SocketIO {

Protocol::Protocol(QObject *parent) : QObject(parent)
{
}

QByteArray Protocol::encryptBinary(Packet *p)
{
//    QVariant data = p->data();
    QByteArray payload;
    qDebug() << "encrypt binary" << p->data();
    /*
    if (data.type()==QVariant::Int)
        payload.append(QString::number(data.toInt()));
    if (data.type()==QVariant::String)
        payload.append(data.toString().toUtf8());
    if (data.type()==QVariant::Map||data.type()==QVariant::List)
        payload = QJsonDocument::fromVariant(p->data()).toJson();
    qDebug() << "payload1: " << payload;
    if (p->nsp()!=""&&p->nsp()!="/")
    {
        payload.prepend(',');
        payload.prepend(p->nsp().toUtf8());
    }
    if (p->frameType()==FrameType::Message)
        payload.prepend(QString::number(p->type()).toUtf8());
    payload.prepend(QString::number(p->frameType()).toUtf8());
    qDebug() << "encoding " << payload << " l:" << payload.length() << " nsp: " << p->nsp();
    */
    return payload;
}

QString Protocol::encryptString(Packet *p)
{
    QVariant data = p->data();
    QString payload;
    payload.append(QString::number(p->frameType()));
    if (p->frameType()==FrameType::Message)
        payload.append(QString::number(p->type()));
    if (!p->defaultNsp())
    {
        payload.append(p->nsp());
        payload.append(",");
    }
    if (p->rawData().length())
    {
        payload.append(p->rawData());
    } else {
        if (p->data().isValid()&&!p->data().isNull())
        {
//            QJsonDocument jdoc = QJsonDocument::fromVariant(p->data());
//            jdoc.to
            payload.append(QString::fromUtf8(QJsonDocument::fromVariant(p->data()).toJson(QJsonDocument::Compact)));
        }
    }
    qDebug() << "str packed encrypted: " << payload;
    return payload;
}



Packet *Protocol::decrypt(QByteArray src)
{
    qDebug() << "decrypt packet " << src;
    if (src.at(0)=='b')
        return decryptBinary(src);
    return decryptString(QString::fromUtf8(src));
}

Packet *Protocol::decrypt(QString src)
{
    return decryptString(src);
}

bool Protocol::decryptHandShake(QByteArray src, HandShake *hs)
{
    QString srcs = QString::fromUtf8(src);
    size_t posDelimiter = srcs.indexOf(':');
    if (posDelimiter<=0)
        return false;
    uint len = srcs.mid(0,posDelimiter).toInt();
    if (srcs.length()-posDelimiter-1!=len)
        return false;
    QString jsonPayload = srcs.mid(posDelimiter+2);
    QJsonDocument doc = QJsonDocument::fromJson(jsonPayload.toUtf8());
    QVariantMap mp = doc.toVariant().toMap();
    if (mp.isEmpty())
        return false;
    QStringList keys = mp.keys();
    if (!keys.length()||!keys.contains("sid")||!keys.contains("pingInterval")||!keys.contains("pingTimeout"))
        return false;
    hs->sid = mp.value("sid","").toString();
    hs->pingTimeout = mp.value("pingTimeout",5000).toInt();
    hs->pingInterval = mp.value("pingInterval",20000).toInt();
    return true;
}

Packet *Protocol::decryptString(QString src)
{
    Packet * ret = new Packet(this);
    int pos = 0;
    FrameType ft = (FrameType)QString(src.at(pos)).toInt();
    ret->setFrameType(ft);
    pos++;
    if (ft!=Message)
    {
        if (src.length()>pos)
            ret->setRawData(src.mid(pos));
    } else {
        PacketType pt = (PacketType)QString(src.at(pos)).toInt();
        ret->setType(pt);
        pos++;
        if (src.length()>pos)
        {
            if (src.at(pos)=='/')
            {
                //have nsp
                int comma = src.indexOf(',',pos);
                if (comma>pos)
                {
                    ret->setNsp(src.mid(pos,comma));
                    pos = comma;
                }
            }
        }
        if (src.length()>pos)
            ret->setRawData(src.mid(pos));
    }
    qDebug() << "string packet decrypted: " << src;
    return ret;
}

Packet *Protocol::decryptBinary(QByteArray src)
{
    qDebug() << "decrypt binary " << src;
    return errorPacket("binary decrypt not implemented");
}

Packet *Protocol::errorPacket(QString error)
{
    Packet * ret = new Packet(this);
    ret->setType(PacketType::Error);
    ret->setFrameType(FrameType::Message);
    ret->setRawData(error);
    return ret;
}

}

#include "packet.h"

#include <QDebug>
#include <QJsonDocument>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>

namespace SocketIO {

Packet::Packet(QObject *parent) : QObject(parent)
{
    _nsp = "/";
    _frameType = Message;
    _dataType = Event;
}

Packet::Packet(FrameType ftype, QObject *parent):
    QObject(parent),
    _frameType(ftype)
{
    _nsp = "/";
    _dataType = PacketType::UnknownData;
}

QString Packet::rawData()
{
    return _rawData;
}

IOEvent Packet::eventData()
{
    IOEvent ret;
    QVariant data = QJsonDocument::fromJson(_rawData.toUtf8()).toVariant();
    if (data.type()!=QVariant::List)
        return ret;
    QVariantList lst = data.toList();
    if (!lst.length())
        return ret;
    ret.code = lst.at(0).toString();
    if (lst.length()>1)
        ret.data = lst.at(1);
    return ret;
}

void Packet::setEventData(IOEvent data)
{
    setEventData(data.code,data.data);
}

void Packet::setEventData(QString ev, QVariant args)
{
    QJsonDocument doc;
    QVariantList lst;
    lst << ev << args;
    doc = QJsonDocument::fromVariant(lst);
    _rawData = doc.toJson(QJsonDocument::Compact);
}

void Packet::dbg()
{
    qDebug() << "P: ft:" << _frameType << " dt:" << _dataType << " raw: " << _rawData;
}

}

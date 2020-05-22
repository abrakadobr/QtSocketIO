#include "nsp.h"
#include "client.h"
#include "packet.h"

#include <QJSEngine>

namespace SocketIO {

Nsp::Nsp(QString name, Client *parent) :
    QObject(parent),
    _client(parent),
    _name(name)
{

}

void Nsp::on(QString ev, QJSValue cb)
{
    if (!cb.isCallable())
        return;
    _ioOnMap.insertMulti(ev,cb);
}

void Nsp::off(QString ev)
{
    _ioOnMap.remove(ev);
    _ioOnceMap.remove(ev);
}

void Nsp::once(QString ev, QJSValue cb)
{
    if (!cb.isCallable())
        return;
    _ioOnceMap.insertMulti(ev,cb);
}

void Nsp::eemit(QString ev, QJSValue data)
{
    Packet * p = new Packet(FrameType::Message,_client);
    p->setType(PacketType::Event);
    p->setEventData(ev,data.toVariant());
    _client->sendPacket(p);
}

void Nsp::inPacket(Packet *p)
{
    if (p->frameType()==FrameType::Open)
    {
        callEvent("connected",_name);
    }
    if (p->frameType()==FrameType::Close)
    {
        callEvent("disconnected",_name);
    }
    if (p->frameType()==FrameType::Ping)
    {
        callEvent("ping");
    }
    if (p->frameType()==FrameType::Pong)
    {

    }
    if (p->frameType()==FrameType::Message)
    {

    }
}

void Nsp::callEvent(QString ev, QVariant data)
{
    foreach(QJSValue cb, _ioOnceMap.values(ev))
    {
        QJSValueList args;
        QVariantList lst;
        if (data.type()!=QVariant::Invalid)
        {
            if (data.type()==QVariant::List)
                lst = data.toList();
            else
                lst << data;
        }
        foreach(QVariant v,lst)
            args << cb.engine()->toScriptValue(v);
        cb.call(args);
    }
    _ioOnceMap.remove(ev);
    foreach(QJSValue cb, _ioOnMap.values(ev))
    {
        QJSValueList args;
        QVariantList lst;
        if (data.type()!=QVariant::Invalid)
        {
            if (data.type()==QVariant::List)
                lst = data.toList();
            else
                lst << data;
        }
        foreach(QVariant v,lst)
            args << cb.engine()->toScriptValue(v);
        cb.call(args);
    }
}

}

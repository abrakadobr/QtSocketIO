#ifndef SIO_NSP_H
#define SIO_NSP_H

#include <QObject>
#include <QJSValue>
#include <QVariant>
#include <QMap>

namespace SocketIO {

class Client;
class Packet;

class Nsp : public QObject
{
    Q_OBJECT

    friend class Client;
public:
    explicit Nsp(QString name,Client *parent);

    Q_INVOKABLE void on(QString ev, QJSValue cb);
    Q_INVOKABLE void off(QString ev);
    Q_INVOKABLE void once(QString ev, QJSValue cb);
    Q_INVOKABLE void eemit(QString ev, QJSValue data);

signals:

private:
    void inPacket(Packet * p);
    void callEvent(QString ev, QVariant data = QVariant(QVariant::Invalid));
private:
    Client * _client;
    QString _name;
    QMap<QString,QJSValue> _ioOnMap;
    QMap<QString,QJSValue> _ioOnceMap;
    QMap<QString,QJSValue> _ioAcksMap;
};

}
#endif // NSP_H

#ifndef SIO_PROTOCOL_H
#define SIO_PROTOCOL_H

#include <QObject>
#include <QByteArray>
#include "packet.h"

namespace SocketIO {

struct HandShake {
    QString sid;
    int64_t pingTimeout;
    int64_t pingInterval;
    QStringList upgrades;
};


class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = nullptr);

    QByteArray encryptBinary(Packet * p);
    QString encryptString(Packet * p);

    Packet * decrypt(QByteArray src);
    Packet * decrypt(QString src);

    bool decryptHandShake(QByteArray src, HandShake * hs);

signals:
    void decrypted(Packet*);

protected:
private:
    Packet * decryptString(QString src);
    Packet * decryptBinary(QByteArray src);

    Packet * errorPacket(QString error);
};

}
#endif // PROTOCOL_H

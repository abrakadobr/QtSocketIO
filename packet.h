#ifndef SIO_PACKET_H
#define SIO_PACKET_H

#include <QObject>
#include <QVariant>


namespace SocketIO {

enum FrameType {
    Unknown = -1,
    Open = 0,
    Close = 1,
    Ping = 2,
    Pong = 3,
    Message = 4,
    Upgrade = 5,
    Noop = 6
};

enum PacketType {
    UnknownData = -1,
    Connect = 0,
    Disconnect = 1,
    Event = 2,
    Ack = 3,
    Error = 4,
    BinaryEvent = 5,
    BinaryAck = 6
};


struct IOEvent {
    QString code;
    QVariant data;
};

class Packet : public QObject
{
    Q_OBJECT
public:
    explicit Packet(QObject *parent = nullptr);
    explicit Packet(FrameType ftype, QObject * parent = nullptr);

    FrameType frameType() { return _frameType; }
    void setFrameType(FrameType type) { _frameType = type; }

    PacketType type() { return _dataType; }
    void setType(PacketType type) { _dataType = type; }

    QString nsp() { return _nsp; }
    void setNsp(QString nsp) { _nsp = nsp; }
    bool defaultNsp() { return _nsp == "" || _nsp == "/"; }

    QVariant data() { return _data; };
    void setData(QVariant data) { _data = data; }

    QString rawData();
    void setRawData(QString data) { _rawData = data; }

    QByteArray binaryData() { return _binaryData; }
    void setBinaryData(QByteArray data) { _binaryData = data; }

    bool isBinary() const { return !_binaryData.isEmpty(); }

    IOEvent eventData();
    void setEventData(IOEvent data);
    void setEventData(QString ev, QVariant args);

    void dbg();
signals:

private:
    FrameType _frameType;
    PacketType _dataType;
    QString _nsp;

    QVariant _data;
    QString _rawData;
    QByteArray _binaryData;
};

}

#endif // PACKET_H

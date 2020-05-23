Qt/QML SocketIO Client
----------------------

SocketIO client impementation for C++/Qt/QML without any other deps (except Qt, ofc)), like boost.

Ussage
======

include pri file to project, #include headers, register for qml

    #include "socket.io/client.h"

    ....

    qmlRegisterUncreatableType<SocketIO::Nsp>("QSocketIO",1,0,"SocketIONsp","Namespaces are managed by SocketIO");
    qmlRegisterType<SocketIO::Client>("QSocketIO",1,0,"SocketIOClient");

QML:

    import QSocketIO 1.0

    SocketIOClient {
        id: io
        onConnected: function() {
            console.log('io connected')
            root.wsConnected()
            root.state = "stConnected"
        }
        onDisconnected: function() {
            console.log('io connected')
            root.wsDisconnected()
            root.state = "stDisconnected"
        }
    }

    Component.onCompleted: function()
    {
        io.on('eventCode',(eventData)=>{
            console.log('SocketIO event',eventData)
        })

        let url = '.....'
        io.open(url)

        io.eemit('myEventCode',{data:1})
    }

ToDo
====

namespaces (currently only /), acks, binary (currently not implemented), "on" option for QML items, C++ nice ussage

p.s.
====

Main project repo based on gitlab: https://gitlab.com/abrakadobr/qt-socket.io . This is github mirror

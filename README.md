Qt/QML SocketIO Client
----------------------

register for qml

    qmlRegisterUncreatableType<SocketIO::Nsp>("QSocketIO",1,0,"SocketIONsp","Namespaces are managed by SocketIO");
    qmlRegisterType<SocketIO::Client>("QSocketIO",1,0,"SocketIOClient");

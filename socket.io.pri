QT += quick websockets network

CONFIG += c++11

HEADERS += \
    $$PWD/client.h \
    $$PWD/nsp.h \
    $$PWD/packet.h \
    $$PWD/protocol.h

SOURCES += \
    $$PWD/client.cpp \
    $$PWD/nsp.cpp \
    $$PWD/packet.cpp \
    $$PWD/protocol.cpp

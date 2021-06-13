TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lpthread

SOURCES += \
        client.c \
        main.c \
        server.c \
        utils.c

HEADERS += \
    client.h \
    server.h \
    utils.h

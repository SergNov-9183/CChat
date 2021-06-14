TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lpthread

SOURCES += \
        commands.c \
        main.c \
        client.c \
        utils.c
    
HEADERS += \
        client.h \
        commands.h \
        utils.h
        

DISTFILES += \
    HELP.txt


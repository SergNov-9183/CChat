TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -lpthread

SOURCES += \
        main.c \
        client.c \
        utils.c
    
HEADERS += \
        client.h \
        utils.h
        

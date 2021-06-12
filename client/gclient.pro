TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
LIBS += -lpthread
CONFIG -= qt

SOURCES += \
    Gen_util.C \
    client.c

HEADERS += \
    Gen_util.h \
    client.h

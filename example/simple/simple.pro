TEMPLATE = app
TARGET = simple
CONFIG += qt link_pkgconfig

QT += widgets

DEPENDPATH += ../../src
INCLUDEPATH += ../../src
LIBS += -L../../src -lgrilo-qt5
PKGCONFIG = grilo-0.2

SOURCES += \
    simple-model.cpp \
    simple.cpp

HEADERS += simple-model.h

target.path = /usr/bin

INSTALLS += target

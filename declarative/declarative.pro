TEMPLATE = lib
CONFIG += qt plugin link_pkgconfig no_keywords
DEPENDPATH += ../src
INCLUDEPATH += ../src

QT = core qml

LIBS += -L../src -lgrilo-qt5
PKGCONFIG = grilo-0.3

EXAMPLE = ../example/declarative/*

OTHER_FILES += $${EXAMPLE}

TARGET = grilo-qt5-qml-plugin
PLUGIN_IMPORT_PATH = org/nemomobile/grilo

QMAKE_SUBSTITUTES = qmldir.in

SOURCES += \
    griloplugin.cpp \
    declarativegrilomodel.cpp

HEADERS += \
    griloplugin.h \
    declarativegrilomodel.h

target.path = $$[QT_INSTALL_QML]/$$PLUGIN_IMPORT_PATH

qml.files = qmldir plugins.qmltypes
qml.path = $$target.path
INSTALLS += target qml

qmltypes.commands = qmlplugindump -nonrelocatable org.nemomobile.grilo 0.1 > $$PWD/plugins.qmltypes
QMAKE_EXTRA_TARGETS += qmltypes

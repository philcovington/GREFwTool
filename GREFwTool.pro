
QT       += widgets
QT       += serialport
QT       += network

TARGET = GREFwTool
TEMPLATE = app
VERSION = 0.2.0.3

DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += QT_NO_SSL

SOURCES += \
    source/main.cpp \
    source/mainwindow.cpp \
    source/settingsdialog.cpp \
    source/greparser.cpp \
    source/grefirmware.cpp \
    source/webdownloader.cpp \
    source/display.cpp

HEADERS += \
    include/mainwindow.h \
    include/settingsdialog.h \
    include/greparser.h \
    include/grefirmware.h \
    include/webdownloader.h \
    include/display.h

FORMS += \
    ui/mainwindow.ui \
    ui/settingsdialog.ui

RESOURCES += \
    GREFwTool.qrc

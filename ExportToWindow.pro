TARGET = exportToWindow

TEMPLATE = app

QT += quick qml widgets core gui quickcontrols2
CONFIG += static
static {
# Everything below takes effect with CONFIG += static
    CONFIG += static
    QTPLUGIN += qsqloci qgif
    DEFINES += STATIC
# Equivalent to "#define STATIC" in source code
    message("Static build.")
}
#INCLUDEPATH += "C:\Program Files (x64)\Qt6\include\qt6"
INCLUDEPATH += "C:\Program Files (x64)\Qt6.8\include\qt6"
SOURCES += main.cpp \
    backend.cpp
RESOURCES += \
    exportToWindow.qrc \
    shared\shared.qrc

target.path = EXPORTWINDOW/exportToWindow
INSTALLS += target

RC_ICONS = resources/icon.ico
ICON = resources/icon.ico
win32: RC_FILE = resources/exportToWindow.rc

HEADERS += \
    backend.h

DISTFILES += \
    CustomBorderRect.qml \
    main.qml

CONFIG += qmltypes
QML_IMPORT_NAME = backend
QML_IMPORT_MAJOR_VERSION = 1

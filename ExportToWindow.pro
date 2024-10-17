TARGET = ExportToWindow

TEMPLATE = app

QT += quick qml widgets core gui
CONFIG += static
static {
# Everything below takes effect with CONFIG += static
    CONFIG += static
    QTPLUGIN += qsqloci qgif
    DEFINES += STATIC
# Equivalent to "#define STATIC" in source code
    message("Static build.")
}

SOURCES += main.cpp \
    backend.cpp
RESOURCES += \
    ExportToWindow.qrc \
    shared\shared.qrc

target.path = EXPORTWINDOW/ExportToWindow
INSTALLS += target

RC_ICONS = resources/icon.ico
ICON = resources/icon.ico
win32: RC_FILE = resources/ExportToWindow.rc

HEADERS += \
    backend.h

DISTFILES += \
    CustomBorderRect.qml \
    main.qml

CONFIG += qmltypes
QML_IMPORT_NAME = backend
QML_IMPORT_MAJOR_VERSION = 1

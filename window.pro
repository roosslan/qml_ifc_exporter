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
    window.qrc \
    shared\shared.qrc
EXPORTWINDOW = \
    window.qml \
    resources

target.path = EXPORTWINDOW/window
INSTALLS += target

ICON = resources/icon64.png
macx: ICON = resources/icon.icns
win32: RC_FILE = resources/window.rc

HEADERS += \
    backend.h

DISTFILES += \
    CustomBorderRect.qml \
    TimePicker.qml \
    main.qml

CONFIG += qmltypes
QML_IMPORT_NAME = backend
QML_IMPORT_MAJOR_VERSION = 1

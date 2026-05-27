# last changed 12.5.2026

TARGET = ifc_exporter

TEMPLATE = app

QT += quick qml widgets core gui quickcontrols2
CONFIG += c++20 \
    static
static {
# Everything below takes effect with CONFIG += static
    CONFIG += static
    QTPLUGIN += qsqloci qgif
    DEFINES += STATIC
# Equivalent to "#define STATIC" in source code
    message("Static build.")
}

INCLUDEPATH += C:\Program Files (x64)\Qt\6.10.0\msvc2022_64\include \
                C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207\include

RESOURCES += \
    ifc_exporter.qrc \
    shared\shared.qrc

target.path = IFC_EXPORTER/ifc_exporter
INSTALLS += target

RC_ICONS = resources/icon.ico
ICON = resources/icon.ico
win32: RC_FILE = resources/ifc_exporter.rc

SOURCES += main.cpp \
    backend.cpp \
    helper_funcs.cpp \
    named_pipe_client.cpp

HEADERS += \
    backend.h \
    helper_funcs.h \
    named_pipe_client.h \
    resources/ifc_exporter.rc \
    sensitive_data.h

DISTFILES += \
    shared/RCheckBox.qml \
    shared/RLabel.qml \
    shared/SensitiveData.qml \
    shared/TextField.qml \
    shared/UTimeDialog.qml  \
    shared/UTumbler.qml  \
    shared/UCard.qml    \
    shared/URect.qml    \
    shared/UTimePicker.qml  \
    shared/CheckBox3DViews.qml \
    shared/PlusButtonRow.qml    \
    shared/CustomBorderRect.qml \
    shared/JWDMDatePicker.qml \
    main.qml

CONFIG += qmltypes
QML_IMPORT_NAME = backend
QML_IMPORT_MAJOR_VERSION = 1

# last changed 16.10.2026

TARGET = ifc_exporter

TEMPLATE = app

QT += quick qml widgets core gui quickcontrols2 network
CONFIG += c++20 \
    static
static {
# Everything below takes effect with CONFIG += static
    CONFIG += static
    QTPLUGIN += qgif
    DEFINES += STATIC
# Equivalent to "#define STATIC" in source code
    message("Static build.")
}

# src/sensitive_data.h и shared/SensitiveData.qml в git не хранятся:
# при их отсутствии создаются из шаблонов *.example, значения затем необходимо заменить на реальные
for(sensitive_file, $$list(src/sensitive_data.h shared/SensitiveData.qml)) {
    !exists($$PWD/$$sensitive_file) {
        system($$QMAKE_COPY $$shell_quote($$shell_path($$PWD/$${sensitive_file}.example)) $$shell_quote($$shell_path($$PWD/$$sensitive_file)))
        warning("$$sensitive_file создан из шаблона $${sensitive_file}.example")
    }
}

RESOURCES += \
    ifc_exporter.qrc \
    shared/shared.qrc

target.path = IFC_EXPORTER/ifc_exporter
INSTALLS += target

RC_ICONS = resources/ico.ico
win32: RC_FILE = resources/ifc_exporter.rc

INCLUDEPATH += src

SOURCES += src/main.cpp \
    src/backend.cpp \
    src/helper_funcs.cpp \
    src/named_pipe_client.cpp

HEADERS += \
    src/backend.h \
    src/helper_funcs.h \
    src/named_pipe_client.h \
    src/sensitive_data.h \
    src/simpleini.h

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
    main.qml \
    src/sensitive_data.h.example \
    shared/SensitiveData.qml.example

CONFIG += qmltypes
QML_IMPORT_NAME = backend
QML_IMPORT_MAJOR_VERSION = 1

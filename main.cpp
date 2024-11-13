#include <windows.h>
#include <QtGui/QGuiApplication>

#include <QtGui/QScreen>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>
#include <QtQuick/QQuickWindow>
#include <QtCore/QUrl>
#include <QtPlugin>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QQuickItem>
#include "backend.h"

#include <QApplication>
#include <QGuiApplication>
#include <QListView>
#include <QQuickView>
#include <QStandardItem>
#include <QWidget>
#include <QtQml>
#include <QAbstractListModel>
#include <QtLogging>

int main (int argc, char* argv[])
{
    qInstallMessageHandler(bgMessageHandler);
    const int windowWidth = 1300;
    const int windowHeight = 870;

    QGuiApplication qGUIApp(argc, argv);

    QQuickView view;
    view.setSource(QUrl::fromLocalFile("../main.qml"));
    QObject *item = view.rootObject();

    BackEnd backEndRula(&qGUIApp, item);

    QObject::connect(item, SIGNAL(escKeyPressedSignal()), &backEndRula, SLOT(escSlot()));
    QObject::connect(item, SIGNAL(signalStopClicked()), &backEndRula, SLOT(slotStopClicked()));
    QObject::connect(item, SIGNAL(signalRunClicked(QString)), &backEndRula, SLOT(slotRunClicked(QString)));
    view.setIcon(QIcon("resources/ico.ico"));
    view.show();

    view.setTitle("BIMALDE - ExportTo");

    view.setMaximumHeight(windowHeight);
    view.setMinimumHeight(windowHeight);
    view.setMaximumWidth(windowWidth);
    view.setMinimumWidth(windowWidth);

    return qGUIApp.exec ();
}

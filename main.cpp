
#include <QtGui/QGuiApplication>

#include <QtGui/QScreen>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>
#include <QtQuick/QQuickWindow>
#include <QtCore/QUrl>
#include <QtPlugin>
#include <QDebug>

#include <QQuickItem>
#include "backend.h"

#include <QApplication>
#include <QGuiApplication>
#include <QListView>
#include <QQuickView>
#include <QStandardItem>
#include <QWidget>
#include <QtQml>


int main (int argc, char* argv[])
{
    const int windowWidth = 1300;
    const int windowHeight = 870;

//  QApplication app(argc, argv);
    QGuiApplication q_app(argc, argv);
    QQuickView view;
//  QWidget *container = QWidget::createWindowContainer(&view);
    view.setSource(QUrl::fromLocalFile("../main.qml"));
    QObject *item = view.rootObject();
    BackEnd BackEnd(&q_app);
    QObject::connect(item, SIGNAL(qmlSignal(QString)), &BackEnd, SLOT(cppSlot(QString)));
    QObject::connect(item, SIGNAL(selectedFileSignal(QString)), &BackEnd, SLOT(selectedFileSlot(QString)));
    view.setIcon(QIcon("resources/ico.ico"));
    view.show();
//    auto listView = item->findChild<QQuickItem *>("objLVItem");
    view.setTitle("BIMALDE - ExportTo");
    view.setMaximumHeight(windowHeight);
    view.setMinimumHeight(windowHeight);
    view.setMaximumWidth(windowWidth);
    view.setMinimumWidth(windowWidth);

    return q_app.exec ();
}

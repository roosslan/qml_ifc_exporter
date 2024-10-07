
#include <QtGui/QGuiApplication>

#include <QtGui/QScreen>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>
#include <QtQuick/QQuickWindow>
#include <QtCore/QUrl>
#include <QtPlugin>
#include <QDebug>
#include <qquickview.h>
#include <QQuickItem>
#include "backend.h"

#include <QApplication>
#include <QGuiApplication>
#include <QListView>
#include <QQuickView>
#include <QWidget>
#include <QtQml>


int main (int argc, char* argv[])
{
//  QApplication app(argc, argv);
    QGuiApplication q_app(argc, argv);
    QQuickView view;
//  QWidget *container = QWidget::createWindowContainer(&view);
    view.setSource(QUrl::fromLocalFile("../main.qml"));
    QObject *item = view.rootObject();
    BackEnd BackEnd(&q_app);
    QObject::connect(item, SIGNAL(qmlSignal(QString)), &BackEnd, SLOT(cppSlot(QString)));
    QObject::connect(item, SIGNAL(selectedFileSignal(QString)), &BackEnd, SLOT(selectedFileSlot(QString)));
    view.show();
    auto listView = item->findChild<QQuickItem *>("objLVItem");



    return q_app.exec ();
}

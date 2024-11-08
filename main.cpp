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

    QGuiApplication q_app(argc, argv);
    QQuickView view;
    view.setSource(QUrl::fromLocalFile("../main.qml"));
    QObject *item = view.rootObject();

    BackEnd backEndRula(&q_app);

    QObject::connect(item, SIGNAL(qmlSignal(QString)), &backEndRula, SLOT(cppSlot(QString)));
    QObject::connect(item, SIGNAL(escKeyPressedSignal()), &backEndRula, SLOT(escSlot()));
    QObject::connect(item, SIGNAL(selectedFileSignal(QString)), &backEndRula, SLOT(selectedFileSlot(QString)));
    QObject::connect(item, SIGNAL(signalRunClicked(QString)), &backEndRula, SLOT(slotRunClicked(QString)));
    view.setIcon(QIcon("resources/ico.ico"));
    view.show();

    QString appData = getenv("appdata");
    QString iniFile = appData + "\\alabuga_dev\\export.inf";
    LPCWSTR iniFName = (const wchar_t*) iniFile.utf16();

    QQuickItem* lvMain = item->findChild<QQuickItem*>("o_lvMain");
    QObject* listModel = lvMain->children()[1];
    QAbstractListModel* qmlListModel = qobject_cast<QAbstractListModel*>(listModel);

    QQuickItem* lvCB_RvtVers = item->findChild<QQuickItem*>("row_RvtVersion");
    QObject* cb_RvtVers = lvCB_RvtVers->children()[1];
    QString revitVersion = cb_RvtVers->property("currentText").toString();
    WritePrivateProfileStringW(L"ControlFlags", L"RevitVersion", (const wchar_t*) revitVersion.utf16(), iniFName);

    QQuickItem* qcbIFC = item->findChild<QQuickItem*>("cbIFC");
    //QqmlCheckQmlCheclAbstractListModel* qmlListModel = qobject_cast<QAbstractListModel*>(listModel);
    bool IsCbIFC_checked = qcbIFC->property("checked").toBool();


    if (qmlListModel != nullptr)
    {
        for (int i = 0; i < qmlListModel->rowCount(); ++i)
        {
            QString rvtFile = qmlListModel->data(qmlListModel->index(i, 0), 0).toString();
            LPCWSTR rvtFName = (const wchar_t*) rvtFile.utf16();
            WritePrivateProfileStringW(L"RVT_FILES", rvtFName, L"1", iniFName);
        }
    }
    else
    {
        qDebug() << "failed!";
    }

    view.setTitle("BIMALDE - ExportTo");
    view.setMaximumHeight(windowHeight);
    view.setMinimumHeight(windowHeight);
    view.setMaximumWidth(windowWidth);
    view.setMinimumWidth(windowWidth);

    return q_app.exec ();
}

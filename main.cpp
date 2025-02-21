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
#include <QListView>
#include <QQuickView>
#include <QStandardItem>
#include <QWidget>
#include <QtQml>
#include <QAbstractListModel>
#include <QtLogging>
#include <QQuickStyle>

int main (int argc, char* argv[])
{
    /* Разрешаем только один запуск окна экспорта */
    QSharedMemory shared("02d60619-bb94-4a94-88bb-b965590a7eaa");
    if( !shared.create( 512, QSharedMemory::ReadWrite) )
    {
        QLibrary qLib;
        char win_NameWin[] = "BIMALDE - ExportTo", win_MessageWin[] = "Окно экспорта IFC уже запущено";
        int iResult = 0x00;
        bool unLoad = false;

        qLib.setFileName("user32");
        if(qLib.load())
            if(qLib.isLoaded())
            {
                typedef int (*pMessageBox)(void* hWnd, char *lpText, char *lpCaption, unsigned int uType);
                pMessageBox MessageBoxA = (pMessageBox)qLib.resolve("MessageBoxA");

                if(MessageBoxA)
                    iResult = MessageBoxA(nullptr, &win_NameWin[0x00], &win_MessageWin[0x00], 0x40);

                MessageBoxA = nullptr;
                unLoad = qLib.unload();
            }
        exit(0);
    }

    /* Передаём процесс в заголовок окна ExportTo */
    QString pID = "BIMALDE - ExportTo ";
    //if (argc == 2)
    try {
        pID += argv[1];
    }
    catch (...)
    {
    }

    qInstallMessageHandler(bgMessageHandler);
    const int windowWidth = 1300;
    const int windowHeight = 870;

    QGuiApplication qGUIApp(argc, argv);

    const QUrl url(QStringLiteral("../main.qml"));
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
            &qGUIApp, [url](QObject *obj, const QUrl &objUrl)
            {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);

            }, Qt::QueuedConnection);

    /*  engine.clearComponentCache();       //unload all QML
        engine.exit(0);                     //destroy any existing QQmlEngine instance(s)
        qmlClearTypeRegistrations();        //call qmlClearTypeRegistrations()
        QQuickStyle::setStyle("Material");
        engine.load(url);
    */
    QQuickView view;

    view.setSource(QUrl::fromLocalFile("../main.qml"));

    QObject *item = view.rootObject();

    BackEnd backEndRula(&qGUIApp, item, (HWND)view.winId());
    /* чтобы пользователь был уверен, что путь с последнего сеанса сохранился: */
    QString exportDirectory = backEndRula.ReadInfString("DestinationDirs", "DefaultDestDir");
    QQuickItem* QQuickText_IFCPath = item->findChild<QQuickItem*>("text_IFCPath");
    QQuickText_IFCPath->setProperty("text", exportDirectory);

    QObject::connect(item, SIGNAL(signalBtnIFCSettingsClicked()), &backEndRula, SLOT(slotBtnIFCSettingsClicked()));
    QObject::connect(item, SIGNAL(escKeyPressedSignal()), &backEndRula, SLOT(slotEscPressed()));
    QObject::connect(item, SIGNAL(signalStopClicked()), &backEndRula, SLOT(slotStopClicked()));
    QObject::connect(item, SIGNAL(signalRunClicked(QString, int)), &backEndRula, SLOT(slotRunClicked(QString, int)));
    QObject::connect(item, SIGNAL(signalIsFileExists(QString, QString)), &backEndRula, SLOT(slotIsFileExists(QString, QString)));

    view.setIcon(QIcon("resources/ico.ico"));

    view.show();

    view.setTitle(pID);

    view.setMaximumHeight(windowHeight);
    view.setMinimumHeight(windowHeight);
    view.setMaximumWidth(windowWidth);
    view.setMinimumWidth(windowWidth);

    return qGUIApp.exec ();
}

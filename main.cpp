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

#include <qcheckbox.h>

int main (int argc, char* argv[])
{
    /* Разрешаем только один запуск окна экспорта */
    QSharedMemory shared("ca11ab1e-deaf-dadd-face-ba11ade0cafe");
    if( !shared.create( 512, QSharedMemory::ReadWrite) )
    {
        QLibrary qLib;
        char win_name_win[] = "IFC exporter", win_message_win[] = "IFC exporter is already opened";
        int res = 0x00;
        bool unLoad = false;

        qLib.setFileName("user32");
        if(qLib.load())
            if(qLib.isLoaded())
            {
                typedef int (*pMessageBox)(void* hWnd, char *lpText, char *lpCaption, unsigned int uType);
                pMessageBox MessageBoxA = (pMessageBox)qLib.resolve("MessageBoxA");

                if(MessageBoxA)
                    res = MessageBoxA(nullptr, &win_message_win[0x00], &win_name_win[0x00], 0x40);

                MessageBoxA = nullptr;
                unLoad = qLib.unload();
            }
        exit(0);
    }

    /* Передаём процесс в заголовок окна IFC exporter */
    QString pID = "IFC exporter ";
    try {
        pID += argv[1];
    }
    catch (...)
    {
    }

    qInstallMessageHandler(bgMessageHandler);
    const int window_width = 1300;
    const int window_height = 870;

    QGuiApplication qgui_app(argc, argv);    

    auto local_qml_file = QCoreApplication::applicationDirPath() + "/main.qml";

    const QUrl main_qml_file(QStringLiteral("../main.qml"));
    QQmlApplicationEngine qqml_app_engine;
    QObject::connect(&qqml_app_engine, &QQmlApplicationEngine::objectCreated,
            &qgui_app, [main_qml_file](QObject *obj, const QUrl &obj_url)
            {
                if (!obj && main_qml_file == obj_url)
                    QCoreApplication::exit(-1);

            }, Qt::QueuedConnection);

    ExportQuickView q_view;

    q_view.setSource(QUrl::fromLocalFile("../main.qml"));

    QObject *item = q_view.rootObject();

    BackEnd backend_rula(&qgui_app, item, &q_view);
    q_view.backend_ruler = &backend_rula;

    /* Передаем экземпляр класса в Qml: */
    q_view.rootContext()->setContextProperty("backend", &backend_rula);

    QObject::connect(item, SIGNAL(signal_esc_key_pressed()), &backend_rula, SLOT(slot_esc_pressed()));
    QObject::connect(item, SIGNAL(signal_window_blink()), &backend_rula, SLOT(slot_window_blink()));
    QObject::connect(item, SIGNAL(signal_stop_clicked()), &backend_rula, SLOT(slot_stop_clicked()));
    QObject::connect(item, SIGNAL(signal_copy_to_clipboard_clicked()), &backend_rula, SLOT(slot_copy_to_clipboard_pressed()));

    QObject::connect(item, SIGNAL(signal_run_clicked(const int, const QString&, const QString&)), &backend_rula, SLOT(slot_run_clicked(const int, const QString&, const QString&)));

    QObject::connect(item, SIGNAL(signal_clear_log_files()), &backend_rula, SLOT(slot_clear_log_files()));

    QObject::connect(item, SIGNAL(signal_save_views_and_sites_to_file(const QString&, const QString&, const QString&, const QString&, const QString&, const bool, const int)),
                     &backend_rula, SLOT(slot_save_views_and_sites_to_file(const QString&, const QString&, const QString&, const QString&, const QString&, const bool, const int)));

    /* Чтобы пользователь был уверен, что путь с последнего сеанса сохранился: */
    const QString export_directory = backend_rula.read_inf_string("DestinationDirs", "DefaultDestDir");
    QQuickItem* qquick_text_ifc_path = item->findChild<QQuickItem*>("text_IFCPath");
    qquick_text_ifc_path->setProperty("text", export_directory);

    backend_rula.fill_combobox_sav_files();

   /* Следующий код передает в QML системные переменные наподобие $APPDATA */
    item->setProperty("ifc_exporter_directory", QDir::homePath() + "/AppData/Roaming/" + qml_app_directory);

    q_view.setIcon(QIcon("resources/ico.ico"));

    q_view.show();

    q_view.setTitle(pID);

    q_view.setMaximumHeight(window_height);
    q_view.setMinimumHeight(window_height);
    q_view.setMaximumWidth(window_width);
    q_view.setMinimumWidth(window_width);

    backend_rula.write_inf_string("Manufacturer", "sav_file_for_export", backend_rula.views_and_sites_file);
    backend_rula.v_sav_files.push_back(backend_rula.views_and_sites_file.toStdString());
    backend_rula.load_sav_file_into_main_list(backend_rula.views_and_sites_file);

    return qgui_app.exec ();
}

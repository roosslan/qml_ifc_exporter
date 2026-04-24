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

    BackEnd backend_rula(&qgui_app, item, (HWND)q_view.winId());
    q_view.backend_ruler = &backend_rula;

    QObject::connect(item, SIGNAL(signal_esc_key_pressed()), &backend_rula, SLOT(slot_esc_pressed()));
    QObject::connect(item, SIGNAL(signal_stop_clicked()), &backend_rula, SLOT(slot_stop_clicked()));
    QObject::connect(item, SIGNAL(signal_copy_to_clipboard_clicked()), &backend_rula, SLOT(slot_copy_to_clipboard_pressed()));

    QObject::connect(item, SIGNAL(signal_run_clicked(const int, const QString&, const QString&)), &backend_rula, SLOT(slot_run_clicked(const int, const QString&, const QString&)));

    QObject::connect(item, SIGNAL(signal_is_file_exists(const QString&, const QString&)), &backend_rula, SLOT(slot_is_file_exists(const QString&, const QString&)));

    QObject::connect(item, SIGNAL(signal_save_views_and_sites_to_file(const QString&, const QString&, const QString&, const QString&, const QString&, const bool, const int)),
                     &backend_rula, SLOT(slot_save_views_and_sites_to_file(const QString&, const QString&, const QString&, const QString&, const QString&, const bool, const int)));

    /* Чтобы пользователь был уверен, что путь с последнего сеанса сохранился: */
    const QString export_directory = backend_rula.read_inf_string("DestinationDirs", "DefaultDestDir");
    QQuickItem* qquick_text_ifc_path = item->findChild<QQuickItem*>("text_IFCPath");
    qquick_text_ifc_path->setProperty("text", export_directory);

   /* Следующий код передает в QML системные переменные наподобие $APPDATA */
    item->setProperty("home_directory", QDir::homePath());

    q_view.setIcon(QIcon("resources/ico.ico"));

    q_view.show();

    q_view.setTitle(pID);

    q_view.setMaximumHeight(window_height);
    q_view.setMinimumHeight(window_height);
    q_view.setMaximumWidth(window_width);
    q_view.setMinimumWidth(window_width);

    const QKeysValues v_files_list = backend_rula.get_section_keys_and_values("SourceDisksFiles");
    to_restore line_to_restore;
    const std::vector<to_restore> v_views_n_sites = backend_rula.get_all_keys_and_values_of_file(backend_rula.views_and_sites_file);

    int list_index_to_add = -1;
    QVariant returned_value;

    if (!v_files_list.empty())
    foreach (const auto qpair, v_files_list) { /* first - fname, second - bool_as_str "should_be_exported?" */
         /* Добавляем в список обыкновенные RVT, без вьюх и площадок */
        const auto it = std::find_if(v_views_n_sites.begin(), v_views_n_sites.end(),
                     [&qpair](const to_restore& item) {
                         return item.fname == qpair.first; /* fname */
            });
        if (it == v_views_n_sites.end())
        {
            auto fname = qpair.first;
            bool should_exported = backend_rula.str2bool(qpair.second);
            QMetaObject::invokeMethod(item, "add_row_from_cpp",
                                        Q_RETURN_ARG(QVariant, returned_value),
                                        Q_ARG(const QString&, fname),
                                        Q_ARG(const bool,    should_exported));
            ++list_index_to_add;
         }
    }

    QString last_added_fname = "";
    /* Добавляем в список всё остальное, эти файлы уже с указанными вьюхами или площадками */
    if(!v_views_n_sites.empty())
    foreach (const auto line_to_restore, v_views_n_sites)
    {
        if (last_added_fname == line_to_restore.fname)
            QMetaObject::invokeMethod(item, "add_subrow_wrapper",
                                        Q_RETURN_ARG(QVariant, returned_value),
                                        Q_ARG(const int, list_index_to_add),
                                        Q_ARG(const QString, line_to_restore.view),
                                        Q_ARG(const QString, line_to_restore.site),
                                        Q_ARG(const QString, line_to_restore.outputfname),
                                        Q_ARG(const QString, line_to_restore.jsonpath));
        else
        {
            ++list_index_to_add;
            QMetaObject::invokeMethod(item, "add_row_w_subrows_from_cpp",
                                        Q_RETURN_ARG(QVariant, returned_value),
                                        Q_ARG(const int, list_index_to_add),
                                        Q_ARG(const QString, line_to_restore.fname),
                                        Q_ARG(const QString, line_to_restore.view),
                                        Q_ARG(const QString, line_to_restore.site),
                                        Q_ARG(const QString, line_to_restore.outputfname),
                                        Q_ARG(const QString, line_to_restore.jsonpath),
                                        Q_ARG(const bool,    line_to_restore.should_exported));

        }
        last_added_fname = line_to_restore.fname;
    }
    return qgui_app.exec ();
}

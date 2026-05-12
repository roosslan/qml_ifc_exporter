/* last changed 24.4.2026 */

#ifndef BACKEND_H
#define BACKEND_H

#include "simpleini.h"
#include "helper_funcs.h"
#include <windows.h>

#include <qqml.h>
#include <QObject>
#include <QString>
#include <QGuiApplication>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMetaType>
#include <QMessageBox>
#include <QStandardPaths>
#include <qquickview.h>
#include <qlibrary.h>
#include <qclipboard.h>
#include <QtQuick>

class ExportQuickView; /* forward declaration */

typedef QList<std::pair<QString, QString> > QKeysValues;

struct to_restore
{
    QString fname;
    QString view;
    QString site;
    QString outputfname;
    QString jsonpath;
    bool    should_exported;
};

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QGuiApplication* m_window_;
    QTcpSocket m_tcp_socket_;
    QObject *m_item;
    HWND m_hwnd_;
    std::string m_str_hwnd_; /* Для передачи в окно IFCSettings */
    QTcpServer* m_server;

    QString m_app_data_ = get_env("appdata");

    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    CSimpleIniW m_config_file_;

    QSet<QTcpSocket*> m_connection_set_;
signals:
    void new_message(const QString);
private slots:
    void new_socket_connection();
    void append_to_socket_list(QTcpSocket* socket);
    void read_socket();
    void discard_socket();
    void display_error(QAbstractSocket::SocketError socket_error);
    void display_log_message(const QString& qstr_msg);

public:
    BackEnd(QGuiApplication *parent, QObject* item, HWND hwnd);
    ~BackEnd();
    QString inf_file = m_app_data_ + "\\alabuga_dev\\bimalde.inf";
    QString views_and_sites_file = m_app_data_ + "\\alabuga_dev\\views_sites.sav";
    std::list<std::string> v_sav_files;
    void fill_combobox_sav_files();
    void load_sav_file_into_main_list(const QString &sav_file);
    Q_INVOKABLE void on_sav_combo_changed(int index, const QString &file_name, QString full_path);

    const bool str2bool(const QString& bool_as_str);

    /* Для DeleteInfSection */
    LPCWSTR inf_name = (const wchar_t*) inf_file.utf16();

    QKeysValues get_section_keys_and_values(const QString& section_name);
    std::vector<to_restore> get_all_keys_and_values_of_file(const QString& file_name);
    QString read_inf_string(const QString& section_name, const QString& key_name);
    void write_inf_string(const QString& section_name, const QString& key_name, const QString& value);
    void delete_inf_section(const QString& section_name);

public slots:
    void slot_esc_pressed();
    void slot_stop_clicked();
    void slot_copy_to_clipboard_pressed();
    void slot_run_clicked(const int right_now, const QString &utime, const QString &udate);
    void slot_clear_log_files();
    void slot_save_views_and_sites_to_file(const QString& fname, const QString& view_name, const QString& site_name, const QString& output_file_name,
                                           const QString& json_file_path, const bool should_be_exported, const int append_mode);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg);

template<class T> T& unmove(T&& t) { return static_cast<T&>(t); }

class ExportQuickView : public QQuickView
{
    Q_OBJECT
public:
    explicit ExportQuickView(QWindow *parent = nullptr) : QQuickView(parent) {}
    BackEnd* backend_ruler;
protected:
    void closeEvent(QCloseEvent *event) override
    {
        QVariant returned_value;
        QMetaObject::invokeMethod(rootObject(), "get_rows_wo_views_n_sites",
                              Q_RETURN_ARG(QVariant, returned_value));
        int rows_wo_views_n_sites = returned_value.toInt();
        /* если ListView со списком файлов вернул 0 по позициям со вьюхами, то очищаем секцию в .INF */
        if(rows_wo_views_n_sites != 0)
        {
            const QQuickItem* lv_main = rootObject()->findChild<QQuickItem*>("o_lvMain");
            QObject* list_model = lv_main->children()[1];
            QAbstractListModel* qml_list_model = qobject_cast<QAbstractListModel*>(list_model);

            if (qml_list_model != nullptr)
            {
                for (int i = 0; i < qml_list_model->rowCount(); ++i)
                {
                    const QString rvt_file_name = qml_list_model->data(qml_list_model->index(i, 0), 0).toString();
                    const QString should_be_exported = qml_list_model->data(qml_list_model->index(i, 0), 1).toString();
                    backend_ruler->write_inf_string("SourceDisksFiles", rvt_file_name, should_be_exported);
                }
            }
        }
        else
        {
            backend_ruler->delete_inf_section("SourceDisksFiles");
        }

        QMetaObject::invokeMethod(rootObject(), "save_views_and_sites_to_file");
        event->accept();
    }
};

#endif // BACKEND_H

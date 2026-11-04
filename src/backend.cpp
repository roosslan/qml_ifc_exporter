/*
*  24.04.2025 Выбор версии IFC перенесен в отд. программу/окно
*  24.4.2026, removed 60-chars dividing
*  last changed 14.5.26
*/

#include "backend.h"
#include <QCheckbox>
#include <QNetworkInterface>
#include <QProcess>
#include <windows.h>
#include "qquickitem.h"
#include <QQuickView>
#include <qguiapplication.h>
#include <QFileDialog>
#include <QtCore/qabstractitemmodel.h>
#include <sstream>
#include <QSaveFile>
#include "named_pipe_client.h"

BackEnd::BackEnd(QGuiApplication *parent, QObject* item, ExportQuickView* qview) {
    m_window_ = parent;
    m_item = item;
    m_hwnd_ = (HWND)qview->winId();
    m_qview_ = qview;

    /* На x64 HWND 64-битный, поэтому приводим к quintptr, а не к uint32_t */
    const quintptr int_hwnd = reinterpret_cast<quintptr>(m_hwnd_);

    std::stringstream ss;

    ss << std::hex << int_hwnd;
    ss >> m_str_hwnd_;

    /* Сервер работает в GUI-потоке: сообщений мало, а слоты обращаются к QML */
    m_server = new QTcpServer(this);

    const QString qtcp_port = read_inf_string("Manufacturer", "tcp_port");
    bool port_ok = false;
    int tcp_port = qtcp_port.toInt(&port_ok);
    if (!port_ok || tcp_port <= 0 || tcp_port > 65535) {
        /* Тот же порт по умолчанию, что и у bgHelper (GetPrivateProfileIntW(..., 6777, ...)) */
        display_log_message("| Ошибка: в " + inf_file + " не задан корректный [Manufacturer] tcp_port ('"
                            + qtcp_port + "'), используется порт по умолчанию " + QString::number(default_tcp_port));
        tcp_port = default_tcp_port;
    }

    display_log_message("| Подключаемся к bgHelper для отображения лога...");

    if (!m_server->listen(QHostAddress::LocalHost, tcp_port)) {
        qDebug() << "Unable to start QTcp server: " << m_server->errorString();
        display_log_message("| Лог bgHelper не выведется! " + m_server->errorString() + "; TCP port " + QString::number(tcp_port));
        m_server->close();
    }
    else {
        qDebug() << "QTcp server started";
        connect(this, &BackEnd::new_message, this, &BackEnd::display_log_message);
        connect(m_server, &QTcpServer::newConnection, this, &BackEnd::new_socket_connection);
    }
}

BackEnd::~BackEnd() {
    /* configFile.Reset(); */
}

void BackEnd::new_socket_connection() {
    while (m_server->hasPendingConnections())
        append_to_socket_list(m_server->nextPendingConnection());
}

void BackEnd::append_to_socket_list(QTcpSocket* socket) {
    m_connection_set_.insert(socket);
    connect(socket, &QTcpSocket::readyRead, this, &BackEnd::read_socket);
    connect(socket, &QTcpSocket::disconnected, this, &BackEnd::discard_socket);
    connect(socket, &QAbstractSocket::errorOccurred, this, &BackEnd::display_error);

    display_log_message(QString("| Подготовка к выгрузке ") + QString::fromStdString(m_str_hwnd_));
    display_log_message(QString("| Фоновой процесс %1 запуска Revit подключен!").arg(socket->socketDescriptor()));
}

void BackEnd::discard_socket() {
    QTcpSocket* q_socket = reinterpret_cast<QTcpSocket*>(sender());
    const QSet<QTcpSocket*>::iterator it = m_connection_set_.find(q_socket);
    if (it != m_connection_set_.end()) {
/*      displayMessage(QString("INFO :: A client : %1 has just left").arg(socket->socketDescriptor())); */
        m_connection_set_.remove(*it);
    }

    m_socket_buffers_.remove(q_socket);
    q_socket->deleteLater();
}

void BackEnd::read_socket() {
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket == nullptr)
        return;
    auto& buffer = m_socket_buffers_[socket];
    buffer += socket->readAll();
    qsizetype end;
    while ((end = buffer.indexOf('\n')) >= 0) {
        QByteArray line = buffer.left(end);
        buffer.remove(0, end + 1);
        if (line.endsWith('\r'))
            line.chop(1);
        display_log_message("bgHelper | " + QString::fromUtf8(line));
    }
    /* Ограничиваем память при подключении клиента с неверным протоколом. */
    if (buffer.size() > 1024 * 1024) {
        display_log_message("| Ошибка TCP: строка лога превышает 1 МиБ, соединение закрыто");
        buffer.clear();
        socket->abort();
    }
}

void BackEnd::display_error(QAbstractSocket::SocketError socket_error) {
    switch (socket_error) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug() << "The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.";
        break;
    default:
        QTcpSocket* qsocket = qobject_cast<QTcpSocket*>(sender());
        qDebug() << "The following error occurred: %1." << qsocket->errorString();
        break;
    }
}

/* Adds and displays a message in the LOG-listbox */
void BackEnd::display_log_message(const QString& qstr_msg) {
    QObject* lm_log = m_item != nullptr ? m_item->findChild<QAbstractListModel*>("model_log") : nullptr;
    if (lm_log == nullptr) {
        qDebug() << "Log ListView not found, message:" << qstr_msg;
    }
    QVariant returned_value;
    const QVariant lm_msg = qstr_msg;
    if (lm_log != nullptr)
        QMetaObject::invokeMethod(lm_log, "add_to_log_listview",  /* add_row function defined in QML-file */
                              Q_RETURN_ARG(QVariant, returned_value),
                              Q_ARG(QVariant, lm_msg));

    if (qstr_msg.startsWith("bgHelper | End of export")) {
        QMetaObject::invokeMethod(m_item, "stop_clicked",
                                  Q_RETURN_ARG(QVariant, returned_value));

        const QString app_data = get_env("appdata");
        if (QFile::exists(app_data + app_directory + "\\msg_on_finish")) {

            m_qview_->requestActivate();

            QMetaObject::invokeMethod(m_item, "show_message_box",
                                      Q_RETURN_ARG(QVariant, returned_value));
        }
    }
}

void BackEnd::slot_copy_to_clipboard_pressed() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString text_for_clipboard = "";
    QAbstractListModel* qml_log_model = (m_item != nullptr ? m_item->findChild<QAbstractListModel*>("model_log") : nullptr);

    if (qml_log_model != nullptr) {
        for (int i = 0; i < qml_log_model->rowCount(); ++i) {
            const QString log_line = qml_log_model->data(qml_log_model->index(i, 0), 0).toString();
            text_for_clipboard += log_line + "\n";
        }
        clipboard->setText(text_for_clipboard);
    }
}

/* Вызывается из .qml - btnStop::onClicked */
void BackEnd::slot_stop_clicked() {
    write_inf_string("ControlFlags", "Enabled", "false");
    qDebug() << "The control flag 'Enabled' was set to false";
}

bool BackEnd::load_config(bool allow_missing) {
    m_config_file_.Reset();
    m_config_file_.SetUnicode();
    if (allow_missing && !QFile::exists(inf_file))
        return true;
    const SI_Error rc = m_config_file_.LoadFile(inf_file.toStdWString().c_str());
    if (rc < 0) {
        display_log_message("| Ошибка чтения INF: " + inf_file + "; код " + QString::number(rc));
        return false;
    }
    return true;
}

bool BackEnd::save_config() {
    std::string data;
    const SI_Error rc = m_config_file_.Save(data);
    QSaveFile file(inf_file);
    if (rc < 0 || !file.open(QIODevice::WriteOnly)
        || file.write(data.data(), static_cast<qint64>(data.size())) != static_cast<qint64>(data.size())
        || !file.commit()) {
        display_log_message("| Ошибка сохранения INF: " + inf_file + "; " + file.errorString());
        return false;
    }
    return true;
}

void BackEnd::delete_inf_section(const QString& section_name) {
    if (!load_config(true))
        return;
    m_config_file_.Delete(section_name.toStdWString().c_str(), nullptr);
    save_config();
}

void BackEnd::delete_inf_key_by_value(const QString& section_name, const QString& value) {

    const QKeysValues vsav_files_list = get_section_keys_and_values(section_name);

    QString key_to_remove = "";

    if (!vsav_files_list.empty())
        /* first - GUID, second - sav-file name  */
        foreach (const auto qpair, vsav_files_list) {
            if (value == qpair.second)
                key_to_remove = qpair.first;
        }

    if (key_to_remove != "") {
        if (!load_config())
            return;
        m_config_file_.Delete(section_name.toStdWString().c_str(), key_to_remove.toStdWString().c_str());
        save_config();
    }
}

void BackEnd::write_inf_string(const QString& section_name, const QString& key_name, const QString& value) {
    if (!load_config(true))
        return;
    const SI_Error rc = m_config_file_.SetValue(section_name.toStdWString().c_str(), key_name.toStdWString().c_str(), value.toStdWString().c_str());
    if (rc < 0) {
        display_log_message("| Ошибка изменения INF: " + inf_file + "; код " + QString::number(rc));
        return;
    }
    save_config();
}

QString BackEnd::read_inf_string(const QString& section_name, const QString& key_name) {
    if (!load_config(true))
        return {};
    return QString::fromWCharArray(m_config_file_.GetValue(section_name.toStdWString().c_str(), key_name.toStdWString().c_str(), L"ОШИБКА_ЧТЕНИЯ_ПУТИ_КАТАЛОГА"));
}

/* Для загрузки всего содержимого секции [SourceDisksFiles] */
QKeysValues BackEnd::get_section_keys_and_values(const QString& section_name) {
    QKeysValues rret;
    if (!load_config(true))
        return rret;
    if(!m_config_file_.SectionExists(section_name.toStdWString().c_str())) {
        const QKeysValues qkv_nullptr_list = {};
        return qkv_nullptr_list;
    }
    auto keys_n_values = m_config_file_.GetSection(section_name.toStdWString().c_str());
    foreach (const auto& key_val, *keys_n_values) {
        std::pair<QString, QString> qpair;
        qpair.first = QString::fromWCharArray(key_val.first.pItem).trimmed();
        qpair.second = QString::fromWCharArray(key_val.second).trimmed();
        rret.push_back(qpair);
    }
    return rret;
}

const bool BackEnd::str2bool(const QString& bool_as_str) {
    /* При ошибке разбора operator>> не всегда записывает значение, поэтому по умолчанию false */
    bool bret = false;
    std::istringstream(bool_as_str.toStdString()) >> std::boolalpha >> bret;
    return bret;
}

/*
 * Парсим в vector строки (вьюхи/площадки) вида
 * C:/Для экспорта IFC/АР3_проект.rvt = 3dViewNavisworks = Площадка1 = ВыходноеИмяФайла = jsonСконфигурацией = Экспорт?true/false
 */
std::vector<to_restore> BackEnd::get_all_keys_and_values_of_file(const QString& file_name) {
    to_restore line_to_restore;
    std::vector<to_restore> rret;

    QFile q_file(file_name);
    if(!q_file.open(QIODevice::ReadOnly)) {
        qDebug() << q_file.errorString();
    }

    QTextStream ss_in(&q_file);

    while(!ss_in.atEnd()) {
        const QString q_line = ss_in.readLine();
        if (q_line != "") {
            const QStringList fields = q_line.split("=");
            /* Пропускаем повреждённые строки, иначе fields.at() выходит за границы */
            if (fields.size() != 6) {
                qDebug() << "Skipping malformed line in" << file_name << ":" << q_line;
                continue;
            }
            line_to_restore.fname = fields.at(0).trimmed();
            line_to_restore.view = fields.at(1).trimmed();
            line_to_restore.site = fields.at(2).trimmed();
            line_to_restore.outputfname = fields.at(3).trimmed();
            line_to_restore.jsonpath = fields.at(4).trimmed();
            line_to_restore.should_exported = str2bool(fields.at(5).trimmed());
            rret.push_back(line_to_restore);
            line_to_restore  = {};
        }
    }
    q_file.close();
    return rret;
}

/* ф-ция bool2str - эдакая альтернатива std::boolalpha */
inline const char * const bool2str(bool bval){ return bval ? "true" : "false"; }

void BackEnd::slot_save_views_and_sites_to_file(
    const QString& fname,
    const QString& view_name,
    const QString& site_name,
    const QString& output_file_name,
    const QString& json_file_path,
    const bool should_be_exported,
    const int append_mode)
{
    QIODeviceBase::OpenModeFlag write_mode = QIODevice::WriteOnly;
    QFile sav_file(ifc_exporter_directory + "/" + views_and_sites_file);
    /* Если append_mode == -1, то очищаем файл */
    if (append_mode == -1)
        sav_file.resize(0);
    else {
        /* Если append_mode == 0, то создаем файл заново */
        if (append_mode)
            write_mode = QIODevice::Append;
        if (fname != "" && sav_file.open(write_mode)) {

            QTextStream text_stream(&sav_file);
            text_stream << fname.trimmed() << " = " <<
                           view_name.trimmed() << " = " <<
                           site_name.trimmed() << " = " <<
                           output_file_name.trimmed() << " = " <<
                           json_file_path.trimmed() << " = " <<
                           bool2str(should_be_exported) << "\n";

            sav_file.close();
        }
    }
    /* Путь передаём как std::filesystem::path: в нём может быть кириллица */
    remove_duplicate_lines_from_file(std::filesystem::path(sav_file.fileName().toStdWString()));
}

void BackEnd::on_sav_combo_changed(bool remove_from_combobox, const QString &file_name, QString full_name) {

delete_inf_section("SourceDisksFiles");

if (!remove_from_combobox){

    /* Если full_name пустое, значит просто выбрали другую позицию в combobox'e */
    if (full_name == "") {
        write_inf_string("Manufacturer", "sav_file_for_export", file_name);
        views_and_sites_file = file_name;
        load_sav_file_into_main_list(views_and_sites_file);
    }
    else /* иначе - добавили новый sav-файл */
    try {
        views_and_sites_file = full_name;

        v_sav_files.push_back(full_name.toStdString());
        QUuid random_guid = QUuid::createUuid();
        QString key_name = random_guid.toString();

        write_inf_string("Manufacturer", "sav_file_for_export", file_name);
        if (file_name != "views_sites.sav")
            write_inf_string("SourceDisksNames.amd64", key_name, file_name);

        load_sav_file_into_main_list(views_and_sites_file);
    }
    catch(std::exception ex) {
        qDebug() << ex.what();
    }
 }
/* remove_from_combobox = true, нажали "Удалить" конфигурацию */
else {
    delete_inf_key_by_value("SourceDisksNames.amd64", file_name);
    write_inf_string("Manufacturer", "sav_file_for_export", "views_sites.sav");
    load_sav_file_into_main_list("views_sites.sav");
 }
}

void BackEnd::load_sav_file_into_main_list(const QString &sav_file){

    QVariant returned_value;
    /* Очищаем в QML основной массив, содержащий вьюхи, 3D-виды etc */
    QMetaObject::invokeMethod(m_item, "clear_main_list",
                              Q_RETURN_ARG(QVariant, returned_value));

    const QKeysValues v_files_list = get_section_keys_and_values("SourceDisksFiles");
    to_restore line_to_restore;
    const std::vector<to_restore> v_views_n_sites = get_all_keys_and_values_of_file(ifc_exporter_directory + "/" + sav_file);

    int list_index_to_add = -1;

    if (!v_files_list.empty())
        /* first - fname, second - bool_as_str "should_be_exported?" */
        foreach (const auto qpair, v_files_list) {
            /* Добавляем в список обыкновенные RVT, без вьюх и площадок */
            const auto it = std::find_if(v_views_n_sites.begin(), v_views_n_sites.end(),
                                         [&qpair](const to_restore& item) {
                                             return item.fname == qpair.first; /* fname */
                                        });
            if (it == v_views_n_sites.end()) {
                auto fname = qpair.first;
                bool should_exported = str2bool(qpair.second);
                QMetaObject::invokeMethod(m_item, "add_row_from_cpp",
                                          Q_RETURN_ARG(QVariant, returned_value),
                                          Q_ARG(const QString&, fname),
                                          Q_ARG(const bool,    should_exported));
                ++list_index_to_add;
            }
        }

    QString last_added_fname = "";
    /* Добавляем в список всё остальное, эти файлы уже с указанными вьюхами или площадками */
    if(!v_views_n_sites.empty())
        foreach (const auto line_to_restore, v_views_n_sites) {
            if (last_added_fname == line_to_restore.fname)
                QMetaObject::invokeMethod(m_item, "add_subrow_wrapper",
                                          Q_RETURN_ARG(QVariant, returned_value),
                                          Q_ARG(const int, list_index_to_add),
                                          Q_ARG(const QString, line_to_restore.view),
                                          Q_ARG(const QString, line_to_restore.site),
                                          Q_ARG(const QString, line_to_restore.outputfname),
                                          Q_ARG(const QString, line_to_restore.jsonpath));
            else {
                ++list_index_to_add;
                QMetaObject::invokeMethod(m_item, "add_row_w_subrows_from_cpp",
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
}

void BackEnd::fill_combobox_sav_files() {
    QVariant returned_value;
    /* sav'ы добавляем в выпадающий список */
    if (!load_config(true))
        return;
    QString section_name = "SourceDisksNames.amd64";

    const CSimpleIniW::TKeyVal* sav_files = m_config_file_.GetSection(section_name.toStdWString().c_str());
    /* При первом запуске секции в .INF ещё нет */
    if (sav_files == nullptr)
        return;
    for (const auto& item : *sav_files) {
        const wchar_t* ws_sav_file_path = static_cast<const wchar_t*>(item.second);
        QString sav_file_path = QString::fromWCharArray(ws_sav_file_path);

        v_sav_files.push_back(sav_file_path.toStdString());
        const QFileInfo fi(sav_file_path);
        QMetaObject::invokeMethod(m_item, "add_item_cb_files_sav",
                                  Q_RETURN_ARG(QVariant, returned_value),
                                  Q_ARG(const QString&, fi.fileName()));
    }
}

void clr_logfile(const QString &fpath) {
    std::ofstream ofs;
    ofs.open(fpath.toStdString(), std::ofstream::out | std::ofstream::trunc);
    ofs.close();
}
void BackEnd::slot_clear_log_files() {
    const QString app_data = get_env("appdata");
    clr_logfile(app_data + qt_log);
    clr_logfile(app_data + addin_log);
    clr_logfile(app_data + bg_helper_log);
    display_log_message("| Файлы журналов очищены");
}

void BackEnd::report_launch_error(const QString& message) {
    display_log_message("| Запуск отменён: " + message);
    emit launch_failed();
}

void BackEnd::slot_run_clicked(const int right_now, const QString& utime, const QString& udate) {
    if (m_item == nullptr) {
        report_launch_error("не загружен интерфейс QML");
        return;
    }
    const auto* version = m_item->findChild<QQuickItem*>("combo_revit_version");
    const auto* ifc = m_item->findChild<QQuickItem*>("cbIFC");
    const auto* navi = m_item->findChild<QQuickItem*>("cbNavi");
    const auto* path = m_item->findChild<QQuickItem*>("text_IFCPath");
    const auto* files = m_item->findChild<QAbstractListModel*>("model_files");
    if (version == nullptr || ifc == nullptr || navi == nullptr || path == nullptr || files == nullptr || files->rowCount() == 0) {
        report_launch_error("не найдены элементы настроек или список файлов пуст");
        return;
    }
    if (!load_config(true)) {
        report_launch_error("не удалось прочитать INF");
        return;
    }

    /* Меняем конфигурацию в памяти и сохраняем целиком до отправки команды. */
    bool valid = true;
    const auto set = [this, &valid](const QString& section, const QString& key, const QString& value) {
        if (m_config_file_.SetValue(section.toStdWString().c_str(), key.toStdWString().c_str(), value.toStdWString().c_str()) < 0)
            valid = false;
    };
    m_config_file_.Delete(L"SourceDisksFiles", nullptr);
    set("ControlFlags", "Time", right_now ? QString() : utime);
    set("ControlFlags", "Date", right_now ? QString() : udate);
    set("ControlFlags", "RevitVersion", version->property("currentText").toString());
    set("RVT", "IFC", ifc->property("checked").toBool() ? "true" : "false");
    set("RVT", "NWC", navi->property("checked").toBool() ? "true" : "false");
    set("DestinationDirs", "DefaultDestDir", path->property("text").toString());
    for (int i = 0; i < files->rowCount(); ++i)
        set("SourceDisksFiles", files->data(files->index(i, 0), 0).toString().trimmed(), files->data(files->index(i, 0), 1).toString());
    set("ControlFlags", "Enabled", "true");
    if (!valid || !save_config()) {
        report_launch_error("не удалось сохранить настройки экспорта");
        return;
    }
    if (right_now) {
        if (m_pipe_client_ == nullptr) {
            m_pipe_client_ = new pipe_client("\\\\.\\pipe\\bghelperpipe", this);
            connect(m_pipe_client_, &pipe_client::send_failed, this, [this](const QString& message) {
                write_inf_string("ControlFlags", "Enabled", "false");
                report_launch_error(message);
            });
        }
        m_pipe_client_->send_message_to_server("START_IMMEDIATELY");
    }
}

void BackEnd::slot_esc_pressed() {
    /* TODO: добавить вопросительное окно? */
    m_window_->exit(0);
}

void BackEnd::slot_window_blink() {
    m_qview_->requestActivate();
}


void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg){
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("export: %1").arg(msg);
        break;
    /*  case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    */
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        abort();
    }
    const QString file_name{"qt.log"};
    const QString app_data = get_env("appdata");

    if (app_data.isEmpty()) {
        qFatal("Unable to find appData directory!");
    }
    const QString log_file_location = app_directory +"\\" + file_name;
    QFile out_file(app_data + log_file_location);

    if (txt != "" && !msg.startsWith("QML Debugger: Waiting for connection on port") ) {
        out_file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ss_ts(&out_file);
        ss_ts << QTime::currentTime().toString() << " " << txt << Qt::endl;
    }
}

/*
void BackEnd::slot_btn_ifc_settings_clicked() {
    Передаем окну IFCSettings наш handle, чтобы диалог ifc_settings показался модально
    QStringList args;

    args.append(QString::fromStdString(m_str_hwnd));
    QProcess::startDetached("ifcsettings.exe", args);
}
*/

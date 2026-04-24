/* last change 24.4.2026, removed 60-chars dividing */

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
#include <QThread>
#include <sstream>
#include "named_pipe_client.h"

/*
void BackEnd::slot_btn_ifc_settings_clicked()
{
    Передаем окну IFCSettings наш handle, чтобы ifc_settings показался модально
    QStringList args;

    args.append(QString::fromStdString(m_str_hwnd));
    QProcess::startDetached("ifcsettings.exe", args);
}

std::list<QString> BackEnd::get_all_keys_of_section(const QString section_name)
{
    std::list<QString> rret;
    m_config_file_.SetUnicode();
    CSimpleIniW::TNamesDepend key_list;
    const SI_Error rc = m_config_file_.LoadFile(inf_file.toStdString().c_str());
    const bool res = m_config_file_.GetAllKeys(section_name.toStdWString().c_str(), key_list);
    foreach (const auto key, key_list) {
        rret.push_back(QString::fromWCharArray(key.pItem).trimmed());
    }
    return rret;
}
*/

BackEnd::BackEnd(QGuiApplication *parent, QObject* item, HWND hwnd)
{
    m_window = parent;
    m_item = item;
    m_hwnd = hwnd;

    const std::uint32_t int_hwnd = reinterpret_cast<std::uint32_t>(m_hwnd);

    std::stringstream ss;

    ss << std::hex << int_hwnd;
    ss >> m_str_hwnd;

    QThread* q_thread = new QThread();

    m_server = new QTcpServer(this);
    m_server->moveToThread(q_thread);

    if (!m_server->listen(QHostAddress::Any, 6667))
    {
        qDebug() << "Unable to start QTcp server: " << m_server->errorString();
        m_server->close();
    }
    else
    {
        qDebug() << "QTcp server started";
        connect(this, &BackEnd::new_message, this, &BackEnd::display_log_message);
        connect(m_server, &QTcpServer::newConnection, this, &BackEnd::new_socket_connection);
    }
}

BackEnd::~BackEnd()
{
    /* configFile.Reset(); */
}

void BackEnd::new_socket_connection()
{
    while (m_server->hasPendingConnections())
        append_to_socket_list(m_server->nextPendingConnection());
}

void BackEnd::append_to_socket_list(QTcpSocket* socket)
{
    m_connection_set_.insert(socket);
    connect(socket, &QTcpSocket::readyRead, this, &BackEnd::read_socket);
    connect(socket, &QTcpSocket::disconnected, this, &BackEnd::discard_socket);
    connect(socket, &QAbstractSocket::errorOccurred, this, &BackEnd::display_error);

    display_log_message(QString("| Подготовка к выгрузке ") + QString::fromStdString(m_str_hwnd));
    display_log_message(QString("| Фоновой процесс %1 запуска Revit подключен!").arg(socket->socketDescriptor()));
}

void BackEnd::discard_socket()
{
    QTcpSocket* q_socket = reinterpret_cast<QTcpSocket*>(sender());
    const QSet<QTcpSocket*>::iterator it = m_connection_set_.find(q_socket);
    if (it != m_connection_set_.end()){
/*      displayMessage(QString("INFO :: A client : %1 has just left").arg(socket->socketDescriptor())); */
        m_connection_set_.remove(*it);
    }

    q_socket->deleteLater();
}

void BackEnd::read_socket()
{
    QTcpSocket* q_socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray qmessage = q_socket->readAll(); // Read message
    qDebug() << "bg | " << QString(qmessage);

    const std::string display_log_msg = QString(qmessage).toStdString();

    display_log_message("bgHelper | " + QString::fromStdString(display_log_msg));

    const int charCount = 60;  /* split 60 chars */
    /*
    for (size_t i = 0; i < display_log_msg.length(); i += charCount)
    {
        const std::string to_display_str = display_log_msg.substr(i, charCount);
        display_log_message("bgHelper | " + QString::fromStdString(to_display_str));
    }
    */
}

void BackEnd::display_error(QAbstractSocket::SocketError socket_error)
{
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

/* Adds and displays a message in the LOG listbox */
void BackEnd::display_log_message(const QString& qstr_msg)
{
    const QQuickItem* lv_log = m_item->findChild<QQuickItem*>("o_lvLog");
    QObject* lm_log = lv_log->children()[1];
    QVariant returned_value;
    const QVariant lm_msg = qstr_msg;
    QMetaObject::invokeMethod(lm_log, "add_to_log_listview",  /* add_row function defined in QML-file */
                              Q_RETURN_ARG(QVariant, returned_value),
                              Q_ARG(QVariant, lm_msg));

    if (qstr_msg.startsWith("bgHelper | End of export"))
    {
        QMetaObject::invokeMethod(m_item, "stop_clicked",
                                  Q_RETURN_ARG(QVariant, returned_value));
    }
}

void BackEnd::slot_copy_to_clipboard_pressed()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    QString text_for_clipboard = "";
    const QQuickItem* lv_log = m_item->findChild<QQuickItem*>("o_lvLog");
    QObject* lm_log = lv_log->children()[1];
    QAbstractListModel* qml_log_model = qobject_cast<QAbstractListModel*>(lm_log);

    if (qml_log_model != nullptr)
    {
        for (int i = 0; i < qml_log_model->rowCount(); ++i)
        {
            const QString log_line = qml_log_model->data(qml_log_model->index(i, 0), 0).toString();
            text_for_clipboard += log_line + "\n";
        }
        clipboard->setText(text_for_clipboard);
    }
}

/* Вызывается из .qml - btnStop::onClicked */
void BackEnd::slot_stop_clicked()
{
    write_inf_string("ControlFlags", "Enabled", "false");
    qDebug() << "The control flag 'Enabled' was set to false";
}

void BackEnd::delete_inf_section(const QString& section_name)
{
    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    m_config_file_.SetUnicode();
    SI_Error rc = m_config_file_.LoadFile(inf_file.toStdString().c_str());
    /* if (rc < 0){ qDebug() << "Cannot open INF-file " << infFile; }; */

    m_config_file_.Delete(section_name.toStdWString().c_str(), nullptr);
    rc = m_config_file_.SaveFile(inf_file.toStdString().c_str(), false);
}

void BackEnd::write_inf_string(const QString& section_name, const QString& key_name, const QString& value)
{
    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    m_config_file_.SetUnicode();
    SI_Error rc = m_config_file_.LoadFile(inf_file.toStdString().c_str());
    /* if (rc < 0){ qDebug() << "Cannot open INF-file " << infFile; }; */

    /* Такой wrapper получился после замены WritePrivateProfileStringW на ф-ции simpleini */
    m_config_file_.SetValue(section_name.toStdWString().c_str(), key_name.toStdWString().c_str(), value.toStdWString().c_str());
    rc = m_config_file_.SaveFile(inf_file.toStdString().c_str(), false);
}

QString BackEnd::read_inf_string(const QString& section_name, const QString& key_name)
{
    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    m_config_file_.SetUnicode();
    SI_Error rc = m_config_file_.LoadFile(inf_file.toStdString().c_str());
    /* if (rc < 0){ qDebug() << "Cannot open INF-file " << infFile; }; */

    const auto wc_val = m_config_file_.GetValue(section_name.toStdWString().c_str(), key_name.toStdWString().c_str(), L"ОШИБКА_ЧТЕНИЯ_ПУТИ_КАТАЛОГА");
    const QString q_ret = QString::fromWCharArray(wc_val);
    return q_ret;
}

/* Для загрузки всего содержимого секции [SourceDisksFiles] */
QKeysValues BackEnd::get_section_keys_and_values(const QString& section_name)
{
    QKeysValues rret;
    m_config_file_.SetUnicode();

    qDebug() << "Checking if the section [SourceDisksFiles] exists in the .inf file";
    const SI_Error rc = m_config_file_.LoadFile(inf_file.toStdString().c_str());
    if(!m_config_file_.SectionExists(section_name.toStdWString().c_str())){
        const QKeysValues qkv_nullptr_list = {};
        return qkv_nullptr_list;
    }
    auto keys_n_values = m_config_file_.GetSection(section_name.toStdWString().c_str());
    foreach (const auto& key_val, *keys_n_values)
    {
        std::pair<QString, QString> qpair;
        qpair.first = QString::fromWCharArray(key_val.first.pItem).trimmed();
        qpair.second = QString::fromWCharArray(key_val.second).trimmed();
        rret.push_back(qpair);
    }
    return rret;
}

const bool BackEnd::str2bool(const QString& bool_as_str)
{
    bool bret;
    std::istringstream(bool_as_str.toStdString()) >> std::boolalpha >> bret;
    return bret;
}

/* Парсим в vec строки (вьюхи/площадки) вида C:/Для экспорта IFC/АР3_проект.rvt = 3dViewNavisworks = Площадка1 */
std::vector<to_restore> BackEnd::get_all_keys_and_values_of_file(const QString& file_name)
{
    to_restore line_to_restore;
    std::vector<to_restore> rret;

    QFile q_file(file_name);
    if(!q_file.open(QIODevice::ReadOnly)) {
        qDebug() << q_file.errorString();
    }

    QTextStream ss_in(&q_file);

    while(!ss_in.atEnd()) {
        const QString q_line = ss_in.readLine();
        QStringList fields = q_line.split("=");
        line_to_restore.fname = fields.at(0).trimmed();
        line_to_restore.view = fields.at(1).trimmed();
        line_to_restore.site = fields.at(2).trimmed();
        line_to_restore.outputfname = fields.at(3).trimmed();
        line_to_restore.jsonpath = fields.at(4).trimmed();
        line_to_restore.should_exported = str2bool(fields.at(5).trimmed());
        rret.push_back(line_to_restore);
        line_to_restore  = {};
    }
    q_file.close();
    return rret;
}

/* ф-ция boolToString - эдакая альтернатива std::boolalpha */
inline const char * const bool2str(bool bval){ return bval ? "true" : "false"; }

void BackEnd::slot_save_views_and_sites_to_file(const QString& fname, const QString& view_name, const QString& site_name,
                                                const QString& output_file_name, const QString& json_file_path, const bool should_be_exported, const int append_mode)
{
    QIODeviceBase::OpenModeFlag write_mode = QIODevice::WriteOnly;
    QFile sav_file(views_and_sites_file);
    /* Если AppendMode == -1, то очищаем файл */
    if (append_mode == -1)
        sav_file.resize(0);
    else
    {
        /* Если AppendMode == 0, то создаем файл заново */
        if (append_mode)
            write_mode = QIODevice::Append;
        if (sav_file.open(write_mode)) {
            QTextStream text_stream(&sav_file);
            text_stream << fname.trimmed() << " = " << view_name.trimmed() << " = " << site_name.trimmed() << " = " << output_file_name.trimmed() << " = " << json_file_path.trimmed() << " = " << bool2str(should_be_exported) << "\n";
            sav_file.close();
        }
    }
}

void BackEnd::slot_is_file_exists(QString fname, const QString& rvt_version)
{
    QStringList args;
    args.append(fname);
    args.append(rvt_version);

    const std::uint32_t int_hwnd = reinterpret_cast<std::uint32_t>(m_hwnd);

    std::stringstream ss;
    std::string s_hwnd;
    ss << int_hwnd;
    ss >> s_hwnd;

    args.append(QString::fromStdString(s_hwnd));
    QProcess::startDetached("rvthelper.exe", args);

    const QQuickItem* lv_main = m_item->findChild<QQuickItem*>("o_lvMain");
    QObject* list_model = lv_main->children()[1];

    /* После закрытия общего доступа к папке по сети, замены в RSN:// неактуальны
    if (fname.startsWith("RSN://"))
    {
        fname.replace("RSN://", "\\\\");
        int spos = fname.indexOf("/");
        const QString server_name = fname.mid(2, spos-2);
        fname.replace("\\\\" + server_name + "/", "\\\\" + server_name + "\\Revit23\\");
    }
    else if (QFile::exists(fname)){ }; */

    QVariant returned_value;
    const QVariant bool_msg = true;
    QMetaObject::invokeMethod(list_model, "remove_last_row",
                              Q_RETURN_ARG(QVariant, returned_value),
                              Q_ARG(QVariant, bool_msg));
}

void BackEnd::slot_run_clicked(const int right_now, const QString& utime, const QString& udate)
{
    delete_inf_section("SourceDisksFiles");

    write_inf_string("ControlFlags", "Time", utime);
    write_inf_string("ControlFlags", "Date", udate);

    const QQuickItem* qcb_rvt_version = m_item->findChild<QQuickItem*>("row_RvtVersion");
    QObject* cb_rvt_version = qcb_rvt_version->children()[1];
    const QString revit_version = cb_rvt_version->property("currentText").toString();
    write_inf_string("ControlFlags", "RevitVersion", revit_version);

    const QQuickItem* qcheckbox_ifc = m_item->findChild<QQuickItem*>("cbIFC");
    bool is_checkbox_ifc_checked = qcheckbox_ifc->property("checked").toBool();
    const QString s_ifc_checked = QVariant(is_checkbox_ifc_checked).toString();
    write_inf_string("RVT", "IFC", s_ifc_checked);

    const QQuickItem* checkbox_navi = m_item->findChild<QQuickItem*>("cbNavi");
    bool is_checkbox_navi_checked = checkbox_navi->property("checked").toBool();
    const QString s_navi_checked = QVariant(is_checkbox_navi_checked).toString();
    write_inf_string("RVT", "NWC", s_navi_checked);

    const QQuickItem* qquick_text_ifc_path = m_item->findChild<QQuickItem*>("text_IFCPath");
    const QString s_ifc_path = qquick_text_ifc_path->property("text").toString();
    write_inf_string("DestinationDirs", "DefaultDestDir", s_ifc_path);

    /* 24.04.2025 Выбор версии IFC перенесен в отд. программу/окно
     *
     * QQuickItem* qtextIFCVers = m_item->findChild<QQuickItem*>("row_IFCVersion");
     * QObject* cbIFCvers = qtextIFCVers->children()[1];
     * QString ifcVersion = cbIFCvers->property("currentText").toString();
     * WriteInfString("ControlFlags", "IFCVersion", ifcVersion);            */

    const QQuickItem* lv_main = m_item->findChild<QQuickItem*>("o_lvMain");
    QObject* list_model = lv_main->children()[1];
    QAbstractListModel* qml_list_model = qobject_cast<QAbstractListModel*>(list_model);

    if (qml_list_model != nullptr)
    {
        for (int i = 0; i < qml_list_model->rowCount(); ++i)
        {
            const QString rvt_file_name = qml_list_model->data(qml_list_model->index(i, 0), 0).toString();
            const QString should_be_exported = qml_list_model->data(qml_list_model->index(i, 0), 1).toString();
            write_inf_string("SourceDisksFiles", rvt_file_name.trimmed(), should_be_exported);
        }
    }
    else
    {
        qDebug() << "Getting *.RVT files list is failed!";
    }

    write_inf_string("ControlFlags", "Enabled", "true");
    qDebug() << "The control flag 'Enabled' was set to true";

    if(right_now)    /* Запустить ПРЯМО сейчас! == 1 */
    {
        pipe_client* local_pipe = new pipe_client("\\\\.\\pipe\\bghelperpipe", this);
        local_pipe->send_message_to_server("START_IMMEDIATELY");
    }
}

void BackEnd::slot_esc_pressed()
{
    m_window->exit(0);
}

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type)
    {
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
    const QString file_name{"alabuga.q.log"};
    const QString app_data = getenv("appdata");

    if (app_data.isEmpty()) {
        qFatal("Unable to find appData directory!");
    }
    const QString log_file_location = "\\alabuga_dev\\" + file_name;
    QFile out_file(app_data + log_file_location);

    if (txt != "" && !msg.startsWith("QML Debugger: Waiting for connection on port") )
    {
        out_file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ss_ts(&out_file);
        ss_ts << QTime::currentTime().toString() << " " << txt << Qt::endl;
    }
}




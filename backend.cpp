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
#include "localsocketipcclient.h"

/*
void BackEnd::slotBtnIFCSettingsClicked()
{
    Передаем окну IFCSettings наш handle, чтобы ifcSettings показался модально
    QStringList args;

    args.append(QString::fromStdString(m_str_hwnd));
    QProcess::startDetached("ifcsettings.exe", args);
}
*/

BackEnd::BackEnd(QGuiApplication *parent, QObject* item, HWND hWnd)
{
    m_Window = parent;
    m_item = item;
    m_hwnd = hWnd;

    const std::uint32_t intHwnd = reinterpret_cast<std::uint32_t>(m_hwnd);

    std::stringstream ss;

    ss << std::hex << intHwnd;
    ss >> m_str_hwnd;

    QThread* cThread = new QThread();

    m_server = new QTcpServer(this);
    m_server->moveToThread(cThread);

    if (!m_server->listen(QHostAddress::Any, 6667))
    {
        qDebug() << "Unable to start QTcp server: " << m_server->errorString();
        m_server->close();
    }
    else
    {
        qDebug() << "QTcp server started";
        connect(this, &BackEnd::newMessage, this, &BackEnd::displayMessage);
        connect(m_server, &QTcpServer::newConnection, this, &BackEnd::newSocketConnection);
    }
}

BackEnd::~BackEnd()
{
    /* configFile.Reset(); */
}

void BackEnd::newSocketConnection()
{
    while (m_server->hasPendingConnections())
        appendToSocketList(m_server->nextPendingConnection());
}

void BackEnd::appendToSocketList(QTcpSocket* socket)
{
    connection_set.insert(socket);
    connect(socket, &QTcpSocket::readyRead, this, &BackEnd::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &BackEnd::discardSocket);
    connect(socket, &QAbstractSocket::errorOccurred, this, &BackEnd::displayError);
    // ui->comboBox_receiver->addItem(QString::number(socket->socketDescriptor()));
    displayMessage(QString("| Подготовка к выгрузке ") + QString::fromStdString(m_str_hwnd));
    displayMessage(QString("| Фоновой процесс %1 запуска Revit подключен!").arg(socket->socketDescriptor()));
//   socket->write("Sending msg to bgHelper");
}

void BackEnd::discardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    const QSet<QTcpSocket*>::iterator it = connection_set.find(socket);
    if (it != connection_set.end()){
/*      displayMessage(QString("INFO :: A client : %1 has just left").arg(socket->socketDescriptor())); */
        connection_set.remove(*it);
    }

    socket->deleteLater();
}

void BackEnd::readSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray message = socket->readAll(); // Read message
    qDebug() << "bg | " << QString(message);

    const std::string dispLogMsg = QString(message).toStdString();
    const int charCount = 60;  /* split 60 chars */
    for (size_t i = 0; i < dispLogMsg.length(); i += charCount)
    {
        const std::string toDispStr = dispLogMsg.substr(i, charCount);
        displayMessage("bgHelper | " + QString::fromStdString(toDispStr));
    }
}

void BackEnd::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug() << "The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.";
        break;
    default:
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        qDebug() << "The following error occurred: %1." << socket->errorString();
        break;
    }
}

void BackEnd::displayMessage(const QString& str)
{
    //ui->textBrowser_receivedMessages->append(str);
    const QQuickItem* lvLog = m_item->findChild<QQuickItem*>("o_lvLog");
    QObject* lmLog = lvLog->children()[1];
//  QAbstractListModel* qLmLog = qobject_cast<QAbstractListModel*>(lmLog);
    QVariant returnedValue;
    const QVariant lmMsg = str;
    QMetaObject::invokeMethod(lmLog, "addRow",  /* addRow function defined in QML-file */
                              Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, lmMsg));

    if (str.startsWith("bgHelper | End of export"))
    {
        QMetaObject::invokeMethod(m_item, "stopClicked",
                                  Q_RETURN_ARG(QVariant, returnedValue));
    }
}

/* Вызывается из .qml - btnStop::onClicked */
void BackEnd::slotStopClicked()
{
    WriteInfString("ControlFlags", "Enabled", "false");
    qDebug() << "The control flag 'Enabled' was set to false";
}

void BackEnd::DeleteInfSection(const QString sectionName)
{
    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    configFile.SetUnicode();
    SI_Error rc = configFile.LoadFile(infFile.toStdString().c_str());
    /* if (rc < 0){ qDebug() << "Cannot open INF-file " << infFile; }; */
    configFile.Delete(sectionName.toStdWString().c_str(), nullptr);
    rc = configFile.SaveFile(infFile.toStdString().c_str(), false);
}

void BackEnd::WriteInfString(const QString sectionName, const QString keyName, const QString value)
{
    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    configFile.SetUnicode();
    SI_Error rc = configFile.LoadFile(infFile.toStdString().c_str());
    /* if (rc < 0){ qDebug() << "Cannot open INF-file " << infFile; }; */
    /* Такой wrapper получился после замены WritePrivateProfileStringW на ф-ции simpleini */
    configFile.SetValue(sectionName.toStdWString().c_str(), keyName.toStdWString().c_str(), value.toStdWString().c_str());
    rc = configFile.SaveFile(infFile.toStdString().c_str(), false);
}

QString BackEnd::ReadInfString(const QString sectionName, const QString keyName)
{
    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    configFile.SetUnicode();
    SI_Error rc = configFile.LoadFile(infFile.toStdString().c_str());
    /* if (rc < 0){ qDebug() << "Cannot open INF-file " << infFile; }; */
    const auto wc_Val = configFile.GetValue(sectionName.toStdWString().c_str(), keyName.toStdWString().c_str(), L"ОШИБКА_ЧТЕНИЯ_ПУТИ_КАТАЛОГА");
    const QString s_Ret = QString::fromWCharArray(wc_Val);
    return s_Ret;
}

/* Для загрузки всего содержимого секции [SourceDisksFiles] */
std::list<QString> BackEnd::GetAllKeysOfSection(const QString sectionName)
{
    std::list<QString> rret;
    configFile.SetUnicode();
    CSimpleIniW::TNamesDepend keyList;
    const SI_Error rc = configFile.LoadFile(infFile.toStdString().c_str());

    const bool res = configFile.GetAllKeys(sectionName.toStdWString().c_str(), keyList);
    foreach (const auto key, keyList) {
        rret.push_back(QString::fromWCharArray(key.pItem).trimmed());
    }
    return rret;
}

/* Парсим в vec строки (вьюхи/площадки) вида C:/Для экспорта IFC/АР3_проект.rvt = 3dViewNavisworks = Площадка1 */
std::vector<toRestore> BackEnd::GetAllKeysAndValuesOfFile(const QString fileName)
{
    toRestore lineToRestore;
    std::vector<toRestore> rret;

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        const QString line = in.readLine();
        QStringList fields = line.split("=");
        lineToRestore.fname = fields.at(0).trimmed();
        lineToRestore.view = fields.at(1).trimmed();
        lineToRestore.site = fields.at(2).trimmed();
        lineToRestore.outputfname = fields.at(3).trimmed();
        lineToRestore.jsonpath = fields.at(4).trimmed();
        rret.push_back(lineToRestore);
        lineToRestore  = {};
    }
    file.close();
    return rret;
}

void BackEnd::slotSaveViewAndSiteToFile(const QString fName, const QString viewName, const QString siteName, const QString outputFileName, const QString jsonFilePath, const int appendMode)
{
    QIODeviceBase::OpenModeFlag writeMode = QIODevice::WriteOnly;
    QFile sav_file(viewsAndSitesFile);
    /* Если AppendMode == -1, то очищаем файл */
    if (appendMode == -1)
        sav_file.resize(0);
    else
    {
        /* Если AppendMode == 0, то создаем файл заново */
        if (appendMode)
            writeMode = QIODevice::Append;
        if (sav_file.open(writeMode)) {
            QTextStream stream(&sav_file);
            stream << fName << " = " << viewName << " = " << siteName << " = " << outputFileName << " = " << jsonFilePath << "\n";
            sav_file.close();
        }
    }
}

void BackEnd::slotIsFileExists(QString fname, const QString rvtVersion)
{
    QStringList args;
    args.append(fname);
    args.append(rvtVersion);

    const std::uint32_t intHwnd = reinterpret_cast<std::uint32_t>(m_hwnd);

    std::stringstream ss;
    std::string s_hwnd;
    ss << intHwnd;
    ss >> s_hwnd;

    args.append(QString::fromStdString(s_hwnd));
    QProcess::startDetached("rvthelper.exe", args);

    const QQuickItem* lvMain = m_item->findChild<QQuickItem*>("o_lvMain");
    QObject* listModel = lvMain->children()[1];

    /* После закрытия общего доступа к папке по сети, замены в RSN:// неактуальны */
    if (fname.startsWith("RSN://"))
    {
        fname.replace("RSN://", "\\\\");
        int spos = fname.indexOf("/");
        const QString serverName = fname.mid(2, spos-2);
        fname.replace("\\\\" + serverName + "/", "\\\\" + serverName + "\\Revit23\\");        
    }
    else if (QFile::exists(fname)){ };

    QVariant returnedValue;
    const QVariant boolMsg = true;
    QMetaObject::invokeMethod(listModel, "removeLastRow",
                              Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, boolMsg));
}

void BackEnd::slotRunClicked(const int rightNow, const QString &utime, const QString &udate)
{
    DeleteInfSection("SourceDisksFiles");

    WriteInfString("ControlFlags", "Time", utime);
    WriteInfString("ControlFlags", "Date", udate);

    const QQuickItem* qcbRvtVers = m_item->findChild<QQuickItem*>("row_RvtVersion");
    QObject* cb_RvtVers = qcbRvtVers->children()[1];
    const QString revitVersion = cb_RvtVers->property("currentText").toString();
    WriteInfString("ControlFlags", "RevitVersion", revitVersion);

    const QQuickItem* qcheckboxIFC = m_item->findChild<QQuickItem*>("cbIFC");
    bool IsCheckboxIFC_checked = qcheckboxIFC->property("checked").toBool();
    const QString sIFC_checked = QVariant(IsCheckboxIFC_checked).toString();
    WriteInfString("RVT", "IFC", sIFC_checked);

    const QQuickItem* checkboxNavi = m_item->findChild<QQuickItem*>("cbNavi");
    bool IsCheckboxNavi_checked = checkboxNavi->property("checked").toBool();
    const QString sNavi_checked = QVariant(IsCheckboxNavi_checked).toString();
    WriteInfString("RVT", "NWC", sNavi_checked);

    const QQuickItem* QQuickText_IFCPath = m_item->findChild<QQuickItem*>("text_IFCPath");
    const QString sIFCPath = QQuickText_IFCPath->property("text").toString();
    WriteInfString("DestinationDirs", "DefaultDestDir", sIFCPath);

    /* 24.04.2025 Выбор версии IFC перенесен в отд. программу/окно  */
    /*
    /* QQuickItem* qtextIFCVers = m_item->findChild<QQuickItem*>("row_IFCVersion");
    /* QObject* cbIFCvers = qtextIFCVers->children()[1];
    /* QString ifcVersion = cbIFCvers->property("currentText").toString();
    /* WriteInfString("ControlFlags", "IFCVersion", ifcVersion);    */

    const QQuickItem* lvMain = m_item->findChild<QQuickItem*>("o_lvMain");
    const QObject* listModel = lvMain->children()[1];
    const QAbstractListModel* qmlListModel = qobject_cast<QAbstractListModel*>(listModel);

    if (qmlListModel != nullptr)
    {
        for (int i = 0; i < qmlListModel->rowCount(); ++i)
        {
            const QString rvtFileName = qmlListModel->data(qmlListModel->index(i, 0), 0).toString();
            WriteInfString("SourceDisksFiles", rvtFileName, "");
        }
    }
    else
    {
        qDebug() << "Getting *.RVT files list is failed!";
    }

    WriteInfString("ControlFlags", "Enabled", "true");
    qDebug() << "The control flag 'Enabled' was set to true";

    if(rightNow)    /* Запустить ПРЯМО сейчас! == 1 */
    {
        LocalSocketIpcClient* lp = new LocalSocketIpcClient("\\\\.\\pipe\\bghelperpipe", this);
        lp->send_MessageToServer("START_IMMEDIATELY");
    }
}


void BackEnd::slotEscPressed()
{
    m_Window->exit(0);
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
    const QString fileName{"alabuga.q.log"};
    const QString appData = getenv("appdata");

    if (appData.isEmpty()) {
        qFatal("Unable to find appData directory!");
    }
    const QString logFileLocation = "\\alabuga_dev\\" + fileName;
    QFile outFile(appData + logFileLocation);

    if (txt != "" && !msg.startsWith("QML Debugger: Waiting for connection on port") )
    {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << QTime::currentTime().toString() << " " << txt << Qt::endl;
    }
}




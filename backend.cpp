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

void BackEnd::slotBtnIFCSettingsClicked()
{
    QStringList args;
    std::uint32_t intHwnd = reinterpret_cast<std::uint32_t>(m_hwnd);

    std::stringstream ss;
    std::string s_hwnd;
    ss << std::hex << intHwnd;
    ss >> s_hwnd;

    args.append(QString::fromStdString(s_hwnd));
    QProcess::startDetached("ifcsettings.exe", args);
}

BackEnd::BackEnd(QGuiApplication *parent, QObject* item, HWND hWnd)
{
    m_Window = parent;
    m_item = item;
    m_hwnd = hWnd;

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
    displayMessage(QString("Фоновой процесс %1 запуска Revit подключен!").arg(socket->socketDescriptor()));
//    socket->write("Sending msg to bgHelper");
}

void BackEnd::discardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_set.find(socket);
    if (it != connection_set.end()){
//        displayMessage(QString("INFO :: A client : %1 has just left").arg(socket->socketDescriptor()));
        connection_set.remove(*it);
    }

    socket->deleteLater();
}

void BackEnd::readSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray message = socket->readAll(); // Read message
    qDebug() << "bgHelper | " << QString(message);

    std::string dispLogMsg = QString(message).toStdString();
    int charCount = 60;  /* split 60 chars */
    for (size_t i = 0; i < dispLogMsg.length(); i += charCount)
    {
        std::string toDispStr = dispLogMsg.substr(i, charCount);
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
    QQuickItem* lvLog = m_item->findChild<QQuickItem*>("o_lvLog");
    QObject* lmLog = lvLog->children()[1];
    QAbstractListModel* qLmLog = qobject_cast<QAbstractListModel*>(lmLog);
    QVariant returnedValue;
    QVariant lmMsg = str;
    QMetaObject::invokeMethod(lmLog, "addRow",
                              Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, lmMsg));
}

void BackEnd::slotStopClicked()
{
    WriteInfString("ControlFlags", "runNow", "false");
    qDebug() << "The control flag 'runNow' was set to false";

}

void BackEnd::DeleteInfSection(QString sectionName)
{
    LPCWSTR wsSection = (const wchar_t*) sectionName.utf16();
    WritePrivateProfileStringW(wsSection, NULL, NULL, infName);
    wsSection = nullptr;
}

void BackEnd::AppendInfSection(QString sectionName)
{
    QFile f(infFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QTextStream outInf(&f);
        outInf << "[" << sectionName << "]" << "\n";
    }
    f.flush();
    f.close();
}

void BackEnd::AddInfString(QString keyAsValue)
{
    QFile f(infFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {
        QTextStream outInf(&f);        
        outInf << keyAsValue << "\n";
    }
    f.flush();
    f.close();
}

void BackEnd::WriteInfString(QString sectionName, QString keyName, QString value)
{
    LPCWSTR wsSection = (const wchar_t*) sectionName.utf16();
    LPCWSTR wsKey = (const wchar_t*) keyName.utf16();
    LPCWSTR wsValue = (const wchar_t*) value.utf16();
    WritePrivateProfileStringW(wsSection, wsKey, wsValue, infName);
    wsSection = wsKey = wsValue = nullptr;
}

QString BackEnd::ReadInfString(QString sectionName, QString keyName)
{
    wchar_t wc_Val[_MAX_FNAME] = L"";

    LPCWSTR wsSection = (const wchar_t*) sectionName.utf16();
    LPCWSTR wsKey = (const wchar_t*) keyName.utf16();
    GetPrivateProfileStringW(wsSection, wsKey, nullptr, wc_Val, std::size(wc_Val), infName);
    QString s_Ret = QString::fromWCharArray(wc_Val);
    wsSection = wsKey = nullptr;
    return s_Ret;
}

void BackEnd::slotIsFileExists(QString fname, QString rvtVersion)
{
    QStringList args;
    args.append(fname);
    args.append(rvtVersion);

    std::uint32_t intHwnd = reinterpret_cast<std::uint32_t>(m_hwnd);

    std::stringstream ss;
    std::string s_hwnd;
    ss << intHwnd;
    ss >> s_hwnd;

    args.append(QString::fromStdString(s_hwnd));
    QProcess::startDetached("rvtversion.exe", args);

    QQuickItem* lvMain = m_item->findChild<QQuickItem*>("o_lvMain");
    QObject* listModel = lvMain->children()[1];

    if (fname.startsWith("RSN://"))
    {
        fname.replace("RSN://", "\\\\");
        int spos = fname.indexOf("/");
        QString serverName = fname.mid(2, spos-2);
        fname.replace("\\\\" + serverName + "/", "\\\\" + serverName + "\\Revit23\\");        
    }
    else if (QFile::exists(fname)){ };

    QVariant returnedValue;
    QVariant boolMsg = true;
    QMetaObject::invokeMethod(listModel, "removeLastRow",
                              Q_RETURN_ARG(QVariant, returnedValue),
                              Q_ARG(QVariant, boolMsg));
}


void BackEnd::slotRunClicked(const QString &utime, const int rightNow)
{
    DeleteInfSection("SourceDisksFiles");
    AppendInfSection("SourceDisksFiles");

    WriteInfString("ControlFlags", "Time", utime);

    QQuickItem* qcbRvtVers = m_item->findChild<QQuickItem*>("row_RvtVersion");
    QObject* cb_RvtVers = qcbRvtVers->children()[1];
    QString revitVersion = cb_RvtVers->property("currentText").toString();
    WriteInfString("ControlFlags", "RevitVersion", revitVersion);

    QQuickItem* qcheckboxIFC = m_item->findChild<QQuickItem*>("cbIFC");
    bool IsCheckboxIFC_checked = qcheckboxIFC->property("checked").toBool();
    QString sIFC_checked = QVariant(IsCheckboxIFC_checked).toString();
    WriteInfString("RVT", "IFC", sIFC_checked);

    QQuickItem* checkboxNavi = m_item->findChild<QQuickItem*>("cbNavi");
    bool IsCheckboxNavi_checked = checkboxNavi->property("checked").toBool();
    QString sNavi_checked = QVariant(IsCheckboxNavi_checked).toString();
    WriteInfString("RVT", "NWC", sNavi_checked);

    QQuickItem* QQuickText_IFCPath = m_item->findChild<QQuickItem*>("text_IFCPath");
    QString sIFCPath = QQuickText_IFCPath->property("text").toString();
    WriteInfString("DestinationDirs", "DefaultDestDir", sIFCPath);

    QQuickItem* qtextIFCVers = m_item->findChild<QQuickItem*>("row_IFCVersion");
    QObject* cbIFCvers = qtextIFCVers->children()[1];
    QString ifcVersion = cbIFCvers->property("currentText").toString();
    WriteInfString("ControlFlags", "IFCVersion", ifcVersion);

    QQuickItem* lvMain = m_item->findChild<QQuickItem*>("o_lvMain");
    QObject* listModel = lvMain->children()[1];
    QAbstractListModel* qmlListModel = qobject_cast<QAbstractListModel*>(listModel);

    if (qmlListModel != nullptr)
    {
        for (int i = 0; i < qmlListModel->rowCount(); ++i)
        {
            QString rvtFileName = qmlListModel->data(qmlListModel->index(i, 0), 0).toString();
            AddInfString(rvtFileName);
        }
    }
    else
    {
        qDebug() << "Getting *.RVT files list is failed!";
    }
    WriteInfString("ControlFlags", "runNow", "true");
    qDebug() << "The control flag 'runNow' was set to true";

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
    QString appData = getenv("appdata");

    if (appData.isEmpty()) {
        qFatal("Unable to find appData directory!");
    }
    QString logFileLocation = "\\alabuga_dev\\" + fileName;
    QFile outFile(appData + logFileLocation);

    if (txt != "" && !msg.startsWith("QML Debugger: Waiting for connection on port") )
    {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << QTime::currentTime().toString() << " " << txt << Qt::endl;
    }
}

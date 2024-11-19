#include "backend.h"
#include <windows.h>
#include "qquickitem.h"
#include <QQuickView>
#include <qguiapplication.h>
#include <QFileDialog>
#include <QtCore/qabstractitemmodel.h>

BackEnd::BackEnd(QGuiApplication *parent, QObject* item)
{
    m_Window = parent;
    m_item = item;
    m_server = new QTcpServer();
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

void BackEnd::slotRunClicked(const QString &utime)
{
    DeleteInfSection("SourceDisksFiles");
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
            WriteInfString("SourceDisksFiles", rvtFileName, "1");
        }
    }
    else
    {
        qDebug() << "Getting *.RVT files list is failed!";
    }
    WriteInfString("ControlFlags", "runNow", "true");
    qDebug() << "The control flag 'runNow' was set to true";
}

void BackEnd::escSlot()
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

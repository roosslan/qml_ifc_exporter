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
}

void BackEnd::slotStopClicked()
{

}

void BackEnd::DeleteSection(QString sectionName)
{
    LPCWSTR wsSection = (const wchar_t*) sectionName.utf16();
    WritePrivateProfileStringW(wsSection, NULL, NULL, iniFName);
    wsSection = nullptr;
}

void BackEnd::SaveToInf(QString sectionName, QString keyName, QString value)
{
    LPCWSTR wsSection = (const wchar_t*) sectionName.utf16();
    LPCWSTR wsKey = (const wchar_t*) keyName.utf16();
    LPCWSTR wsValue = (const wchar_t*) value.utf16();
    WritePrivateProfileStringW(wsSection, wsKey, wsValue, iniFName);
    wsSection = wsKey = wsValue = nullptr;
}

void BackEnd::slotRunClicked(const QString &utime)
{
    DeleteSection("SourceDisksFiles");
    SaveToInf("ControlFlags", "Time", utime);

    QQuickItem* qcbRvtVers = m_item->findChild<QQuickItem*>("row_RvtVersion");
    QObject* cb_RvtVers = qcbRvtVers->children()[1];
    QString revitVersion = cb_RvtVers->property("currentText").toString();
    SaveToInf("ControlFlags", "RevitVersion", revitVersion);

    QQuickItem* qcheckboxIFC = m_item->findChild<QQuickItem*>("cbIFC");
    bool IsCheckboxIFC_checked = qcheckboxIFC->property("checked").toBool();
    QString sIFC_checked = QVariant(IsCheckboxIFC_checked).toString();
    SaveToInf("RVT", "IFC", sIFC_checked);

    QQuickItem* checkboxNavi = m_item->findChild<QQuickItem*>("cbNavi");
    bool IsCheckboxNavi_checked = checkboxNavi->property("checked").toBool();
    QString sNavi_checked = QVariant(IsCheckboxNavi_checked).toString();
    SaveToInf("RVT", "NWC", sNavi_checked);

    QQuickItem* QQuickText_IFCPath = m_item->findChild<QQuickItem*>("text_IFCPath");
    QString sIFCPath = QQuickText_IFCPath->property("text").toString();
    SaveToInf("DestinationDirs", "DefaultDestDir", sIFCPath);

    QQuickItem* qtextIFCVers = m_item->findChild<QQuickItem*>("row_IFCVersion");
    QObject* cbIFCvers = qtextIFCVers->children()[1];
    QString ifcVersion = cbIFCvers->property("currentText").toString();
    SaveToInf("ControlFlags", "IFCVersion", ifcVersion);

    QQuickItem* lvMain = m_item->findChild<QQuickItem*>("o_lvMain");
    QObject* listModel = lvMain->children()[1];
    QAbstractListModel* qmlListModel = qobject_cast<QAbstractListModel*>(listModel);    

    if (qmlListModel != nullptr)
    {
        for (int i = 0; i < qmlListModel->rowCount(); ++i)
        {
            QString rvtFileName = qmlListModel->data(qmlListModel->index(i, 0), 0).toString();
            SaveToInf("SourceDisksFiles", rvtFileName, "1");
        }
    }
    else
    {
        qDebug() << "Getting *.RVT files list is failed!";
    }
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
        txt = QString("Debug: %1").arg(msg);
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
    const QString fileName{"alabuga.dev.log"};
    QString appData = getenv("appdata");

    if (appData.isEmpty()) {
        qFatal("Unable to find appData directory!");
    }
    QString logFileLocation = "\\alabuga_dev\\" + fileName;
    QFile outFile(appData + logFileLocation);

    if (txt != "" && !txt.startsWith("Debug: QML Debugger: Waiting for connection on port") )
    {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << QTime::currentTime().toString() << " " << txt << Qt::endl;
    }
}

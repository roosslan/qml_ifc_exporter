#include "backend.h"

#include <QQuickView>
#include <qguiapplication.h>
#include <QFileDialog>

BackEnd::BackEnd(QGuiApplication *parent)
{
    qmlWindow = parent;
}

void BackEnd::selectedFileSlot(const QString &fname)
{
    qDebug() << "user selected the file: " << fname;
}

void BackEnd::slotRunClicked(const QString &utime)
{
    qDebug() << "slotbox  selected time " << utime;
}

void BackEnd::escSlot()
{
    qmlWindow->exit(0);
    qDebug() << "ESC DCalled the C++ slot with message:";
}

void BackEnd::cppSlot(const QString &msg)
{
        qDebug() << "Called the C++ slot with message:" << msg;
};

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

    if (txt != "")
    {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        ts << QTime::currentTime().toString() << " " << txt << Qt::endl;
    }
}

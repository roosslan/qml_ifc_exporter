#ifndef BACKEND_H
#define BACKEND_H

#include <qqml.h>
#include <windows.h>
#include <QObject>
#include <QString>
#include <QGuiApplication>
#include <QTcpSocket>
#include <QTcpServer>

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QGuiApplication* m_Window;
    QTcpSocket tcpSocket;
    QObject *m_item;
    QTcpServer* m_server;
    QString appData = getenv("appdata");    
public:    
    BackEnd(QGuiApplication *parent, QObject* item);
    QString iniFile = appData + "\\alabuga_dev\\bimalde.inf";
    LPCWSTR infName = (const wchar_t*) iniFile.utf16();
public slots:
    void escSlot();
    void slotStopClicked();
    void slotRunClicked(const QString &utime);

    QString ReadInfString(QString sectionName, QString keyName);
    void WriteInfString(QString sectionName, QString keyName, QString value);
    void DeleteInfSection(QString sectionName);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

#endif // BACKEND_H

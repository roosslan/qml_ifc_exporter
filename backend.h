#ifndef BACKEND_H
#define BACKEND_H

#include <qqml.h>
#include <windows.h>
#include <QObject>
#include <QString>
#include <QGuiApplication>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMetaType>
#include <QSet>
#include <QStandardPaths>
#include <qquickview.h>
#include "simpleini.h"

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QGuiApplication* m_Window;
    QTcpSocket tcpSocket;
    QObject *m_item;
    HWND m_hwnd;
    std::string m_str_hwnd; /* Для передачи в окно IFCSettings */
    QTcpServer* m_server;

    QString appData = getenv("appdata");


    /* Так как все INI-файлы для WritePrivateProfileStringW всегда ANSI, пишем сами - как UTF8 с русскими символами */
    CSimpleIniW configFile;

    QSet<QTcpSocket*> connection_set;
signals:
    void newMessage(QString);
private slots:
    void newSocketConnection();
    void appendToSocketList(QTcpSocket* socket);
    void readSocket();
    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);
public:    
    BackEnd(QGuiApplication *parent, QObject* item, HWND hWnd);
    ~BackEnd();
    QString infFile = appData + "\\alabuga_dev\\bimalde.inf";

    /* Для DeleteInfSection */
    LPCWSTR infName = (const wchar_t*) infFile.utf16();
public slots:
    void slotEscPressed();
    void slotStopClicked();
    void slotBtnIFCSettingsClicked();
    void slotRunClicked(const QString &utime, const int rightNow);
    void slotIsFileExists(QString fname, QString rvtVersion);
    QString ReadInfString(QString sectionName, QString keyName);
    void WriteInfString(QString sectionName, QString keyName, QString value);
    void DeleteInfSection(QString sectionName);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

#endif // BACKEND_H

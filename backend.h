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

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QGuiApplication* m_Window;
    QTcpSocket tcpSocket;
    QObject *m_item;
    HWND m_hwnd;
    QTcpServer* m_server;
    QString appData = getenv("appdata");
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
    QString infFile = appData + "\\alabuga_dev\\bimalde.inf";
    LPCWSTR infName = (const wchar_t*) infFile.utf16();
public slots:
    void slotEscPressed();
    void slotStopClicked();
    void slotBtnIFCSettingsClicked();
    void slotRunClicked(const QString &utime, const int rightNow);
    void slotIsFileExists(QString fname, QString rvtVersion);
    void AddInfString(QString keyAsValue);
    void AppendInfSection(QString sectionName);
    QString ReadInfString(QString sectionName, QString keyName);
    void WriteInfString(QString sectionName, QString keyName, QString value);
    void DeleteInfSection(QString sectionName);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

#endif // BACKEND_H

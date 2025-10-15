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
#include <QMessageBox>
#include <QStandardPaths>
#include <qquickview.h>
#include <qlibrary.h>
#include "simpleini.h"
#include <QtQuick>

struct toRestore
{
    QString fname;
    QString view;
    QString site;
};

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
    QString viewsFile = appData + "\\alabuga_dev\\rvt_views.txt";
    QString sitesFile = appData + "\\alabuga_dev\\rvt_sites.txt";
    QString viewsAndSitesFile = appData + "\\alabuga_dev\\views_sites.sav";

    /* Для DeleteInfSection */
    LPCWSTR infName = (const wchar_t*) infFile.utf16();
public slots:
    void slotEscPressed();
    void slotStopClicked();
    void slotBtnIFCSettingsClicked();
    void slotRunClicked(const int rightNow, const QString &utime, const QString &udate);
    void slotIsFileExists(QString fname, QString rvtVersion);
    void slotSaveViewAndSiteToFile(QString fName, QString viewName, QString siteName, int appendMode);
    QString ReadInfString(QString sectionName, QString keyName);
    std::list<QString> GetAllKeysOfSection(QString sectionName);
    std::vector<toRestore> GetAllKeysAndValuesOfFile(QString fileName);
    void WriteInfString(QString sectionName, QString keyName, QString value);
    void DeleteInfSection(QString sectionName);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

template<class T> T& unmove(T&& t) { return static_cast<T&>(t); }

class exportQuickView : public QQuickView
{
    Q_OBJECT
public:
    explicit exportQuickView(QWindow *parent = nullptr) : QQuickView(parent) {}

protected:
    void closeEvent(QCloseEvent *event) override
    {
        QMetaObject::invokeMethod(rootObject(), "saveViewsAndSitesToFile");
        event->accept();
    }
};

#endif // BACKEND_H


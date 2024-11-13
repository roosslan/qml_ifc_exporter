#ifndef BACKEND_H
#define BACKEND_H

#include <qqml.h>
#include <windows.h>
#include <QObject>
#include <QString>
#include <QGuiApplication>

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QGuiApplication* m_Window;
    QObject *m_item;
    QString appData = getenv("appdata");
    QString iniFile = appData + "\\alabuga_dev\\export.inf";
    LPCWSTR iniFName = (const wchar_t*) iniFile.utf16();
public:
    BackEnd(QGuiApplication *parent, QObject* item);
public slots:
    void escSlot();
    void slotStopClicked();
    void slotRunClicked(const QString &utime);
    void SaveToInf(QString sectionName, QString keyName, QString value);
    void DeleteSection(QString sectionName);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

#endif // BACKEND_H

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <qqml.h>
#include <QGuiApplication>

class BackEnd : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QGuiApplication* qmlWindow;
public:
    BackEnd(QGuiApplication *parent);
public slots:
    void cppSlot(const QString &msg);
    void escSlot();
    void selectedFileSlot(const QString &fname);
    void slotRunClicked(const QString &utime);
};

void bgMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

#endif // BACKEND_H

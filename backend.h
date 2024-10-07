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
    void selectedFileSlot(const QString &fname);
};

#endif // BACKEND_H

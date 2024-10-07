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

void BackEnd::cppSlot(const QString &msg)
{
        qDebug() << "Called the C++ slot with message:" << msg;
};

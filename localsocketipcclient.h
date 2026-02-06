#ifndef LOCALSOCKETIPCCLIENT_HPP
#define LOCALSOCKETIPCCLIENT_HPP

#include <QObject>
#include <QtNetwork/QLocalSocket>
#include <QLocalSocket>
#include <QLocalServer>
#include <QTextStream>
#include <qdatastream.h>

class LocalSocketIpcClient : public QObject
{
    Q_OBJECT
public:
    LocalSocketIpcClient(QString remote_server_name, QObject *parent = 0);
    ~LocalSocketIpcClient();

signals:
    void signal_server_response(QString);

public slots:
    void send_message_to_server(QString message);

    void socket_connected();
    void socket_disconnected();

    void socket_read_ready();
    void socket_error(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket* m_socket;
    quint16 m_block_size_;
    QString m_message;
    QString m_server_name_;
};

#endif // LOCALSOCKETIPCCLIENT_HPP

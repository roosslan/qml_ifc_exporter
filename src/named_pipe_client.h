#ifndef NAMED_PIPE_CLIENT_H
#define NAMED_PIPE_CLIENT_H

#include <QObject>
#include <QtNetwork/QLocalSocket>
#include <QLocalSocket>
#include <QLocalServer>
#include <QTextStream>
#include <qdatastream.h>

class pipe_client : public QObject
{
    Q_OBJECT
public:
    pipe_client(QString named_pipe_server_name, QObject *parent = 0);
    ~pipe_client();

signals:
    void signal_server_response(QString);

public slots:
    void send_message_to_server(QString message);

    void socket_connected();
    void socket_disconnected();

    void socket_read_ready();
    void socket_error(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket* m_socket_;
    quint16 m_block_size_;
    QString m_message_;
    QString m_server_name_;
};

#endif // NAMED_PIPE_CLIENT_H

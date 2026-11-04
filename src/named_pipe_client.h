#ifndef NAMED_PIPE_CLIENT_H
#define NAMED_PIPE_CLIENT_H

#include <QObject>
#include <QTimer>
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
    void send_failed(const QString& message);

public slots:
    void send_message_to_server(QString message);

    void socket_connected();
    void socket_disconnected();

    void socket_read_ready();
    void socket_error(QLocalSocket::LocalSocketError error);

private:
    QLocalSocket* m_socket_;
    QTimer m_send_timeout_;
    qint64 m_bytes_remaining_ = 0;
    bool m_sending_ = false;
    QString m_message_;
    QString m_server_name_;
};

#endif // NAMED_PIPE_CLIENT_H

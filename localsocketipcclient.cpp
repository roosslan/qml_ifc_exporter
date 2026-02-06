#include "localsocketipcclient.h"
#include <QDebug>

LocalSocketIpcClient::LocalSocketIpcClient(QString remoteServername, QObject *parent) :
    QObject(parent) {

    m_socket = new QLocalSocket(this);
    m_server_name_ = remoteServername;

    connect(m_socket, SIGNAL(connected()), this, SLOT(socket_connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(socket_read_ready()));
    connect(m_socket, SIGNAL(error(QLocalSocket::LocalSocketError() )), this, SLOT(socket_error(QLocalSocket::LocalSocketError() )));

}

LocalSocketIpcClient::~LocalSocketIpcClient() {
    m_socket->abort();
    delete m_socket;
    m_socket = NULL;
}

void LocalSocketIpcClient::send_message_to_server(QString message) {
    m_socket->abort();
    m_message = message;
    m_socket->connectToServer(m_server_name_);
}

void LocalSocketIpcClient::socket_connected(){

    QByteArray block;

    QDataStream ss_out(&block, QIODevice::WriteOnly);
    ss_out.setVersion(QDataStream::Qt_4_0);
    ss_out << m_message.toUtf8().data();
    ss_out.device()->seek(0);

    /* QDataStream adds 4 extra bytes (size header) to the front of block
       Our scenario is connecting to MSVC Pipe, so we remove them */
    block.remove(0, 4);

    m_socket->write(block);
    m_socket->flush();
}

void LocalSocketIpcClient::socket_disconnected() { }


void LocalSocketIpcClient::socket_read_ready() {
    QDataStream ss_in(this->m_socket);
    ss_in.setVersion(QDataStream::Qt_4_0);
    if (this->m_socket->bytesAvailable() < (int)sizeof(quint16)) {
        return;
    }
    QString qmessage;
    ss_in >> qmessage;
    emit signal_server_response(qmessage);
}

void LocalSocketIpcClient::socket_error(QLocalSocket::LocalSocketError error) {
    qDebug() << "socket_error : " << error;
}

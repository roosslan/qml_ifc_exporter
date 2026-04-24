#include "named_pipe_client.h"
#include <QDebug>

pipe_client::pipe_client(QString remoteServername, QObject *parent) :
    QObject(parent) {

    m_socket_ = new QLocalSocket(this);
    m_server_name_ = remoteServername;

    connect(m_socket_, SIGNAL(connected()), this, SLOT(socket_connected()));
    connect(m_socket_, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));

    connect(m_socket_, SIGNAL(readyRead()), this, SLOT(socket_read_ready()));
    connect(m_socket_, SIGNAL(error(QLocalSocket::LocalSocketError() )), this, SLOT(socket_error(QLocalSocket::LocalSocketError() )));

}

pipe_client::~pipe_client() {
    m_socket_->abort();
    delete m_socket_;
    m_socket_ = NULL;
}

void pipe_client::send_message_to_server(QString message) {
    m_socket_->abort();
    m_message_ = message;
    m_socket_->connectToServer(m_server_name_);
}

void pipe_client::socket_connected(){

    QByteArray block;

    QDataStream ss_out(&block, QIODevice::WriteOnly);
    ss_out.setVersion(QDataStream::Qt_4_0);
    ss_out << m_message_.toUtf8().data();
    ss_out.device()->seek(0);

    /* QDataStream adds 4 extra bytes (size header) to the front of block
       Our scenario is connecting to MSVC Pipe, so we remove them */
    block.remove(0, 4);

    m_socket_->write(block);
    m_socket_->flush();
}

void pipe_client::socket_disconnected() { }


void pipe_client::socket_read_ready() {
    QDataStream ss_in(this->m_socket_);
    ss_in.setVersion(QDataStream::Qt_4_0);
    if (this->m_socket_->bytesAvailable() < (int)sizeof(quint16)) {
        return;
    }
    QString qmessage;
    ss_in >> qmessage;
    emit signal_server_response(qmessage);
}

void pipe_client::socket_error(QLocalSocket::LocalSocketError error) {
    qDebug() << "socket_error : " << error;
}

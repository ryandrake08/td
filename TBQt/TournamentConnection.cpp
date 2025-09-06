#include "TournamentConnection.hpp"

#include "TBRuntimeError.hpp"

#include "TournamentService.hpp"

#include <QByteArray>
#include <QDebug>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalSocket>
#include <QTcpSocket>

struct TournamentConnection::impl
{
    // The abstract IO device (either a tcp or unix socket)
    std::unique_ptr<QIODevice> device;

    // buffer for reads
    QByteArray buffer;
};

// construct from TournamentService
TournamentConnection::TournamentConnection(QObject* parent) : QObject(parent), pimpl(new impl)
{
}

TournamentConnection::~TournamentConnection() = default;

// connect to TournamentService
void TournamentConnection::connect(const TournamentService& tournament)
{
    if(tournament.is_remote())
    {
        // create socket
        auto* socket(new QTcpSocket(this->parent()));
        this->pimpl->device.reset(socket);

        // hook up socket signals
        QObject::connect(socket, &QTcpSocket::connected, this, &TournamentConnection::on_connected);
        QObject::connect(socket, &QTcpSocket::disconnected, this, &TournamentConnection::on_disconnected);
        QObject::connect(socket, &QTcpSocket::readyRead, this, &TournamentConnection::on_readyRead);
        QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, &TournamentConnection::on_error);

        // connect
        socket->connectToHost(QString::fromStdString(tournament.address()), static_cast<quint16>(tournament.port()));
    }
    else
    {
        // create socket
        auto* socket(new QLocalSocket(this->parent()));
        this->pimpl->device.reset(socket);

        // hook up socket signals
        QObject::connect(socket, &QLocalSocket::connected, this, &TournamentConnection::on_connected);
        QObject::connect(socket, &QLocalSocket::disconnected, this, &TournamentConnection::on_disconnected);
        QObject::connect(socket, &QLocalSocket::readyRead, this, &TournamentConnection::on_readyRead);
        QObject::connect(socket, QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::errorOccurred), this, &TournamentConnection::on_error);

        // connect
        socket->connectToServer(QString::fromStdString(tournament.path()));
    }
}

void TournamentConnection::disconnect()
{
    if(this->pimpl->device)
    {
        // close connection
        this->pimpl->device->close();
        this->pimpl->device.reset();
    }
}

// send a command
void TournamentConnection::send_command(const QString& cmd, const QVariantMap& arg)
{
    qDebug() << "sending command:" << cmd;

    // check if device is valid and connected
    if(!this->pimpl->device || !this->pimpl->device->isOpen())
    {
        Q_EMIT this->errorOccurred(QObject::tr("Not connected to tournament server"));
        return;
    }

    // serialize to json
    auto json_obj(QJsonObject::fromVariantMap(arg));

    // convert to json document
    QJsonDocument json_doc(json_obj);

    // convert to data
    auto json_data(json_doc.toJson(QJsonDocument::Compact));

    // encode command to utf8, and append rest of command
    auto cmd_data(cmd.toUtf8());
    cmd_data.append(' ');
    cmd_data.append(json_data);
    cmd_data.append('\n');

    // write to socket
    auto bytes_written = this->pimpl->device->write(cmd_data);
    if(bytes_written == -1)
    {
        Q_EMIT this->errorOccurred(QObject::tr("Failed to send command: %1").arg(cmd));
    }
}

// slots

void TournamentConnection::on_connected()
{
    // pass signal through
    Q_EMIT this->connected();
}

void TournamentConnection::on_disconnected()
{
    // pass signal through
    Q_EMIT this->disconnected();
}

void TournamentConnection::on_readyRead()
{
    // read and append to input buffer
    this->pimpl->buffer.append(this->pimpl->device->readAll());

    // iterate line by line
    auto end(this->pimpl->buffer.indexOf('\n'));
    while(end != -1)
    {
        qDebug() << "parsing a line" << end << "bytes long";

        auto json_data(this->pimpl->buffer.left(end + 1));

        // convert to json document
        auto json_doc(QJsonDocument::fromJson(json_data));

        // ensure root of document is an object
        if(!json_doc.isObject())
        {
            // emit error instead of throwing
            Q_EMIT this->errorOccurred(QObject::tr("Invalid response from server"));
            return;
        }

        // convert to json object
        auto json_obj(json_doc.object());

        // convert to variant map
        auto response(json_obj.toVariantMap());

        // call signal
        Q_EMIT this->receivedData(response);

        // remove parsed bytes
        this->pimpl->buffer.remove(0, end + 1);

        // look for next newline
        end = this->pimpl->buffer.indexOf('\n');
    }
}

void TournamentConnection::on_error()
{
    QString errorString;

    // get error string from the socket
    if(auto* tcpSocket = qobject_cast<QTcpSocket*>(this->pimpl->device.get()))
    {
        errorString = tcpSocket->errorString();
    }
    else if(auto* localSocket = qobject_cast<QLocalSocket*>(this->pimpl->device.get()))
    {
        errorString = localSocket->errorString();
    }
    else
    {
        errorString = QObject::tr("Unknown socket error");
    }

    qDebug() << "Connection error:" << errorString;
    Q_EMIT this->errorOccurred(errorString);
}

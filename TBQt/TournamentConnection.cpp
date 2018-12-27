#include "TournamentConnection.hpp"
#include "TournamentService.hpp"
#include "TBRuntimeError.hpp"

#include <QDebug>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>

// construct from TournamentService
TournamentConnection::TournamentConnection(QObject* parent) : QObject(parent)
{
}

// connect to TournamentService
void TournamentConnection::connect(const TournamentService& tournament)
{
    if(tournament.is_remote())
    {
        // create socket
        QTcpSocket* socket(new QTcpSocket(this->parent()));
        this->device.reset(socket);

        // hook up socket signals
        QObject::connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

        // connect
        socket->connectToHost(QString::fromStdString(tournament.address()), static_cast<quint16>(tournament.port()));
    }
    else
    {
        // create socket
        QLocalSocket* socket(new QLocalSocket(this->parent()));
        this->device.reset(socket);

        // hook up socket signals
        QObject::connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

        // connect
        socket->connectToServer(QString::fromStdString(tournament.path()));
    }
}

void TournamentConnection::disconnect()
{
    if(this->device)
    {
        // close connection
        this->device->close();
        this->device.reset();
    }
}

// send a command
void TournamentConnection::send_command(const QString& cmd, const QVariantMap& arg)
{
    qDebug() << "sending command: " << cmd;

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
    this->device->write(cmd_data);
}

// slots

void TournamentConnection::connected()
{
    this->tournament_connected();
}

void TournamentConnection::disconnected()
{
    this->tournament_disconnected();
}

void TournamentConnection::readyRead()
{
    // read the data from the socket line by line
    auto json_data(this->device->readLine());

    // if we read anything, convert it to a QVariantMap
    while(!json_data.isEmpty())
    {
        qDebug() << json_data.size() << "bytes read from tournament";

        // convert to json document
        auto json_doc(QJsonDocument::fromJson(json_data));

        // ensure root of document is an object
        if(!json_doc.isObject())
        {
            // handle invalid json
            throw TBRuntimeError(QObject::tr("Invalid response from server"));
        }

        // convert to json object
        auto json_obj(json_doc.object());

        // convert to variant map
        auto response(json_obj.toVariantMap());

        // call signal
        this->received_data(response);

        // read next line, if any
        json_data = this->device->readLine();
    }
}

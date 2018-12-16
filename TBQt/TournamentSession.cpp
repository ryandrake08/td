#include "TournamentSession.hpp"
#include "TournamentService.hpp"
#include "TBRuntimeError.hpp"

#include <QDebug>
#include <QLocalSocket>
#include <QHash>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSharedPointer>
#include <QString>
#include <QTcpSocket>
#include <QVariant>

#include <random>

struct TournamentSession::impl
{
    // pointer to the current socket object
    QSharedPointer<QIODevice> socket;

    // mapping between unique command and function to handle the command's response
    QHash<int, std::function<void(const QVariantHash&)>> functions_for_commands;

    // current tournament state
    QVariantHash state;

    // true if connected
    bool connected;

    // true if authorized
    bool authorized;

    explicit impl()
    {
    }
};

TournamentSession::TournamentSession(QObject* parent) : QObject(parent), pimpl(new impl())
{
}

TournamentSession::~TournamentSession()
{
    // disconnect before further cleaning up pimpl
    this->disconnect();
}

// client identifier (used for authenticating with servers)
int TournamentSession::client_identifier()
{
    const auto key(QStringLiteral("clientIdentifier"));

    // if we don't already have one
    QSettings settings;
    if(!settings.contains(key))
    {
        // generate a new identifier
        std::random_device rd;
        std::default_random_engine gen(rd());
        std::uniform_int_distribution<int> dist(10000, 99999);
        auto cid(dist(gen));

        // store it (client id persists in settings)
        settings.setValue(key, cid);
    }

    return settings.value(key).toInt();
}

// connect to a tournament service
bool TournamentSession::connect(const TournamentService& tournament)
{
    // first disconnect
    this->disconnect();

    if(tournament.is_remote())
    {
        // create socket
        QTcpSocket* socket(new QTcpSocket(this));
        this->pimpl->socket.reset(socket);

        // hook up socket signals
        QObject::connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        QObject::connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
        QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

        // connect
        socket->connectToHost(QString::fromStdString(tournament.address()), tournament.port());
    }
    else
    {
        // create socket
        QLocalSocket* socket(new QLocalSocket(this));
        this->pimpl->socket.reset(socket);

        // hook up socket signals
        QObject::connect(socket, SIGNAL(connected()), this, SLOT(connected()));
        QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
        QObject::connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
        QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

        // connect
        socket->connectToServer(QString::fromStdString(tournament.path()));
    }

    return !this->pimpl->socket.isNull();
}

void TournamentSession::disconnect()
{
    // reset the object, which is expected to disconnect
    this->pimpl->socket.reset();
}

// slots
void TournamentSession::connected()
{
    if(this->pimpl)
    {
        // clear state
        this->pimpl->state.clear();

        // set connected state
        this->pimpl->connected = true;

        // always check if we're authorized right away
        this->check_authorized([this](bool authorized)
        {
            // set internal state if changed
            if(this->pimpl->authorized != authorized)
            {
                this->pimpl->authorized = authorized;
            }

            if(authorized)
            {
                qDebug() << "connected and authorized";
            }
            else
            {
                qDebug() << "connected but not authorized";
            }
        });

        // and request initial ztate
        this->get_state([this](const QVariantHash& result)
        {
            this->pimpl->state = result;
        });
    }
    else
    {
        qDebug() << "BUG: connected() called while pimpl is deleted!";
    }
}

void TournamentSession::disconnected()
{
    qDebug() << "disconnected";

    if(this->pimpl)
    {
        // clear state
        this->pimpl->state.clear();

        // set other state
        this->pimpl->connected = false;
        this->pimpl->authorized = false;
    }
    else
    {
        qDebug() << "BUG: disconnected() called while pimpl is deleted!";
    }
}

void TournamentSession::bytesWritten(qint64 bytes)
{
    qDebug() << bytes << "bytes written...";
}

void TournamentSession::readyRead()
{
    if(this->pimpl)
    {
        // read the data from the socket line by line
        auto json_data(this->pimpl->socket->readLine());

        while(!json_data.isEmpty())
        {
            qDebug() << json_data.size() << "bytes read from tournament";

            // convert to json document
            auto json_doc(QJsonDocument::fromJson(json_data));

            // ensure non-null document and root of document is an object
            if(json_doc.isNull() || !json_doc.isObject())
            {
                // handle invalid json
                throw TBRuntimeError(QObject::tr("Invalid response from server"));
            }

            // convert to json object
            auto json_obj(json_doc.object());

            // convert to variant map
            auto response(json_obj.toVariantHash());

            // look for command key
            auto command_key(response.take("echo"));
            if(command_key.isNull())
            {
                // no command key, treat data as state
                // for now, just replace entire QVariantMap. later will want to only update what is changed and delete keys no longer relevant
                this->pimpl->state = response;
            }
            else
            {
                // look up function
                auto function(this->pimpl->functions_for_commands.take(command_key.toInt()));

                // call function
                if(function)
                {
                    function(response);
                }
            }

            json_data = this->pimpl->socket->readLine();
        }
    }
    else
    {
        qDebug() << "BUG: readyRead() called while pimpl is deleted!";
    }
}

// send command
void TournamentSession::send_command(const QString& cmd, const QVariantHash& arg=QVariantHash(), const std::function<void(const QVariantHash&)>& handler=std::function<void(const QVariantHash&)>())
{
    QVariantHash argument(arg);

    // append to every command: authentication
    auto cid(TournamentSession::client_identifier());
    argument["authenticate"] = cid;

    // append to every command: command key
    static int incrementing_key(0);
    auto command_key(incrementing_key++);
    argument["echo"] = command_key;

    // map function to command key for later lookup
    this->pimpl->functions_for_commands[command_key] = handler;

    // serialize to json
    auto json_obj(QJsonObject::fromVariantHash(argument));

    // convert to json document
    QJsonDocument json_doc(json_obj);

    // convert to data
    auto json_data(json_doc.toJson(QJsonDocument::Compact));

    // encode command to utf8, and append rest of command
    auto cmd_data(cmd.toUtf8());
    cmd_data.append(' ');
    cmd_data.append(json_data);
    cmd_data.append('\n');

    qDebug() << "sending command: " << cmd_data.constData();

    // write to socket
    this->pimpl->socket->write(cmd_data);
}

// tournament commands
void TournamentSession::check_authorized(const std::function<void(bool)>& handler)
{
    this->send_command("check_authorized", QVariantHash(), [handler](const QVariantHash& result) { handler(result["authorized"].toBool()); });
}

void TournamentSession::get_state(const std::function<void(const QVariantHash&)>& handler)
{
    this->send_command("get_state", QVariantHash(), [handler](const QVariantHash& result) { handler(result); });
}

void TournamentSession::get_config(const std::function<void(const QVariantHash&)>& handler)
{
    this->send_command("get_config", QVariantHash(), [handler](const QVariantHash& result) { handler(result); });
}

void TournamentSession::configure(const QVariantHash& config, const std::function<void(const QVariantHash&)>& handler)
{
    this->send_command("configure", config, [handler](const QVariantHash& result) { handler(result); });
}

void TournamentSession::reset_state()
{
    this->send_command("reset_state");
}

void TournamentSession::start_game_at(const QDateTime& datetime)
{
    this->send_command("start_game", QVariantHash{{"start_at", datetime}});
}

void TournamentSession::start_game()
{
    this->send_command("start_game");
}

void TournamentSession::stop_game()
{
    this->send_command("stop_game");
}

void TournamentSession::resume_game()
{
    this->send_command("resume_game");
}

void TournamentSession::pause_game()
{
    this->send_command("pause_game");
}

void TournamentSession::toggle_pause_game()
{
    this->send_command("toggle_pause_game");
}

void TournamentSession::set_previous_level(const std::function<void(int)>& handler)
{
    this->send_command("set_previous_level", QVariantHash(), [handler](const QVariantHash& result) { handler(result["blind_level_changed"].toInt()); });
}

void TournamentSession::set_next_level(const std::function<void(int)>& handler)
{
    this->send_command("set_next_level", QVariantHash(), [handler](const QVariantHash& result) { handler(result["blind_level_changed"].toInt()); });
}

void TournamentSession::set_action_clock(int milliseconds)
{
    this->send_command("set_action_clock", QVariantHash{{"duration", milliseconds}});
}

void TournamentSession::clear_action_clock()
{
    this->send_command("clear_action_clock");
}

void TournamentSession::gen_blind_levels(const QVariantHash& request, std::function<void(const QVariantList&)>& handler)
{
    this->send_command("gen_blind_levels", request, [handler](const QVariantHash& result) { handler(result["blind_levels"].toList()); });
}

void TournamentSession::fund_player(const QString& player_id, int source)
{
    this->send_command("fund_player", QVariantHash{{"player_id", player_id}, {"source_id", source}});
}

void TournamentSession::plan_seating_for(int expected_players, std::function<void(const QVariantList&)>& handler)
{
    this->send_command("plan_seating_for", QVariantHash{{"max_expected_players", expected_players}}, [handler](const QVariantHash& result) { handler(result["players_moved"].toList()); });
}

void TournamentSession::seat_player(const QString& player_id, std::function<void(const QString&, const QString&, const QString&, bool)>& handler)
{
    this->send_command("seat_player", QVariantHash{{"player_id", player_id}}, [handler](const QVariantHash& result)
    {
        if(result.contains("player_seated"))
        {
            auto player_seated(result["player_seated"].toHash());
            handler(player_seated["player_id"].toString(), player_seated["table_name"].toString(), player_seated["seat_name"].toString(), false);
        }
        else if(result.contains("already_seated"))
        {
            auto already_seated(result["already_seated"].toHash());
            handler(already_seated["player_id"].toString(), already_seated["table_name"].toString(), already_seated["seat_name"].toString(), true);
        }
    });
}

void TournamentSession::unseat_player(const QString& player_id)
{
    this->send_command("unseat_player", QVariantHash{{"player_id", player_id}});
}

void TournamentSession::bust_player(const QString& player_id, std::function<void(const QVariantList&)>& handler)
{
    this->send_command("bust_player", QVariantHash{{"player_id", player_id}}, [handler](const QVariantHash& result) { handler(result["players_moved"].toList()); });
}

void TournamentSession::rebalance_seating(std::function<void(const QVariantList&)>& handler)
{
    this->send_command("rebalance_seating", QVariantHash(), [handler](const QVariantHash& result) { handler(result["players_moved"].toList()); });
}

void TournamentSession::quick_setup(std::function<void(const QVariantList&)>& handler)
{
    this->send_command("quick_setup", QVariantHash(), [handler](const QVariantHash& result) { handler(result["seated_players"].toList()); });
}

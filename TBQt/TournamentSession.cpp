#include "TournamentSession.hpp"

#include "TBRuntimeError.hpp"

#include "TournamentConnection.hpp"
#include "TournamentService.hpp"

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QSettings>
#include <QTextCodec>

#include <random>
#include <set>

struct TournamentSession::impl
{
    // pointer to the current socket object
    TournamentConnection connection;

    // mapping between unique command and function to handle the command's response
    QHash<int, std::function<void(const QVariantMap&)>> functions_for_commands;

    // current tournament state
    QVariantMap state;

    // true if connected
    bool connected;

    // true if authorized
    bool authorized;

    // last connection error
    QString last_error;
};

TournamentSession::TournamentSession(QObject* parent) : QObject(parent), pimpl(new impl())
{
    // initialize state
    pimpl->connected = false;
    pimpl->authorized = false;

    // hook up TournamentConnection signals
    QObject::connect(&this->pimpl->connection, &TournamentConnection::connected, this, &TournamentSession::on_connected);
    QObject::connect(&this->pimpl->connection, &TournamentConnection::disconnected, this, &TournamentSession::on_disconnected);
    QObject::connect(&this->pimpl->connection, &TournamentConnection::receivedData, this, &TournamentSession::on_receivedData);
    QObject::connect(&this->pimpl->connection, &TournamentConnection::errorOccurred, this, &TournamentSession::on_connectionError);
}

TournamentSession::~TournamentSession() = default;

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
void TournamentSession::connect(const TournamentService& tournament)
{
    // forward to connection
    qDebug() << "connecting to tournament service:" << QString::fromStdString(tournament.name());
    this->pimpl->connection.connect(tournament);
}

void TournamentSession::disconnect()
{
    // reset the object, which is expected to disconnect
    qDebug() << "disconnecting from tournament service";
    this->pimpl->connection.disconnect();
}

// update state
void TournamentSession::update(const QVariantMap& new_state)
{
    // replace old state
    auto old_state = this->pimpl->state;
    this->pimpl->state = new_state;

    // special handling for the clocks because we may need to add an offset to what the daemon provides
    if (this->pimpl->state.contains("clock_remaining") && this->pimpl->state.contains("current_time"))
    {
        // format time for display
        qint64 clockRemaining = this->pimpl->state.value("clock_remaining").toLongLong();
        qint64 currentTime = this->pimpl->state.value("current_time").toLongLong();

        QString clockText = formatClockTime(clockRemaining, currentTime, true);
        this->pimpl->state["clock_text"] = clockText;
    }

    if (this->pimpl->state.contains("elapsed_time") && this->pimpl->state.contains("current_time"))
    {
        qint64 elapsedTime = this->pimpl->state.value("elapsed_time").toLongLong();
        qint64 currentTime = this->pimpl->state.value("current_time").toLongLong();

        QString elapsedText = formatClockTime(elapsedTime, currentTime, false);
        this->pimpl->state["elapsed_time_text"] = elapsedText;
    }

    if (!this->pimpl->state.value("running", true).toBool())
    {
        this->pimpl->state["clock_text"] = QObject::tr("PAUSED");
    }

    // emit a stateChanged each map item difference
    auto first1(old_state.constBegin());
    auto last1(old_state.constEnd());
    auto first2(new_state.constBegin());
    auto last2(new_state.constEnd());

    while(first1 != last1 || first2 != last2)
    {
        if(first1 == last1 && first2 != last2)
        {
            // no more keys in state1, emit for all remaining keys in state2
            Q_EMIT this->stateChanged(first2.key(), first2.value());
            qDebug() << "New state:" << first2.key();
            ++first2;
        }
        else if(first2 == last2 && first1 != last1)
        {
            // no more keys in state2, emit for all remaining keys in state1
            Q_EMIT this->stateChanged(first1.key(), first1.value());
            qDebug() << "Removed state:" << first1.key();
            ++first1;
        }
        else if(first1.key() < first2.key())
        {
            // emit for deleted keys
            Q_EMIT this->stateChanged(first1.key(), first1.value());
            qDebug() << "Removed state:" << first1.key();
            ++first1;
        }
        else if(first2.key() < first1.key())
        {
            // emit for new keys
            Q_EMIT this->stateChanged(first2.key(), first2.value());
            qDebug() << "New state:" << first2.key();
            ++first2;
        }
        else
        {
            if(first1.value() != first2.value())
            {
                Q_EMIT this->stateChanged(first2.key(), first2.value());
                qDebug() << "State changed:" << first1.key();
            }
            ++first1; ++first2;
        }
    }
}

// format clock time for display
QString TournamentSession::formatClockTime(qint64 timeValue, qint64 currentTime, bool countingDown)
{
    // Calculate time offset based on current system time vs daemon time
    qint64 systemTime = QDateTime::currentMSecsSinceEpoch();
    qint64 timeOffset = systemTime - currentTime;

    qint64 adjustedTime;
    if (countingDown)
    {
        // For countdown clocks, subtract the offset
        adjustedTime = timeValue - timeOffset;
        if (adjustedTime < 0)
        {
            adjustedTime = 0;
        }
    }
    else
    {
        // For elapsed time, add the offset
        adjustedTime = timeValue + timeOffset;
        if (adjustedTime < 0)
        {
            adjustedTime = 0;
        }
    }

    // Convert to seconds
    int totalSeconds = adjustedTime / 1000;

    if (countingDown)
    {
        // Format as MM:SS for countdown
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    }
    else
    {
        // Format as HH:MM:SS for elapsed time
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        return QString("%1:%2:%3")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'));
    }
}

// slots
void TournamentSession::on_connected()
{
    // clear state
    this->update(QVariantMap());
    qDebug() << "tournament state cleared";

    // set connected state
    if(this->pimpl->connected == false)
    {
        this->pimpl->connected = true;
        qDebug() << "tournament connected";
        Q_EMIT this->connectedChanged(this->pimpl->connected);
    }

    // always check if we're authorized right away
    this->check_authorized_with_handler([this](bool authorized)
    {
        // set internal state if changed
        if(this->pimpl->authorized != authorized)
        {
            this->pimpl->authorized = authorized;
            qDebug() << (authorized ? "user authorized" : "user not authorized");
            Q_EMIT this->authorizedChanged(this->pimpl->authorized);
        }
    });

    // and request initial state
    this->get_state_with_handler([this](const QVariantMap& result)
    {
        this->update(result);
        qDebug() << "got initial state";
    });
}

void TournamentSession::on_disconnected()
{
    if(this->pimpl)
    {
        // clear state
        this->update(QVariantMap());
        qDebug() << "tournament state cleared";

        // clear authorized status
        if(this->pimpl->authorized)
        {
            this->pimpl->authorized = false;
            qDebug() << "user deauthoriized";
            Q_EMIT this->authorizedChanged(this->pimpl->authorized);
        }

        // clear connection status
        if(this->pimpl->connected)
        {
            this->pimpl->connected = false;
            qDebug() << "tournament disconnected";
            Q_EMIT this->connectedChanged(this->pimpl->connected);
        }
    }
    else
    {
        qDebug() << "BUG: disconnected() called while pimpl is deleted!";
    }
}

void TournamentSession::on_receivedData(const QVariantMap& data)
{
    QVariantMap response(data);

    // look for command key
    auto command_key(response.take("echo"));
    if(command_key.isNull())
    {
        // no command key, treat data as state

        // update state
        this->update(response);
        qDebug() << "tournament state updated";
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
}

void TournamentSession::on_connectionError(const QString& error)
{
    // store last error
    this->pimpl->last_error = error;

    // emit error signals
    qDebug() << "Connection error:" << error;
    Q_EMIT this->connectionError(error);
    Q_EMIT this->networkError(error);
}

// send command
void TournamentSession::send_command(const QString& cmd, const QVariantMap& arg=QVariantMap(), const std::function<void(const QVariantMap&)>& handler=std::function<void(const QVariantMap&)>())
{
    // validate connection state
    if(!this->pimpl->connected)
    {
        QString error = QObject::tr("Cannot send command '%1': not connected to tournament server").arg(cmd);
        qDebug() << error;
        Q_EMIT this->commandError(cmd, error);
        return;
    }

    QVariantMap argument(arg);

    // append to every command: authentication
    auto cid(TournamentSession::client_identifier());
    argument["authenticate"] = cid;

    // append to every command: command key
    static int incrementing_key(0);
    auto command_key(incrementing_key++);
    argument["echo"] = command_key;

    // map function to command key for later lookup
    this->pimpl->functions_for_commands[command_key] = handler;

    // send command using connection
    this->pimpl->connection.send_command(cmd, argument);
}

// tournament commands
void TournamentSession::check_authorized()
{
    check_authorized_with_handler({});
}

void TournamentSession::check_authorized_with_handler(const std::function<void(bool)>& handler)
{
    this->send_command("check_authorized", QVariantMap(), [handler](const QVariantMap& result) { if(handler) handler(result["authorized"].toBool()); });
}

void TournamentSession::get_state()
{
    get_state_with_handler({});
}

void TournamentSession::get_state_with_handler(const std::function<void(const QVariantMap&)>& handler)
{
    this->send_command("get_state", QVariantMap(), [handler](const QVariantMap& result) { if(handler) handler(result); });
}

void TournamentSession::get_config()
{
    get_config_with_handler({});
}

void TournamentSession::get_config_with_handler(const std::function<void(const QVariantMap&)>& handler)
{
    this->send_command("get_config", QVariantMap(), [handler](const QVariantMap& result) { if(handler) handler(result); });
}

void TournamentSession::configure(const QVariantMap& config)
{
    configure_with_handler(config, {});
}

void TournamentSession::configure_with_handler(const QVariantMap& config, const std::function<void(const QVariantMap&)>& handler)
{
    this->send_command("configure", config, [handler](const QVariantMap& result) { if(handler) handler(result); });
}

void TournamentSession::reset_state()
{
    this->send_command("reset_state");
}

void TournamentSession::start_game_at(const QDateTime& datetime)
{
    this->send_command("start_game", QVariantMap{{"start_at", datetime}});
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

void TournamentSession::set_previous_level()
{
    set_previous_level_with_handler({});
}

void TournamentSession::set_previous_level_with_handler(const std::function<void(int)>& handler)
{
    this->send_command("set_previous_level", QVariantMap(), [handler](const QVariantMap& result) { if(handler) handler(result["blind_level_changed"].toInt()); });
}

void TournamentSession::set_next_level()
{
    set_next_level_with_handler({});
}

void TournamentSession::set_next_level_with_handler(const std::function<void(int)>& handler)
{
    this->send_command("set_next_level", QVariantMap(), [handler](const QVariantMap& result) { if(handler) handler(result["blind_level_changed"].toInt()); });
}

void TournamentSession::set_action_clock(int milliseconds)
{
    this->send_command("set_action_clock", QVariantMap{{"duration", milliseconds}});
}

void TournamentSession::clear_action_clock()
{
    this->send_command("set_action_clock");
}

void TournamentSession::gen_blind_levels(const QVariantMap& request)
{
    gen_blind_levels_with_handler(request, {});
}

void TournamentSession::gen_blind_levels_with_handler(const QVariantMap& request, const std::function<void(const QVariantList&)>& handler)
{
    this->send_command("gen_blind_levels", request, [handler](const QVariantMap& result) { if(handler) handler(result["blind_levels"].toList()); });
}

void TournamentSession::fund_player(const QString& player_id, int source)
{
    this->send_command("fund_player", QVariantMap{{"player_id", player_id}, {"source_id", source}});
}

void TournamentSession::plan_seating(int expected_players)
{
    plan_seating_with_handler(expected_players, {});
}

void TournamentSession::plan_seating_with_handler(int expected_players, const std::function<void(const QVariantList&)>& handler)
{
    this->send_command("plan_seating", QVariantMap{{"max_expected_players", expected_players}},
                       [this, handler](const QVariantMap& result) {
                           QVariantList movements = result["players_moved"].toList();
                           if (!movements.isEmpty())
                           {
                               playerMovementsUpdated(movements);
                           }
                           if (handler)
                           {
                               handler(movements);
                           }
                       });
}

void TournamentSession::seat_player(const QString& player_id)
{
    seat_player_with_handler(player_id, {});
}

void TournamentSession::seat_player_with_handler(const QString& player_id, const std::function<void(const QString&, const QString&, const QString&, bool)>& handler)
{
    this->send_command("seat_player", QVariantMap{{"player_id", player_id}}, [handler](const QVariantMap& result)
    {
        if(result.contains("player_seated"))
        {
            auto player_seated(result["player_seated"].toMap());
            if(handler) handler(player_seated["player_id"].toString(), player_seated["table_name"].toString(), player_seated["seat_name"].toString(), false);
        }
        else if(result.contains("already_seated"))
        {
            auto already_seated(result["already_seated"].toMap());
            if(handler) handler(already_seated["player_id"].toString(), already_seated["table_name"].toString(), already_seated["seat_name"].toString(), true);
        }
    });
}

void TournamentSession::unseat_player(const QString& player_id)
{
    this->send_command("unseat_player", QVariantMap{{"player_id", player_id}});
}

void TournamentSession::bust_player(const QString& player_id)
{
    bust_player_with_handler(player_id, {});
}

void TournamentSession::bust_player_with_handler(const QString& player_id, const std::function<void(const QVariantList&)>& handler)
{
    this->send_command("bust_player", QVariantMap{{"player_id", player_id}},
                       [this, handler](const QVariantMap& result) {
                           QVariantList movements = result["players_moved"].toList();
                           if (!movements.isEmpty())
                           {
                               playerMovementsUpdated(movements);
                           }
                           if (handler)
                           {
                               handler(movements);
                           }
                       });
}

void TournamentSession::rebalance_seating()
{
    rebalance_seating_with_handler({});
}

void TournamentSession::rebalance_seating_with_handler(const std::function<void(const QVariantList&)>& handler)
{
    this->send_command("rebalance_seating", QVariantMap(),
                       [this, handler](const QVariantMap& result) {
                           QVariantList movements = result["players_moved"].toList();
                           if (!movements.isEmpty())
                           {
                               playerMovementsUpdated(movements);
                           }
                           if (handler)
                           {
                               handler(movements);
                           }
                       });
}

void TournamentSession::quick_setup()
{
    quick_setup_with_handler({});
}

void TournamentSession::quick_setup_with_handler(const std::function<void(const QVariantList&)>& handler)
{
    this->send_command("quick_setup", QVariantMap(), [handler](const QVariantMap& result) { if(handler) handler(result["seated_players"].toList()); });
}

// serialization
QByteArray TournamentSession::results_as_csv() const
{
    // header contains column names
    QString csv(QObject::tr("Player,Finish,Win"));

    // 1 result per line
    for(const auto& obj : this->pimpl->state["results"].toList())
    {
        auto result(obj.toMap());
        csv.append(QString("\n\"%1\",%2,%3").arg(result["name"].toString(), result["place"].toString(), result["payout"].toString()));
    }

    // serialize results using WINDOWS-1252
    auto codec(QTextCodec::codecForName("Windows-1252"));
    return codec->fromUnicode(csv);
}

// accessors
const QVariantMap& TournamentSession::state() const
{
    return this->pimpl->state;
}

bool TournamentSession::is_connected() const
{
    return this->pimpl->connected;
}

bool TournamentSession::is_authorized() const
{
    return this->pimpl->authorized;
}

QString TournamentSession::last_error() const
{
    return this->pimpl->last_error;
}

// String conversion helper implementations
QString TournamentSession::toString(FundingType type)
{
    switch (type) {
        case FundingType::Buyin: return QObject::tr("Buy-in");
        case FundingType::Rebuy: return QObject::tr("Rebuy");
        case FundingType::Addon: return QObject::tr("Add-on");
        default: return QObject::tr("Unknown");
    }
}

QString TournamentSession::toString(RebalancePolicy policy)
{
    switch (policy) {
        case RebalancePolicy::Manual: return QObject::tr("Manual");
        case RebalancePolicy::Automatic: return QObject::tr("Automatic");
        case RebalancePolicy::Shootout: return QObject::tr("Shootout");
        default: return QObject::tr("Unknown");
    }
}

QString TournamentSession::toString(PayoutPolicy policy)
{
    switch (policy) {
        case PayoutPolicy::Automatic: return QObject::tr("Automatic");
        case PayoutPolicy::Forced: return QObject::tr("Forced");
        case PayoutPolicy::Manual: return QObject::tr("Manual");
        default: return QObject::tr("Unknown");
    }
}

QString TournamentSession::toString(AnteType type)
{
    switch (type) {
        case AnteType::None: return QObject::tr("None");
        case AnteType::Traditional: return QObject::tr("Traditional");
        case AnteType::BigBlind: return QObject::tr("Big Blind");
        default: return QObject::tr("None");
    }
}

TournamentSession::FundingType TournamentSession::fundingTypeFromString(const QString& str)
{
    if (str == QObject::tr("Buy-in")) return FundingType::Buyin;
    if (str == QObject::tr("Rebuy")) return FundingType::Rebuy;
    if (str == QObject::tr("Add-on")) return FundingType::Addon;
    return FundingType::Buyin; // Default fallback
}

TournamentSession::RebalancePolicy TournamentSession::rebalancePolicyFromString(const QString& str)
{
    if (str == QObject::tr("Manual")) return RebalancePolicy::Manual;
    if (str == QObject::tr("Automatic")) return RebalancePolicy::Automatic;
    if (str == QObject::tr("Shootout")) return RebalancePolicy::Shootout;
    return RebalancePolicy::Manual; // Default fallback
}

TournamentSession::PayoutPolicy TournamentSession::payoutPolicyFromString(const QString& str)
{
    if (str == QObject::tr("Automatic")) return PayoutPolicy::Automatic;
    if (str == QObject::tr("Forced")) return PayoutPolicy::Forced;
    if (str == QObject::tr("Manual")) return PayoutPolicy::Manual;
    return PayoutPolicy::Automatic; // Default fallback
}

TournamentSession::AnteType TournamentSession::anteTypeFromString(const QString& str)
{
    if (str == QObject::tr("Traditional")) return AnteType::Traditional;
    if (str == QObject::tr("Big Blind")) return AnteType::BigBlind;
    if (str == QObject::tr("None")) return AnteType::None;
    return AnteType::None; // Default fallback to None
}
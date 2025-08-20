#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QMap>
#include <QList>
#include <QObject>
#include <QString>
#include <QVariant>
#include <functional>
#include <memory>

class TournamentService;

class TournamentSession : public QObject
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // send command
    void send_command(const QString& cmd, const QVariantMap& arg, const std::function<void(const QVariantMap&)>& result);

    // update state
    void update(const QVariantMap& new_state);

private Q_SLOTS:
    void on_connected();
    void on_disconnected();
    void on_receivedData(const QVariantMap& data);
    void on_connectionError(const QString& error);

public:
    // constants
    static const int default_action_clock_request_time = 60000;
    static const int default_audio_warning_time = 60000;

    // funding types (sync with enum funding_source_type_t in types.hpp)
    static const int FundingTypeBuyin = 0;
    static const int FundingTypeRebuy = 1;
    static const int FundingTypeAddon = 2;

    explicit TournamentSession(QObject* parent=nullptr);
    virtual ~TournamentSession();

    // client identifier (used for authenticating with servers)
    static int client_identifier();

    // connect to a tournament service
    void connect(const TournamentService& tournament);
    void disconnect();

    // tournament commands
    void check_authorized();
    void check_authorized_with_handler(const std::function<void(bool)>& handler);
    void get_state();
    void get_state_with_handler(const std::function<void(const QVariantMap&)>& handler);
    void get_config();
    void get_config_with_handler(const std::function<void(const QVariantMap&)>& handler);
    void configure(const QVariantMap& config);
    void configure_with_handler(const QVariantMap& config, const std::function<void(const QVariantMap&)>& handler);
    void reset_state();
    void start_game_at(const QDateTime& datetime);
    void start_game();
    void stop_game();
    void resume_game();
    void pause_game();
    void toggle_pause_game();
    void set_previous_level();
    void set_previous_level_with_handler(const std::function<void(int)>& handler);
    void set_next_level();
    void set_next_level_with_handler(const std::function<void(int)>& handler);
    void set_action_clock(int milliseconds=default_action_clock_request_time);
    void clear_action_clock();
    void gen_blind_levels(const QVariantMap& request);
    void gen_blind_levels_with_handler(const QVariantMap& request, const std::function<void(const QVariantList&)>& handler);
    void fund_player(const QString& player_id, int source);
    void plan_seating_for(int expected_players);
    void plan_seating_for_with_handler(int expected_players, const std::function<void(const QVariantList&)>& handler);
    void seat_player(const QString& player_id);
    void seat_player_with_handler(const QString& player_id, const std::function<void(const QString&, const QString&, const QString&, bool)>& handler);
    void unseat_player(const QString& player_id);
    void bust_player(const QString& player_id);
    void bust_player_with_handler(const QString& player_id, const std::function<void(const QVariantList&)>& handler);
    void rebalance_seating();
    void rebalance_seating_with_handler(const std::function<void(const QVariantList&)>& handler);
    void quick_setup();
    void quick_setup_with_handler(const std::function<void(const QVariantList&)>& handler);

    // serialization
    QByteArray results_as_csv() const;

    // accessors
    const QVariantMap& state() const;
    bool is_connected() const;
    bool is_authorized() const;
    QString last_error() const;

Q_SIGNALS:
    void connectedChanged(bool conn);
    void authorizedChanged(bool auth);
    void stateChanged(const QString& key, const QVariant& value);
    void connectionError(const QString& error);
    void commandError(const QString& command, const QString& error);
    void networkError(const QString& error);
};

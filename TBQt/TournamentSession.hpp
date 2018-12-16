#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QHash>
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
    void send_command(const QString& cmd, const QVariantHash& arg, const std::function<void(const QVariantHash&)>& result);

private Q_SLOTS:
    void tournament_connected();
    void tournament_disconnected();
    void received_data(const QVariantHash& data);

public:
    explicit TournamentSession(QObject* parent=nullptr);
    virtual ~TournamentSession();

    // client identifier (used for authenticating with servers)
    static int client_identifier();

    // connect to a tournament service
    void connect(const TournamentService& tournament);
    void disconnect();

    // tournament commands
    void check_authorized(const std::function<void(bool)>& handler);
    void get_state(const std::function<void(const QVariantHash&)>& handler);
    void get_config(const std::function<void(const QVariantHash&)>& handler);
    void configure(const QVariantHash& config, const std::function<void(const QVariantHash&)>& handler);
    void reset_state();
    void start_game_at(const QDateTime& datetime);
    void start_game();
    void stop_game();
    void resume_game();
    void pause_game();
    void toggle_pause_game();
    void set_previous_level(const std::function<void(int)>& handler);
    void set_next_level(const std::function<void(int)>& handler);
    void set_action_clock(int milliseconds);
    void clear_action_clock();
    void gen_blind_levels(const QVariantHash& request, std::function<void(const QVariantList&)>& handler);
    void fund_player(const QString& player_id, int source);
    void plan_seating_for(int expected_players, std::function<void(const QVariantList&)>& handler);
    void seat_player(const QString& player_id, std::function<void(const QString&, const QString&, const QString&, bool)>& handler);
    void unseat_player(const QString& player_id);
    void bust_player(const QString& player_id, std::function<void(const QVariantList&)>& handler);
    void rebalance_seating(std::function<void(const QVariantList&)>& handler);
    void quick_setup(std::function<void(const QVariantList&)>& handler);

    // serialization
    QByteArray results_as_csv() const;
};

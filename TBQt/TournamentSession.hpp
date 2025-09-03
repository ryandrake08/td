#pragma once

#include <QByteArray>
#include <QDateTime>
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

    // format clock time for display
    QString formatClockTime(qint64 timeValue, qint64 currentTime, bool countingDown);

private Q_SLOTS:
    void on_connected();
    void on_disconnected();
    void on_receivedData(const QVariantMap& data);
    void on_connectionError(const QString& error);

public:
    // type-safe enums
    enum class FundingType : int
    {
        Buyin = 0,
        Rebuy = 1,
        Addon = 2
    };

    enum class RebalancePolicy : int
    {
        Manual = 0,
        Automatic = 1,
        Shootout = 2
    };

    enum class PayoutPolicy : int
    {
        Automatic = 0,
        Forced = 1,
        Manual = 2
    };

    enum class AnteType : int
    {
        None = 0,
        Traditional = 1,
        BigBlind = 2
    };

    // conversion helpers for JSON serialization and use in Qt controls
    static constexpr int toInt(FundingType type) { return static_cast<int>(type); }
    static constexpr int toInt(RebalancePolicy policy) { return static_cast<int>(policy); }
    static constexpr int toInt(PayoutPolicy policy) { return static_cast<int>(policy); }
    static constexpr int toInt(AnteType type) { return static_cast<int>(type); }

    static FundingType toFundingType(int value) {
        Q_ASSERT(value >= 0 && value <= 2);
        return (value >= 0 && value <= 2) ? static_cast<FundingType>(value) : FundingType::Buyin;
    }
    static RebalancePolicy toRebalancePolicy(int value) {
        Q_ASSERT(value >= 0 && value <= 2);
        return (value >= 0 && value <= 2) ? static_cast<RebalancePolicy>(value) : RebalancePolicy::Manual;
    }
    static PayoutPolicy toPayoutPolicy(int value) {
        Q_ASSERT(value >= 0 && value <= 2);
        return (value >= 0 && value <= 2) ? static_cast<PayoutPolicy>(value) : PayoutPolicy::Automatic;
    }
    static AnteType toAnteType(int value) {
        Q_ASSERT(value >= 0 && value <= 2);
        return (value >= 0 && value <= 2) ? static_cast<AnteType>(value) : AnteType::None;
    }

    // string conversion helpers
    static QString toString(FundingType type);
    static QString toString(RebalancePolicy policy);
    static QString toString(PayoutPolicy policy);
    static QString toString(AnteType type);

    static FundingType fundingTypeFromString(const QString& str);
    static RebalancePolicy rebalancePolicyFromString(const QString& str);
    static PayoutPolicy payoutPolicyFromString(const QString& str);
    static AnteType anteTypeFromString(const QString& str);

    // action clock
    static const int kActionClockRequestTime = 60000;

    // audio warning
    static const int kAudioWarningTime = 60;

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
    void set_action_clock(int milliseconds);
    void clear_action_clock();
    void gen_blind_levels(const QVariantMap& request);
    void gen_blind_levels_with_handler(const QVariantMap& request, const std::function<void(const QVariantList&)>& handler);
    void fund_player(const QString& player_id, int source);
    void plan_seating(int expected_players);
    void plan_seating_with_handler(int expected_players, const std::function<void(const QVariantList&)>& handler);
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
    void playerMovementsUpdated(const QVariantList& movements);
    void connectionError(const QString& error);
    void commandError(const QString& command, const QString& error);
    void networkError(const QString& error);
};
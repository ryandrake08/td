#pragma once
#include "json.hpp"
#include "types.hpp"
#include <unordered_set>
#include <vector>

class gamefunding
{
public:
    typedef std::size_t currency;

    struct funding_source
    {
        static const std::size_t always_allow;

        bool is_addon;
        std::size_t forbid_after_blind_level;
        std::size_t chips;
        currency cost;
        currency commission;
        currency equity;
    };

private:
    // configuration: name of currency collected (USD, EUR, points)
    std::string cost_currency;

    // configuration: name of currency distributed (USD, EUR, points)
    std::string equity_currency;

    // configuration: rough percentage of seats that get paid (0.0-1.0)
    double percent_seats_paid;

    // configuration: funding rules
    std::vector<funding_source> funding_sources;

    // players who bought in at least once
    std::unordered_set<player_id> buyins;

    // payout structure
    std::vector<currency> payouts;

    // total game currency (chips) in play
    std::size_t total_chips;

    // total funds received
    currency total_cost;
    currency total_commission;

    // total funds to pay out
    currency total_equity;

    // re-calculate payouts
    void recalculate_payouts();

public:
    // initialize game funding rules
    gamefunding();

    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;

    // dump state to JSON
    void dump_state(json& state) const;

    // reset funding information back to zero
    void reset();

    // fund a player, (re-)buyin or addon
    void fund_player(const player_id& player, const funding_source& source, std::size_t current_blind_level);
};
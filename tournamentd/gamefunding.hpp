#pragma once
#include "json.hpp"
#include "types.hpp"
#include <unordered_set>
#include <vector>

class gamefunding
{
    // configuration: name of currency collected (USD, EUR, points)
    std::string cost_currency;

    // configuration: name of currency distributed (USD, EUR, points)
    std::string equity_currency;

    // configuration: rough percentage of seats that get paid (0.0-1.0)
    double percent_seats_paid;

    // configuration: funding rules
    std::vector<td::funding_source> funding_sources;

    // players who bought in at least once
    std::unordered_set<td::player_id> buyins;

    // payout structure
    std::vector<unsigned long> payouts;

    // total game currency (chips) in play
    unsigned long total_chips;

    // total funds received
    unsigned long total_cost;
    unsigned long total_commission;

    // total funds to pay out
    unsigned long total_equity;

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
    void fund_player(const td::player_id& player, const td::funding_source_id& src, std::size_t current_blind_level);
};
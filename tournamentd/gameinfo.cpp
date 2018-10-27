#include "gameinfo.hpp"
#include "datetime.hpp"
#include "json.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>

// special update_value for player map
template <>
bool json::update_value(const char* name, std::unordered_map<td::player_id_t,td::player>& values) const
{
    // read json into vector of players
    std::vector<td::player> new_values;
    if(this->get_value(name, new_values))
    {
        // check size first, if equal, deep compare
        if(values.size() == new_values.size())
        {
            size_t matches(0);
            for(const auto& new_value : new_values)
            {
                // look up by player_id
                auto it(values.find(new_value.player_id));

                // if found and the actual value matches test value, continue
                if(it != values.end() && it->second == new_value)
                {
                    matches++;
                }
                else
                {
                    // no need to test further. data is different
                    break;
                }
            }

            // all values match
            if(matches == values.size())
            {
                return false;
            }
        }

        // replace values
        values.clear();
        for(auto& new_value : new_values)
        {
            values.emplace(new_value.player_id, new_value);
        }
        return true;
    }

    return false;
}

// special update_value for manual payouts map
template <>
bool json::update_value(const char* name, std::unordered_map<size_t, std::vector<td::monetary_value_nocurrency>>& values) const
{
    // read json into vector of manual_payout
    std::vector<td::manual_payout> new_values;
    if(this->get_value(name, new_values))
    {
        // check size first, if equal, deep compare
        if(values.size() == new_values.size())
        {
            size_t matches(0);
            for(const auto& new_value : new_values)
            {
                // look up by player_id
                auto it(values.find(new_value.buyins_count));

                // if found and the actual value matches test value, continue
                if(it != values.end() && it->second == new_value.payouts)
                {
                    matches++;
                }
                else
                {
                    // no need to test further. data is different
                    break;
                }
            }

            // all values match
            if(matches == values.size())
            {
                return false;
            }
        }

        // replace values
        values.clear();
        for(auto& new_value : new_values)
        {
            values.emplace(new_value.buyins_count, new_value.payouts);
        }
        return true;
    }

    return false;
}

// initialize game
gameinfo::gameinfo() :
    table_capacity(2),
    blind_levels(1),
    payout_policy(td::payout_policy_t::automatic),
    previous_blind_level_hold_duration(2000),
    rebalance_policy(td::rebalance_policy_t::manual),
    final_table_policy(td::final_table_policy_t::fill),
    dirty(true),
    tables(0),
    total_chips(0),
    total_equity(0.0),
    current_blind_level(0)
{
}

// get current time
gameinfo::time_point_t gameinfo::now()
{
    return std::chrono::system_clock::now();
}

// load configuration from JSON (object or file)
void gameinfo::configure(const json& config)
{
    logger(ll::info) << "loading tournament configuration\n";

    if(config.update_value("name", this->name))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: name -> " << this->name << '\n';
    }

    if(config.update_value("funding_sources", this->funding_sources))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: funding_sources -> " << this->funding_sources.size() << " sources\n";
    }

    if(config.update_value("previous_blind_level_hold_duration", this->previous_blind_level_hold_duration))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: previous_blind_level_hold_duration -> " << this->previous_blind_level_hold_duration << '\n';
    }

    // TODO: changing the rebalance policy could trigger an immediate rebalance. for now, we wait until the next bust-out
    if(config.update_value("rebalance_policy", reinterpret_cast<int&>(this->rebalance_policy)))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: rebalance_policy -> " << this->rebalance_policy << '\n';
    }

    if(config.update_value("background_color", this->background_color))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: background_color -> " << this->background_color << '\n';
    }

    if(config.update_value("final_table_policy", reinterpret_cast<int&>(this->final_table_policy)))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: final_table_policy -> " << this->final_table_policy << '\n';
    }

    if(config.update_value("available_chips", this->available_chips))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: available_chips -> " << this->available_chips.size() << " chips\n";

        // always sort chips by denomination
        std::sort(this->available_chips.begin(), this->available_chips.end(),
                  [](const td::chip& c0, const td::chip& c1)
                  {
                      return c0.denomination < c1.denomination;
                  });
    }

    if(config.update_value("players", this->players))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: players -> " << this->players.size() << " players\n";

        if(!this->seats.empty() || !this->players_finished.empty() || !this->bust_history.empty() || !this->buyins.empty() || !this->unique_entries.empty() || !this->entries.empty())
        {
            logger(ll::warning) << "re-coniguring players list while in play is not advised, deleted players may still be in the game\n";
        }
    }

    // changing the table capacity can cause a re-plan
    if(config.update_value("table_capacity", this->table_capacity))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: table_capacity -> " << this->table_capacity << '\n';

        if(!this->seats.empty())
        {
            logger(ll::warning) << "re-configuring table capacity will clear seating plan\n";
        }

        // if seats are already set up, replan based on new table capacity
        std::size_t total_seats = this->seats.size() + this->empty_seats.size();
        if(total_seats >= 2)
        {
            // re-plan seating, if needed. TODO: for now, ignore any movements
            (void) this->plan_seating(total_seats);
        }
    }

    // recalculate for any configuration that could alter payouts
    auto recalculate(false);
    if(config.update_value("payout_policy", reinterpret_cast<int&>(this->payout_policy)))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: payout_policy -> " << this->payout_policy << '\n';

        recalculate = true;
    }

    if(config.update_value("payout_currency", this->payout_currency))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: payout_currency -> " << this->payout_currency << '\n';

        recalculate = true;
    }

    if(config.update_value("automatic_payouts", this->automatic_payouts))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: automatic_payouts -> (reconfigured)\n";

        recalculate = true;
    }

    if(config.update_value("forced_payouts", this->forced_payouts))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: forced_payouts -> " << this->forced_payouts.size() << " forced payouts\n";

        recalculate = true;
    }

    if(config.update_value("manual_payouts", this->manual_payouts))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: manual_payouts -> " << this->manual_payouts.size() << " manual payouts\n";

        recalculate = true;
    }

    if(recalculate)
    {
        // after reconfiguring, we'll need to recalculate
        this->recalculate_payouts();
    }

    // stop the game when reconfiguring blind levels
    if(config.update_value("blind_levels", this->blind_levels))
    {
        this->dirty = true;
        logger(ll::info) << "configuration changed: blind_levels -> " << this->blind_levels.size() << " blind levels\n";

        if(this->is_started())
        {
            logger(ll::warning) << "re-configuring blind levels while in play may change the current blind level\n";
        }
    }

    // ensure we have at least one blind level, the setup level
    if(this->blind_levels.empty())
    {
        this->dirty = true;
        logger(ll::info) << "configuration validated: blind_levels -> " << this->blind_levels.size() << " blind levels\n";

        this->blind_levels.resize(1);
    }

    // can also load state (useful for loading from snapshot)

    if(config.get_value("seats", this->seats))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: seats -> " << this->seats.size() << "\n";
    }

    if(config.get_value("players_finished", this->players_finished))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: players_finished -> " << this->players_finished.size() << "\n";
    }

    if(config.get_value("bust_history", this->bust_history))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: bust_history -> " << this->bust_history.size() << "\n";
    }

    if(config.get_value("empty_seats", this->empty_seats))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: empty_seats -> " << this->empty_seats.size() << "\n";
    }

    if(config.get_value("tables", this->tables))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: tables -> " << this->tables << "\n";
    }

    if(config.get_value("buyins", this->buyins))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: buyins -> " << this->buyins.size() << "\n";
    }

    if(config.get_value("unique_entries", this->unique_entries))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: unique_entries -> " << this->unique_entries.size() << "\n";
    }

    if(config.get_value("entries", this->entries))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: entries -> " << this->entries.size() << "\n";
    }

    if(config.get_value("payouts", this->payouts))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: payouts -> " << this->payouts.size() << "\n";
    }

    if(config.get_value("total_chips", this->total_chips))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: total_chips -> " << this->total_chips << "\n";
    }

    if(config.get_value("total_cost", this->total_cost))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: total_cost -> " << this->total_cost.size() << "\n";
    }

    if(config.get_value("total_commission", this->total_commission))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: total_commission -> " << this->total_commission.size() << "\n";
    }

   if(config.get_value("total_equity", this->total_equity))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: total_equity -> " << this->total_equity << "\n";
    }

    if(config.get_value("current_blind_level", this->current_blind_level))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: current_blind_level -> " << this->current_blind_level << "\n";
    }

    if(config.get_value("end_of_round", this->end_of_round))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: end_of_round -> " << datetime(this->end_of_round) << "\n";
    }

    if(config.get_value("end_of_break", this->end_of_break))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: end_of_break -> " << datetime(this->end_of_break) << "\n";
    }

    if(config.get_value("end_of_action_clock", this->end_of_action_clock))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: end_of_action_clock -> " << datetime(this->end_of_action_clock) << "\n";
    }

    if(config.get_value("tournament_start", this->tournament_start))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: tournament_start -> " << datetime(this->tournament_start) << "\n";
    }

    if(config.get_value("paused_time", this->paused_time))
    {
        this->dirty = true;
        logger(ll::info) << "state changed: paused_time -> " << datetime(this->paused_time) << "\n";
    }
}

// dump configuration to JSON
void gameinfo::dump_configuration(json& config) const
{
    logger(ll::debug) << "dumping tournament configuration\n";

    config.set_value("name", this->name);
    config.set_value("players", json(this->players.begin(), this->players.end()));
    config.set_value("table_capacity", this->table_capacity);
    config.set_value("payout_policy", static_cast<int>(this->payout_policy));
    config.set_value("payout_currency", this->payout_currency);
    config.set_value("automatic_payouts", this->automatic_payouts);
    config.set_value("forced_payouts", json(this->forced_payouts.begin(), this->forced_payouts.end()));
    config.set_value("manual_payouts", json(this->manual_payouts.begin(), this->manual_payouts.end()));
    config.set_value("previous_blind_level_hold_duration", this->previous_blind_level_hold_duration);
    config.set_value("rebalance_policy", static_cast<int>(this->rebalance_policy));
    config.set_value("background_color", this->background_color);
    config.set_value("final_table_policy", static_cast<int>(this->final_table_policy));
    config.set_value("funding_sources", json(this->funding_sources.begin(), this->funding_sources.end()));
    config.set_value("blind_levels", json(this->blind_levels.begin(), this->blind_levels.end()));
    config.set_value("available_chips", json(this->available_chips.begin(), this->available_chips.end()));
}

// dump state to JSON
void gameinfo::dump_state(json& state) const
{
    logger(ll::debug) << "dumping tournament state\n";

    state.set_value("seats", json(this->seats.begin(), this->seats.end()));
    state.set_value("players_finished", json(this->players_finished.begin(), this->players_finished.end()));
    state.set_value("bust_history", json(this->bust_history.begin(), this->bust_history.end()));
    state.set_value("empty_seats", json(this->empty_seats.begin(), this->empty_seats.end()));
    state.set_value("tables", this->tables);
    state.set_value("buyins", json(this->buyins.begin(), this->buyins.end()));
    state.set_value("unique_entries", json(this->unique_entries.begin(), this->unique_entries.end()));
    state.set_value("entries", json(this->entries.begin(), this->entries.end()));
    state.set_value("payouts", json(this->payouts.begin(), this->payouts.end()));
    state.set_value("total_chips", this->total_chips);
    state.set_value("total_cost", json(this->total_cost.begin(), this->total_cost.end()));
    state.set_value("total_commission", json(this->total_commission.begin(), this->total_commission.end()));
    state.set_value("total_equity", this->total_equity);
    state.set_value("current_blind_level", this->current_blind_level);
    state.set_value("end_of_round", this->end_of_round);
    state.set_value("end_of_break", this->end_of_break);
    state.set_value("end_of_action_clock", this->end_of_action_clock);
    state.set_value("tournament_start", this->tournament_start);
    state.set_value("paused_time", this->paused_time);
}

// some configuration gets sent to clients along with state, dump to JSON
void gameinfo::dump_configuration_state(json &state) const
{
    logger(ll::debug) << "dumping tournament configuration state\n";

    state.set_value("name", this->name);
    state.set_value("background_color", this->background_color);
    state.set_value("funding_sources", json(this->funding_sources.begin(), this->funding_sources.end()));
    state.set_value("available_chips", json(this->available_chips.begin(), this->available_chips.end()));
}

// blind level to string
static std::ostream& operator<<(std::ostream& os, const td::blind_level& level)
{
    os << level.little_blind << '/' << level.big_blind;
    if(level.ante_type == td::ante_type_t::traditional)
    {
        // TODO: i18n
        os << " Ante:" << level.ante;
    }
    else if(level.ante_type == td::ante_type_t::bba)
    {
        // TODO: i18n
        os << " BBA:" << level.ante;
    }
    return os;
}

// funding source to string
static std::ostream& operator<<(std::ostream& os, const td::funding_source& src)
{
    os << src.name << ": " << src.cost.currency << src.cost.amount;
    if(src.commission.amount != 0.0)
    {
        if(src.commission.currency == src.cost.currency)
        {
            os << '+' << src.commission.amount;
        }
        else
        {
            os << '+' << src.commission.currency << src.commission.amount;
        }
    }
    return os;
}

// calculate derived state and dump to JSON
void gameinfo::dump_derived_state(json& state) const
{
    logger(ll::debug) << "dumping tournament derived state\n";

    auto now(this->now());

    // set current time (for synchronization)
    state.set_value("current_time", now);

    // set elapsed_time if we are past the tournament start
    if(this->tournament_start != time_point_t() && this->tournament_start < now)
    {
        auto elapsed_time(std::chrono::duration_cast<duration_t>(now - this->tournament_start));
        state.set_value("elapsed_time", elapsed_time.count());
    }

    // set action clock if ticking
    if(this->end_of_action_clock != time_point_t() && this->end_of_action_clock > now)
    {
        auto action_clock_time_remaining(std::chrono::duration_cast<duration_t>(this->end_of_action_clock - now));
        state.set_value("action_clock_time_remaining", action_clock_time_remaining.count());
    }
    else
    {
        state.set_value("action_clock_time_remaining", 0);
    }

    // set running (vs paused)
    state.set_value("running", this->end_of_break != time_point_t() && !this->is_paused());

    std::ostringstream os;
    os.imbue(std::locale(""));

    // current round number as text
    if(this->is_started())
    {
        os << this->current_blind_level;
        state.set_value("current_round_number_text", os.str()); os.str("");
    }

    // current game name text
    if(this->is_started() && this->current_blind_level < this->blind_levels.size())
    {
        os << blind_levels[this->current_blind_level].game_name;
        state.set_value("current_game_text", os.str()); os.str("");
    }

    // next game name text
    if(this->is_started() && this->current_blind_level+1 < this->blind_levels.size())
    {
        os << blind_levels[this->current_blind_level+1].game_name;
        state.set_value("next_game_text", os.str()); os.str("");
    }

    if(!this->is_paused())
    {
        // set time remaining based on current clock
        if(this->end_of_round != time_point_t() && now < this->end_of_round)
        {
            // within round, set time remaining
            auto time_remaining(std::chrono::duration_cast<duration_t>(this->end_of_round - now));
            state.set_value("time_remaining", time_remaining.count());
            state.set_value("clock_remaining", time_remaining.count());
            state.set_value("on_break", false);

            if(this->current_blind_level < this->blind_levels.size())
            {
                // set blinds description
                os << this->blind_levels[this->current_blind_level].little_blind << '/' << this->blind_levels[this->current_blind_level].big_blind;
                state.set_value("current_round_blinds_text", os.str()); os.str("");

                // set antes description
                if(this->blind_levels[this->current_blind_level].ante_type == td::ante_type_t::traditional)
                {
                    // TODO: i18n
                    os << "Ante:" << this->blind_levels[this->current_blind_level].ante;
                }
                else if(this->blind_levels[this->current_blind_level].ante_type == td::ante_type_t::bba)
                {
                    // TODO: i18n
                    os << "BBA:" << this->blind_levels[this->current_blind_level].ante;
                }
                state.set_value("current_round_ante_text", os.str()); os.str("");

                // set next round description
                if(this->blind_levels[this->current_blind_level].break_duration == 0)
                {
                    os << this->blind_levels[this->current_blind_level+1];
                }
                else if(this->current_blind_level+1 < this->blind_levels.size())
                {
                    os << "BREAK"; // TODO: i18n
                }
                state.set_value("next_round_text", os.str()); os.str("");
            }
        }
        else if(this->end_of_break != time_point_t() && now < this->end_of_break)
        {
            // within break, set break time remaining
            auto break_time_remaining(std::chrono::duration_cast<duration_t>(this->end_of_break - now));
            state.set_value("break_time_remaining", break_time_remaining.count());
            state.set_value("clock_remaining", break_time_remaining.count());
            state.set_value("on_break", true);

            // describe break
            state.set_value("current_round_blinds_text", std::string("BREAK")); // TODO: i18n

            // set next round description
            if(this->current_blind_level+1 < this->blind_levels.size())
            {
                os << this->blind_levels[this->current_blind_level+1];
                state.set_value("next_round_text", os.str()); os.str("");
            }
        }
    }

    // players left text
    if(!this->seats.empty())
    {
        os << this->seats.size();
        state.set_value("players_left_text", os.str()); os.str("");
    }

    // unique_entries text
    if(!this->unique_entries.empty())
    {
        os << this->unique_entries.size();
        state.set_value("unique_entries_text", os.str()); os.str("");
    }

    // entries text
    if(!this->entries.empty())
    {
        os << this->entries.size();
        state.set_value("entries_text", os.str()); os.str("");
    }

    // average stack text
    if(!this->buyins.empty())
    {
        os << this->total_chips / this->buyins.size();
        state.set_value("average_stack_text", os.str()); os.str("");
    }

    // buyin text
    auto src_it(std::find_if(this->funding_sources.begin(), this->funding_sources.end(), [](const td::funding_source& s) { return s.type == td::funding_source_type_t::buyin; }));
    if(src_it != this->funding_sources.end())
    {
        os << *src_it;
    }
    else
    {
        os << "NO BUYIN"; // TODO: i18n
    }
    state.set_value("buyin_text", os.str()); os.str("");

    // results
    std::vector<td::result> results;
    for(size_t j(0); j<this->buyins.size(); j++)
    {
        td::result result(j+1);
        if(j < this->payouts.size())
        {
            result.payout = this->payouts[j];
        }
        results.push_back(result);
    }
    for(size_t i(0); i<this->players_finished.size(); i++)
    {
        auto player_id(this->players_finished[i]);
        size_t j(this->buyins.size()+i);

        td::result result(j+1, this->player_name(player_id));
        if(j < this->payouts.size())
        {
            result.payout = this->payouts[j];
        }
        results.push_back(result);
    }
    state.set_value("results", json(results.begin(), results.end()));

    // seated players
    std::vector<td::seated_player> seated_players;
    for(const auto& p : this->players)
    {
        auto buyin(this->buyins.find(p.first));
        auto seat(this->seats.find(p.first));
        if(seat == this->seats.end())
        {
            td::seated_player sp(p.first, p.second.name, buyin != this->buyins.end());
            seated_players.push_back(sp);
        }
        else
        {
            td::seated_player sp(p.first, p.second.name, buyin != this->buyins.end(), seat->second.table_number, seat->second.seat_number);
            seated_players.push_back(sp);
        }
    }
    state.set_value("seated_players", json(seated_players.begin(), seated_players.end()));
}

// has internal state been updated since last check?
bool gameinfo::state_is_dirty()
{
    if(this->dirty)
    {
        this->dirty = false;
        return true;
    }
    else
    {
        return false;
    }
}

void gameinfo::reset_state()
{
    // set state dirty
    this->dirty = true;

    // clear results
    this->players_finished.clear();
    this->bust_history.clear();

    // clear all seating and remove all empty seats
    this->seats.clear();
    this->empty_seats.clear();
    this->tables = 0;

    // clear all funding
    this->buyins.clear();
    this->unique_entries.clear();
    this->entries.clear();
    this->payouts.clear();
    this->total_chips = 0;
    this->total_cost.clear();
    this->total_commission.clear();
    this->total_equity = 0.0;

    // stop clock
    this->stop();
}

const std::string gameinfo::player_name(const td::player_id_t& player_id) const
{
    auto player_it(this->players.find(player_id));
    if(player_it == this->players.end())
    {
        throw std::runtime_error("failed to look up player: " + player_id);
    }
    return player_it->second.name;
}

const std::string gameinfo::player_description(const td::player_id_t& player_id) const
{
    return player_id + " (" + this->player_name(player_id) + ")";
}

std::vector<std::vector<td::player_id_t> > gameinfo::players_at_tables() const
{
    // build up two vectors, outer = tables, inner = players per table
    std::vector<std::vector<td::player_id_t> > ret(this->tables);

    for(auto& seat : this->seats)
    {
        ret[seat.second.table_number].push_back(seat.first);
    }

    for(std::size_t i(0); i<ret.size(); i++)
    {
        logger(ll::debug) << "table " << i << " has " << ret[i].size() << " players\n";
    }

    return ret;
}

std::vector<td::player_movement> gameinfo::plan_seating(std::size_t max_expected)
{
    // return value is any movements that have to happen
    std::vector<td::player_movement> movements;

    logger(ll::info) << "planning tournament for " << max_expected << " players\n";

    // check arguments
    if(max_expected < 2)
    {
        throw td::protocol_error("expected players must be at least 2");
    }

    // check against number of already seated players
    if(max_expected < this->seats.size())
    {
        throw td::protocol_error("expected players less than currently seated players");
    }

    // check configuration: table capacity should be sane
    if(this->table_capacity < 2)
    {
        throw td::protocol_error("table capacity must be at least 2");
    }

    // figure out how many tables needed
    auto tables_needed(((max_expected-1) / this->table_capacity) + 1);
    logger(ll::info) << "tables needed: " << tables_needed << "\n";

    // if this plan would require a different number of tables than already exist, then we need to re-plan
    if(tables_needed != this->tables)
    {
        // set state dirty
        this->dirty = true;

        // reset empty seats
        this->empty_seats.clear();

        // set new number of tables
        this->tables = tables_needed;

        // figure out how many seats should be first occupied at each table
        std::size_t preferred_seats = (max_expected + this->tables - 1) / this->tables;
        logger(ll::info) << "prefer: " << preferred_seats << " seats per table\n";

        // build up preferred seat list
        for(std::size_t t(0); t<this->tables; t++)
        {
            for(std::size_t s(0); s<preferred_seats; s++)
            {
                this->empty_seats.push_back(td::seat(t,s));
            }
        }

        // store iterator to start of extra seats
        auto extra_it(this->empty_seats.end());

        // add remaining seats, up to table capacity
        for(std::size_t t(0); t<this->tables; t++)
        {
            for(std::size_t s(preferred_seats); s<this->table_capacity; s++)
            {
                this->empty_seats.push_back(td::seat(t,s));
            }
        }

        // randomize preferred then extra seats separately
        std::shuffle(this->empty_seats.begin(), extra_it, this->random_engine);
        std::shuffle(extra_it, this->empty_seats.end(), this->random_engine);

        // re-seat players and record movements
        std::unordered_map<td::player_id_t,td::seat> new_seats;
        for(const auto& p : this->seats)
        {
            // seat player and remove from empty list
            auto seat(this->empty_seats.front());
            new_seats.insert(std::make_pair(p.first, seat));
            this->empty_seats.pop_front();

            // record movement
            movements.push_back(td::player_movement(p.first, this->player_name(p.first), p.second, seat));
        }

        // swap
        std::swap(new_seats, this->seats);

        logger(ll::info) << "created " << this->empty_seats.size() << " empty seats for " << max_expected << " expected players, re-seating " << new_seats.size() << " players\n";
    }

    return movements;
}

// add player to an existing game
std::pair<std::string, td::seat> gameinfo::add_player(const td::player_id_t& player_id)
{
    // find player in existing seating plan
    auto seat_it(this->seats.find(player_id));
    if(seat_it != this->seats.end())
    {
        auto seat(seat_it->second);
        logger(ll::info) << "player " << this->player_description(player_id) << " already seated at table " << seat.table_number << ", seat " << seat.seat_number << '\n';
        return std::make_pair("already_seated", seat);
    }
    else
    {
        logger(ll::info) << "adding player " << this->player_description(player_id) << " to game\n";

        // while no more empty seats left
        while(this->empty_seats.empty())
        {
            // plan for one more player. TODO: unfortunately, this may re-seat all players if # of tables changes, have to ignore these for now
            (void) this->plan_seating(this->seats.size()+1);
        }

        // set state dirty
        this->dirty = true;

        // seat player and remove from empty list
        auto seat(this->empty_seats.front());
        this->seats.insert(std::make_pair(player_id, seat));
        this->empty_seats.pop_front();

        logger(ll::info) << "seated player " << this->player_description(player_id) << " at table " << seat.table_number << ", seat " << seat.seat_number << '\n';

        return std::make_pair("player_seated", seat);
    }
}

// remove a player
td::seat gameinfo::remove_player(const td::player_id_t& player_id)
{
    logger(ll::info) << "removing player " << this->player_description(player_id) << " from game\n";

    auto seat_it(this->seats.find(player_id));

    if(seat_it == this->seats.end())
    {
        throw td::protocol_error("tried to remove player not seated");
    }

    // set state dirty
    this->dirty = true;

    // remove player and add seat to the end of the empty list
    auto seat(seat_it->second);
    this->empty_seats.push_back(seat);
    this->seats.erase(seat_it);

    return seat;
}

// remove a player
std::vector<td::player_movement> gameinfo::bust_player(const td::player_id_t& player_id)
{
    // check whether player is bought in
    if(this->buyins.find(player_id) == this->buyins.end())
    {
        throw td::protocol_error("tried to bust player not bought in");
    }

    // set state dirty
    this->dirty = true;

    // remove the player
    this->remove_player(player_id);

    logger(ll::info) << "busting player " << this->player_description(player_id) << " from the game\n";

    // add to the busted out list
    this->players_finished.push_front(player_id);
    this->bust_history.push_back(player_id);

    // mark as no longer bought in
    this->buyins.erase(player_id);

    // try to break table or rebalance
    std::vector<td::player_movement> movements;

    switch(this->rebalance_policy)
    {
        case td::rebalance_policy_t::manual:
            // for manual rebalancing, do nothing with tables when a player busts out
            logger(ll::debug) << "manual rebalancing in effect. not trying to break tables or rebalance\n";
            break;

        case td::rebalance_policy_t::automatic:
            // for automatic rebalancing, try to break tables and rebalance every time a player busts out
            movements = this->rebalance_seating();
            break;

        case td::rebalance_policy_t::shootout:
            // for shootout tournaments, only break tables when there is one player left on each table or fewer players
            if(this->tables >= this->seats.size())
            {
                movements = this->rebalance_seating();
            }
            break;
    }

    // collect bought-in players still seated
    struct collector
    {
        const std::unordered_set<td::player_id_t>& container;
        std::vector<td::player_id_t> playing;
        collector(const std::unordered_set<td::player_id_t>& c) : container(c) {}
        void operator()(const std::pair<td::player_id_t,td::seat>& s)
        {
            if(container.find(s.first) != container.end())
            {
                playing.push_back(s.first);
            }
        }
        operator std::vector<td::player_id_t>() { return playing; }
    };
    std::vector<td::player_id_t> playing(std::for_each(this->seats.begin(), this->seats.end(), collector(this->buyins)));

    // if only one bought-in players still seated
    if(playing.size() == 1)
    {
        auto first_place_player_id(playing.front());

        // bust player
        this->bust_player(first_place_player_id);

        logger(ll::info) << "winning player " << this->player_description(first_place_player_id) << '\n';

        // stop the game
        this->stop();
    }

    return movements;
}

template <typename T>
static constexpr bool has_lower_size(const T& i0, const T& i1)
{
    return i0.size() < i1.size();
}

// try to break and rebalance tables
// returns description of movements
std::vector<td::player_movement> gameinfo::rebalance_seating()
{
    std::vector<td::player_movement> movements;

    if(this->tables > 1)
    {
        logger(ll::info) << "attemptying to break a table\n";
        
        // break tables while (player_count-1) div table_capacity < tables
        while(this->seats.size() <= this->table_capacity * (this->tables-1))
        {
            logger(ll::info) << "breaking a table. " << this->seats.size() << " players remain at " << this->tables << " tables of " << this->table_capacity << " capacity\n";

            // always break the highest-numbered table
            auto break_table(this->tables - 1);

            // get each player found to be sitting at the breaking table
            std::vector<td::player_id_t> to_move;
            for(auto& seat : this->seats)
            {
                if(seat.second.table_number == break_table)
                {
                    to_move.push_back(seat.first);
                }
            }

            // move each player in list
            const std::unordered_set<std::size_t> avoid = {break_table};
            for(auto& player : to_move)
            {
                movements.push_back(this->move_player(player, avoid));
            }

            // set state dirty
            this->dirty = true;

            // decrement number of tables
            this->tables--;

            // prune empty table from our open seat list, no need to seat people at unused tables
            this->empty_seats.erase(std::remove_if(this->empty_seats.begin(), this->empty_seats.end(), [&break_table](const td::seat& seat) { return seat.table_number == break_table; }), this->empty_seats.end());

            logger(ll::info) << "broken table " << break_table << ". " << this->seats.size() << " players now at " << this->tables << " tables\n";
        }
    }

    // if we should randomize the non-empty final table seats
    if(this->tables == 1 && this->final_table_policy == td::final_table_policy_t::randomize)
    {
        logger(ll::info) << "randomizing remaining seats\n";

        // get the current list of seats
        std::vector<td::seat> to;
        for(const auto& s : this->seats)
        {
            to.push_back(s.second);
        }

        // shuffle it
        std::shuffle(to.begin(), to.end(), this->random_engine);

        // iterate through again, assigning new seats
        for(auto& s : this->seats)
        {
            const auto& to_seat = to.back();

            logger(ll::info) << "moved player " << this->player_description(s.first) << " from table " << s.second.table_number << ", seat " << s.second.seat_number << " to table " << to_seat.table_number << ", seat " << to_seat.seat_number << '\n';

            // add a new movement
            movements.push_back(td::player_movement(s.first, this->player_name(s.first), s.second, to.back()));

            // set new seat
            s.second = to.back();
            to.pop_back();
        }
    }

    // build players-per-table vector
    auto ppt(this->players_at_tables());
    if(!ppt.empty())
    {
        logger(ll::info) << "attemptying to rebalance tables\n";

        // find smallest and largest tables
        auto fewest_it(std::min_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id_t> >));
        auto most_it(std::max_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id_t> >));

        // if fewest has two fewer players than most (e.g. 6 vs 8), then rebalance
        while(!most_it->empty() && fewest_it->size() < most_it->size() - 1)
        {
            logger(ll::info) << "largest table has " << most_it->size() << " players and smallest table has " << fewest_it->size() << " players\n";

            // pick a random player at the table with the most players
            auto index(std::uniform_int_distribution<std::size_t>(0, most_it->size()-1)(this->random_engine));
            auto random_player((*most_it)[index]);

            // subtract iterator to find table number
            auto table(fewest_it - ppt.begin());
            movements.push_back(this->move_player(random_player, table));

            // update our lists to stay consistent
            (*most_it)[index] = most_it->back();
            most_it->pop_back();
            fewest_it->push_back(random_player);
        }
    }

    // TODO: consolidate any players that moved multiple times

    return movements;
}

// move a player to a specific table
td::player_movement gameinfo::move_player(const td::player_id_t& player_id, std::size_t table)
{
    logger(ll::info) << "moving player " << this->player_description(player_id) << " to table " << table << '\n';

    // build up a list of candidate seats
    std::vector<std::deque<td::seat>::iterator> candidates;
    for(auto seat_it(this->empty_seats.begin()); seat_it != this->empty_seats.end(); seat_it++)
    {
        if(seat_it->table_number == table)
        {
            // store iterator itself
            candidates.push_back(seat_it);
        }
    }

    logger(ll::info) << "choosing from " << candidates.size() << " free seats\n";

    // we should always have at least one seat free
    if(candidates.empty())
    {
        throw td::protocol_error("tried to move player to a full table");
    }

    // pick one at random
    auto index(std::uniform_int_distribution<std::size_t>(0, candidates.size()-1)(this->random_engine));
    auto to_seat_it(candidates[index]);

    // set state dirty
    this->dirty = true;

    // move player
    auto player_seat_it(this->seats.find(player_id));
    auto from_seat(player_seat_it->second);
    player_seat_it->second = *to_seat_it;
    this->empty_seats.push_back(from_seat);
    this->empty_seats.erase(to_seat_it);

    logger(ll::info) << "moved player " << this->player_description(player_id) << " from table " << from_seat.table_number << ", seat " << from_seat.seat_number << " to table " << player_seat_it->second.table_number << ", seat " << player_seat_it->second.seat_number << '\n';

    return td::player_movement(player_id, this->player_name(player_id), from_seat, player_seat_it->second);
}

// move a player to the table with the smallest number of players
td::player_movement gameinfo::move_player(const td::player_id_t& player_id, const std::unordered_set<std::size_t>& avoid_tables)
{
    logger(ll::info) << "moving player " << this->player_description(player_id) << " to a free table\n";

    // build players-per-table vector
    auto ppt(this->players_at_tables());

    // find first table not in avoid set
    std::size_t smallest_table(0);
    while(avoid_tables.find(smallest_table) != avoid_tables.end())
    {
        smallest_table++;
    }

    // find smallest table not in avoid set
    auto table(smallest_table);
    while(table < ppt.size())
    {
        // if current table is smaller and not in avoid_tables set
        if((ppt[table].size() < ppt[smallest_table].size()) && (avoid_tables.find(table) == avoid_tables.end()))
        {
            smallest_table = table;
        }
        table++;
    }

    // make sure at least one candidate table
    if(smallest_table >= ppt.size())
    {
        throw td::protocol_error("tried to move player to another table but no candidate tables");
    }

    return this->move_player(player_id, smallest_table);
}

// fund a player, (re-)buyin or addon
void gameinfo::fund_player(const td::player_id_t& player_id, const td::funding_source_id_t& src)
{
    if(src >= this->funding_sources.size())
    {
        throw td::protocol_error("invalid funding source");
    }

    td::funding_source& source(this->funding_sources[src]);

    if(this->current_blind_level > source.forbid_after_blind_level)
    {
        throw td::protocol_error("too late in the game for this funding source");
    }

    if(source.type != td::funding_source_type_t::buyin && this->unique_entries.find(player_id) == this->unique_entries.end())
    {
        throw td::protocol_error("tried a non-buyin funding source but not bought in yet");
    }

    if(source.type == td::funding_source_type_t::rebuy && this->current_blind_level < 1)
    {
        throw td::protocol_error("tried re-buying before tournamnet start");
    }

    logger(ll::info) << "funding player " << this->player_description(player_id) << " with " << source.name << '\n';

    // set state dirty
    this->dirty = true;

    if(source.type == td::funding_source_type_t::buyin)
    {
        // add player to buyin set
        this->buyins.insert(player_id);

        // add player to unique entry set
        this->unique_entries.insert(player_id);

        // add to entries
        this->entries.push_back(player_id);

        // remove from finished players list if existing (re-entry), do not remove from bust history
        this->players_finished.erase(std::remove(this->players_finished.begin(), this->players_finished.end(), player_id), this->players_finished.end());
    }
    else if(source.type == td::funding_source_type_t::rebuy)
    {
        // add player to buyin set
        this->buyins.insert(player_id);

        // add to entries
        this->entries.push_back(player_id);

        // add to bust history but keep seat
        this->bust_history.push_back(player_id);
    }

    // update totals
    this->total_chips += source.chips;
    this->total_cost[source.cost.currency] += source.cost.amount;
    this->total_commission[source.commission.currency] += source.commission.amount;
    this->total_equity += source.equity.amount;

    // automatically recalculate
    this->recalculate_payouts();
}

// return the maximum number of chips available per player for a given denomination
size_t gameinfo::max_chips_for(unsigned long denomination, std::size_t players_count) const
{
    if(players_count == 0)
    {
        throw td::protocol_error("players_count must be non-zero");
    }

    // find denomination in available_chips
    auto it(std::find_if(this->available_chips.begin(), this->available_chips.end(), [denomination](const td::chip& c) { return c.denomination == denomination; }));
    if(it == this->available_chips.end())
    {
        throw td::protocol_error("denomination not found in chip set");
    }

    return it->count_available / players_count;
}

// return a default funding source of the given type. used for quick setup and structure generator
td::funding_source_id_t gameinfo::source_for_type(const td::funding_source_type_t& type) const
{
    if(this->funding_sources.empty())
    {
        throw td::protocol_error("tried to look up a funding source with none defined");
    }

    // simple: return the first source that matches
    for(td::funding_source_id_t src(0); src<this->funding_sources.size(); src++)
    {
        if(this->funding_sources[src].type == type)
        {
            return src;
        }
    }

    // throw if none exist
    throw td::protocol_error("no funding sources of given type exist");
}

// calculate number of chips per denomination for this funding source, given totals and number of players
// this was tuned to produce a sensible result for the following chip denomination sets:
//  1/5/25/100/500
//  5/25/100/500/1000
//  25/100/500/1000/5000
std::vector<td::player_chips> gameinfo::chips_for_buyin(const td::funding_source_id_t& src, std::size_t max_expected) const
{
    if(src >= this->funding_sources.size())
    {
        throw td::protocol_error("invalid funding source");
    }

    const td::funding_source& source(this->funding_sources[src]);

    if(this->blind_levels.size() < 2)
    {
        throw td::protocol_error("tried to calculate chips for a buyin without blind levels defined");
    }

    if(this->available_chips.empty())
    {
        throw td::protocol_error("tried to calculate chips for a buyin without chips defined");
    }

    if(this->blind_levels.size() < 2)
    {
        throw td::protocol_error("tried to calcualate chips for a buyin without at least one blind level defined");
    }

    // ensure our smallest available chip can play the smallest small blind
    if(this->available_chips[0].denomination > this->blind_levels[1].little_blind)
    {
        throw td::protocol_error("smallest chip available is larger than the smallest little blind");
    }

    // counts for each denomination
    std::unordered_map<unsigned long,unsigned long> q;

    // step 1: fund using highest denominaton chips available
    auto remain(source.chips);
    auto cit(this->available_chips.rbegin());
    while(remain > 0)
    {
        // find highest denomination chip less than what remains
        while(cit != this->available_chips.rend() && cit->denomination > remain)
        {
            cit++;
        }

        if(cit == this->available_chips.rend())
        {
            throw td::protocol_error("buyin is not a multiple of the smallest chip available");
        }

        auto d(cit->denomination);

        // add chips and remove from remainder
        auto count(remain / d);
        while(count+q[d] > this->max_chips_for(d, max_expected))
        {
            count--;
        }
        q[d] += count;
        remain -= count*d;

        logger(ll::info) << "initial chips: T" << d << ": " << q[d] << '\n';
    }

    remain = 0;
    auto moved_a_chip(false);

    do
    {
        moved_a_chip = false;

        // step 2: loop through and shoot for stacks of at least 8
        const unsigned long target(8);
        for(auto cit1(this->available_chips.rbegin()), cit0(cit1+1); cit0 != this->available_chips.rend(); cit1++, cit0++)
        {
            auto d0(cit0->denomination);
            auto d1(cit1->denomination);

            // while no remainder, and lower denomination chips less than target and higher denomination chips remain
            while((remain == 0) && (q[d1] > 0) && (q[d0] < target))
            {
                // remove higher denomination chip and add to remiander
                q[d1]--;
                remain += d1;

                // add lower denomination chips and remove from remainder
                auto count(remain / d0);
                if(count+q[d0] > this->max_chips_for(d0, max_expected))
                {
                    // put it back if we exceed our available quantity
                    q[d1]++;
                    remain -= d1;
                    // and stop trying this denomination
                    break;
                }
                else
                {
                    q[d0] += count;
                    remain -= count*d0;
                    logger(ll::debug) << "move chips: T" << d1 << ": 1 -> T" << d0 << ": " << count << '\n';
                }

                logger(ll::debug) << "current chips: T" << d1 << ": " << q[d1] << '\n';
                logger(ll::debug) << "current chips: T" << d0 << ": " << q[d0] << '\n';
                logger(ll::debug) << "remaining value: T" << remain << '\n';
            }
        }

        // step 3: keep doing the above until no chips left to move
    }
    while(moved_a_chip);

    // check that we can represent buyin with our chip set
    if(remain != 0)
    {
        logger(ll::info) << "done moving, remaining value: T" << remain << '\n';
        return std::vector<td::player_chips>();
    }

    // convert to vector to return
    std::vector<td::player_chips> ret;
    for(auto& p : q)
    {
        if(p.second > 0)
        {
            ret.push_back(td::player_chips(p.first, p.second));
        }
    }

    return ret;
}

// re-calculate payouts
void gameinfo::recalculate_payouts()
{
    // some policies depend on the number of entries
    auto count_entries(this->entries.size());

    if(this->payout_policy == td::payout_policy_t::forced)
    {
        // force payout:
        // overrides everyting. disregard number of players
        if(this->forced_payouts.empty())
        {
            logger(ll::warning) << "payout_policy is forced but no forced_payouts exist. falling back to automatic payouts\n";
        }
        else
        {
            logger(ll::info) << "applying forced payout: " << this->forced_payouts.size() << " seats will be paid\n";

            // set state dirty
            this->dirty = true;

            // use the payout structure specified in forced_payouts
            this->payouts.resize(this->forced_payouts.size());
            std::transform(this->forced_payouts.begin(), this->forced_payouts.end(), this->payouts.begin(), [&](const td::monetary_value_nocurrency& c)
            {
                return td::monetary_value(c.amount, this->payout_currency);
            });
            return;
        }
    }
    else if(this->payout_policy == td::payout_policy_t::manual)
    {
        // manual payout:
        // look for a payout list given this number of unique entries
        auto manual_payout_it(this->manual_payouts.find(count_entries));
        if(manual_payout_it == this->manual_payouts.end())
        {
            logger(ll::warning) << "payout_policy is manual but no payout list with " << count_entries << " entries exists. falling back to automatic payouts\n";
        }
        else
        {
            logger(ll::info) << "applying manual payout for " << count_entries << " entries: " << manual_payout_it->second.size() << " seats will be paid\n";

            // set state dirty
            this->dirty = true;

            // use found payout structure
            this->payouts.resize(manual_payout_it->second.size());
            std::transform(manual_payout_it->second.begin(), manual_payout_it->second.end(), this->payouts.begin(), [&](const td::monetary_value_nocurrency& c)
            {
                return td::monetary_value(c.amount, this->payout_currency);
            });
            return;
        }
    }

    // automatic calculation, if no manual payout found:
    // first, calculate how many places pay, given configuration and number of entries
    std::size_t seats_paid(static_cast<std::size_t>(count_entries * this->automatic_payouts.percent_seats_paid + 0.5));
    if(seats_paid == 0)
    {
        seats_paid = 1;
    }

    // set state dirty
    this->dirty = true;

    // resize our payout structure
    this->payouts.resize(seats_paid);

    // should we round payouts?
    bool round(this->automatic_payouts.round_payouts);

    // payout shape. 0 = flat, 1 = winner takes all
    double shape(this->automatic_payouts.payout_shape);
    if(shape < 0.0)
    {
        logger(ll::warning) << "payout_shape must be >= 0. clamping to 0\n";
        shape = 0.0;
    }
    if(shape > 1.0)
    {
        logger(ll::warning) << "payout_shape must be <= 1. clamping to 1\n";
        shape = 1.0;
    }

    logger(ll::info) << "recalculating " << (round ? "" : "and rounding ") << "payouts for " << count_entries << " entries: " << this->automatic_payouts.percent_seats_paid * 100 << "% (" << seats_paid << " seats) will be paid. payout shape: " << shape << "\n";
    logger(ll::info) << "setting asiude " << this->automatic_payouts.pay_the_bubble << " for the bubble and " << this->automatic_payouts.pay_knockouts << " for each knockout\n";

    // exponent for harmonic series = shape/(shape-1), or 0 -> 0, 0.5 -> -1, 1 -> -inf
    double f;
    if(shape >= 1.0) {
        f = -INFINITY;
    } else {
        f = shape / (shape-1.0);
    }

    // ratio for each seat is comp[seat]:total
    std::vector<double> comp(seats_paid);
    double total(0.0);

    // generate proportional payouts based on harmonic series, place^f / sum(1 -> k)
    for(size_t n(0); n<seats_paid; n++)
    {
        double c(std::pow(n+1,f));
        comp[n] = c;
        total += c;
    }

    // total equity, minus set-asides for bubble and knockouts
    auto knockout_budget(this->automatic_payouts.pay_knockouts * (count_entries - 1));
    auto total_available(this->total_equity - this->automatic_payouts.pay_the_bubble - knockout_budget);

    // next, loop through again generating payouts
    if(round)
    {
        std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c)
        {
            double amount(std::round(total_available * c / total));
            return td::monetary_value(amount, this->payout_currency);
        });

        // count how much total was calculated after rounding
        auto total_allocated_payout(std::accumulate(this->payouts.begin(), this->payouts.end(), 0.0, [](int sum, const td::monetary_value& curr)
        {
            return sum + curr.amount;
        }));

        // remainder (either positive or negative) adjusts first place
        auto remainder(total_available - total_allocated_payout);
        this->payouts[0].amount += remainder;
    }
    else
    {
        std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c)
        {
            double amount(this->total_equity * c / total);
            return td::monetary_value(amount, this->payout_currency);
        });
    }

    // if we're paying the bubble, add it last
    if(this->automatic_payouts.pay_the_bubble > 0.0)
    {
        this->payouts.push_back(td::monetary_value(this->automatic_payouts.pay_the_bubble, this->payout_currency));
    }
}

// quickly set up a game (plan, seat, and buyin, using optional funding source)
std::vector<td::seated_player> gameinfo::quick_setup()
{
    if(this->funding_sources.empty())
    {
        throw td::protocol_error("cannot quick setup with no funding sources");
    }

    return this->quick_setup(this->source_for_type(td::funding_source_type_t::buyin));
}

std::vector<td::seated_player> gameinfo::quick_setup(const td::funding_source_id_t& src)
{
    // reset state
    this->reset_state();

    // plan for all players
    this->plan_seating(this->players.size());

    // seat and fund them
    std::vector<td::seated_player> seated_players;
    for(const auto& p : this->players)
    {
        auto seating(this->add_player(p.second.player_id));
        this->fund_player(p.second.player_id, src);

        // build a seated_player object
        td::seated_player sp(p.first, p.second.name, true, seating.second.table_number, seating.second.seat_number);
        seated_players.push_back(sp);
    }

    return seated_players;
}

// utility: start a blind level (optionally starting offset ms into the round)
void gameinfo::start_blind_level(std::size_t blind_level, duration_t offset)
{
    if(blind_level >= this->blind_levels.size())
    {
        throw td::protocol_error("not enough blind levels configured");
    }

    // set state dirty
    this->dirty = true;

    this->current_blind_level = blind_level;
    duration_t blind_level_duration(this->blind_levels[blind_level].duration);
    duration_t time_remaining(blind_level_duration - offset);
    duration_t break_time_remaining(this->blind_levels[blind_level].break_duration);
    this->end_of_round = this->now() + time_remaining;
    this->end_of_break = this->end_of_round + break_time_remaining;
}

// has the game started?
bool gameinfo::is_started() const
{
    return this->current_blind_level > 0;
}

// return current blind level
std::size_t gameinfo::get_current_blind_level() const
{
    return this->current_blind_level;
}

// start the game
void gameinfo::start()
{
    if(this->is_started())
    {
        throw td::protocol_error("tournament already started");
    }

    if(this->blind_levels.size() < 2)
    {
        throw td::protocol_error("cannot start without blind levels configured");
    }

    logger(ll::info) << "starting the tournament\n";

    // start the blind level
    this->start_blind_level(1, duration_t::zero());

    // set state dirty
    this->dirty = true;

    // set tournament start time
    this->tournament_start = this->now();
}

void gameinfo::start(const time_point_t& starttime)
{
    if(this->is_started())
    {
        throw td::protocol_error("tournament already started");
    }

    if(this->blind_levels.size() < 2)
    {
        throw td::protocol_error("cannot start without blind levels configured");
    }

    logger(ll::info) << "starting the tournament in the future:" << datetime(starttime) << '\n';

    // set state dirty
    this->dirty = true;

    // tournament is not started yet
    this->current_blind_level = 0;
    this->end_of_round = time_point_t();

    // set break end time to equal the tournament start time
    this->end_of_break = starttime;

    // set tournament start time
    this->tournament_start = starttime;
}

// stop the game
void gameinfo::stop()
{
    logger(ll::info) << "stopping the tournament\n";

    // set state dirty
    this->dirty = true;

    this->current_blind_level = 0;
    this->end_of_round = time_point_t();
    this->end_of_break = time_point_t();
    this->end_of_action_clock = time_point_t();
    this->tournament_start = time_point_t();
    this->paused_time = time_point_t();
}

// is paused
bool gameinfo::is_paused() const
{
    return this->paused_time != time_point_t();
}

// pause
void gameinfo::pause()
{
    if(!this->is_started())
    {
        throw td::protocol_error("tournament not started");
    }

    if(this->is_paused())
    {
        throw td::protocol_error("tournament already paused");
    }

    logger(ll::info) << "pausing the tournament\n";

    // set state dirty
    this->dirty = true;

    // save time the clock was paused
    this->paused_time = this->now();
}

// resume
void gameinfo::resume()
{
    if(!this->is_started())
    {
        throw td::protocol_error("tournament not started");
    }

    if(!this->is_paused())
    {
        throw td::protocol_error("tournament not paused");
    }

    logger(ll::info) << "resuming the tournament\n";

    // set state dirty
    this->dirty = true;

    // increment end_of_xxx based on time elapsed since we paused
    auto now(this->now());
    this->end_of_round += now - this->paused_time;
    this->end_of_break += now - this->paused_time;

    // mark unpaused
    this->paused_time = time_point_t();
}

// toggle pause/remove
void gameinfo::toggle_pause_resume()
{
    if(this->is_paused())
    {
        this->resume();
    }
    else
    {
        this->pause();
    }
}

// advance to next blind level
bool gameinfo::next_blind_level(duration_t offset)
{
    if(!this->is_started())
    {
        throw td::protocol_error("tournament not started");
    }

    if(this->current_blind_level + 1 < this->blind_levels.size())
    {
        logger(ll::info) << "setting next blind level from " << this->current_blind_level << " to " << this->current_blind_level + 1 << '\n';

        this->start_blind_level(this->current_blind_level + 1, offset);
        return true;
    }

    return false;
}

// return to prevous blind level
bool gameinfo::previous_blind_level(duration_t offset)
{
    if(!this->is_started())
    {
        throw td::protocol_error("tournament not started");
    }

    if(this->current_blind_level >= this->blind_levels.size())
    {
        throw td::protocol_error("current blind level out of bounds");
    }

    // calculate elapsed time in this blind level
    auto time_remaining(std::chrono::duration_cast<duration_t>(this->end_of_round - this->now()));
    auto blind_level_elapsed_time(this->blind_levels[this->current_blind_level].duration - time_remaining.count());

    // if elapsed time > 2 seconds, just restart current blind level
    if(blind_level_elapsed_time > this->previous_blind_level_hold_duration || this->current_blind_level == 1)
    {
        logger(ll::info) << "restarting blind level " << this->current_blind_level << '\n';

        this->start_blind_level(this->current_blind_level, offset);
        return false;
    }
    else
    {
        logger(ll::info) << "setting previous blind level from " << this->current_blind_level << " to " << this->current_blind_level - 1 << '\n';

        this->start_blind_level(this->current_blind_level - 1, offset);
        return true;
    }
}

// update game state
void gameinfo::update()
{
    // if not paused, and after end of break, increment blind level
    if(!this->is_paused() && this->end_of_break != time_point_t() && this->now() >= this->end_of_break)
    {
        // advance to next blind
        auto offset(std::chrono::duration_cast<duration_t>(this->end_of_break - this->now()));
        if(this->next_blind_level(offset) == false) {
            // if we're at the last blind level, stop the tournament
            this->stop();
        }
    }
}

// set the action clock (when someone 'needs the clock called on them'
void gameinfo::set_action_clock(long duration)
{
    if(this->end_of_action_clock == time_point_t())
    {
        // set state dirty
        this->dirty = true;

        this->end_of_action_clock = this->now() + duration_t(duration);
    }
    else
    {
        throw td::protocol_error("only one action clock at a time");
    }
}

// reset the action clock
void gameinfo::reset_action_clock()
{
    // set state dirty
    this->dirty = true;

    this->end_of_action_clock = time_point_t();
}

static unsigned long calculate_round_denomination(double ideal, const std::vector<td::chip>& chips)
{
    // round to denomination n if ideal small blind is at least 10x denomination n-1
    static const std::size_t multiplier(10);

    auto it(chips.rbegin());
    while(std::next(it) != chips.rend())
    {
        auto candidate(it->denomination);
        std::advance(it, 1);
        auto limit(it->denomination);

        logger(ll::debug) << "ideal: " << ideal << ", candidate: " << candidate << ", limitx10:" << limit * multiplier << '\n';

        if(ideal > limit * multiplier)
        {
            return candidate;
        }
    }

    return it->denomination;
}

// generate a number of progressive blind levels, given increase factor
// level_duration: uniform duraiton for each level
// chip_up_break_duration: if not zero, add a break whenever we can chip up
// blind_increase_factor: amount to multiply to increase blinds each round (1.5 is usually good here)
// this was tuned to produce a sensible result for the following chip denomination sets:
//  1/5/25/100/500
//  5/25/100/500/1000
//  25/100/500/1000/5000
std::vector<td::blind_level> gameinfo::gen_count_blind_levels(std::size_t count, long level_duration, long chip_up_break_duration, double blind_increase_factor, td::ante_type_t antes, double ante_sb_ratio) const
{
    if(this->available_chips.empty())
    {
        throw td::protocol_error("tried to create a blind structure without chips defined");
    }

    logger(ll::info) << "generating " << count << " blind levels" << (antes != td::ante_type_t::none ? " with antes\n" : "\n");
    logger(ll::info) << "blind_increase_factor: " << blind_increase_factor << '\n';
    logger(ll::info) << "ante_sb_ratio: " << ante_sb_ratio << '\n';

    // store last round denomination (to check when it changes)
    auto last_round_denom(this->available_chips.begin()->denomination);

    // starting small blind = smallest denomination
    auto ideal_small(static_cast<double>(last_round_denom));

    // output
    std::vector<td::blind_level> levels(count+1);

    for(size_t i(1); i<count+1; i++)
    {
        // calculate nearest chip denomination
        auto round_denom(calculate_round_denomination(ideal_small, this->available_chips));

        // round up to get little blind
        const auto little_blind(static_cast<unsigned long>(std::ceil(ideal_small / round_denom) * round_denom));

        logger(ll::debug) << "rounding ideal small blind " << ideal_small << " up to nearest " << round_denom << ": " << little_blind << '\n';

        // calculate antes if needed
        unsigned long ante(0);
        if(antes != td::ante_type_t::none)
        {
            // calculate ideal traditional ante
            auto ideal_ante(ante_sb_ratio * little_blind);

            // only have an ante if ideal >= minimal chip denomination
            if(ideal_ante >= this->available_chips[0].denomination)
            {
                if(antes == td::ante_type_t::traditional)
                {
                    // calculate nearest chip denomination
                    round_denom = calculate_round_denomination(ideal_ante, this->available_chips);

                    // round up to get ante
                    ante = static_cast<unsigned long>(std::ceil(ideal_ante / round_denom) * round_denom);

                    logger(ll::debug) << "rounding ideal ante " << ideal_ante << " up to nearest " << round_denom << ": " << ante << '\n';
                }
                else if(antes == td::ante_type_t::bba)
                {
                    // keep it simple. bba = big_blind
                    ante = little_blind * 2;

                    logger(ll::debug) << "ideal traditional ante " << ideal_ante << " and big blind ante is: " << ante << '\n';
                }
            }
        }

        levels[i].little_blind = little_blind;
        levels[i].big_blind = little_blind * 2;
        levels[i].ante = ante;
        levels[i].ante_type = antes;
        levels[i].duration = level_duration;

        // if round_denom changes, we no longer need a chip denomination
        logger(ll::debug) << "comparing round_denom " << round_denom << " with last_round_denom " << last_round_denom << '\n';

        if(i > 0 && round_denom != last_round_denom)
        {
            // break to chip up after each minimum denomination change
            levels[i].break_duration = chip_up_break_duration;
        }

        logger(ll::debug) << "round: " << i << ", will be: " << levels[i].little_blind << '/' << levels[i].big_blind << ':' << levels[i].ante << " with duration: " << levels[i].duration << " and break duration: " << levels[i].break_duration << '\n';

        // next small blind should be about factor times bigger than previous one
        ideal_small *= blind_increase_factor;

        // store last round denom (to determine when we need to chip up
        last_round_denom = round_denom;
    }

    return levels;
}

// generate progressive blind levels, given desired duration and starting stacks
// calculates number of rounds and increase factor and calls other generator
std::vector<td::blind_level> gameinfo::gen_blind_levels(long desired_duration, long level_duration, std::size_t expected_buyins, std::size_t expected_rebuys, std::size_t expected_addons, long chip_up_break_duration, td::ante_type_t antes, double ante_sb_ratio) const
{
    if(desired_duration <= 0)
    {
        throw td::protocol_error("tried to create a blind structure using invalid desired duration");
    }

    if(level_duration <= 0)
    {
        throw td::protocol_error("tried to create a blind structure using invalid level duration");
    }

    if(expected_buyins == 0)
    {
        throw td::protocol_error("tried to create a blind structure without specifying number of expected buyins");
    }

    if(this->available_chips.empty())
    {
        throw td::protocol_error("tried to create a blind structure without chips defined");
    }

    // assume chip up every 10 rounds
    const auto chip_up_rate(10);

    // assume tournament will end around when there are 10 BB left on the table
    const auto bb_at_end(10);

    // count estimated chips in play
    unsigned long chips_in_play(0);
    if(expected_buyins > 0)
    {
        const auto src(this->source_for_type(td::funding_source_type_t::buyin));
        chips_in_play += this->funding_sources[src].chips * expected_buyins;
    }

    if(expected_rebuys > 0)
    {
        const auto src(this->source_for_type(td::funding_source_type_t::rebuy));
        chips_in_play += this->funding_sources[src].chips * expected_rebuys;
    }

    if(expected_addons > 0)
    {
        const auto src(this->source_for_type(td::funding_source_type_t::addon));
        chips_in_play += this->funding_sources[src].chips * expected_addons;
    }

    if(chips_in_play == 0)
    {
        throw td::protocol_error("tried to create a blind structure, but no expected chips in play");
    }

    logger(ll::debug) << "total expected chips in play: " << chips_in_play << '\n';

    // estimate number of rounds in play = desired duration / average level duration including chip up breaks
    auto rounds_in_play(desired_duration / (level_duration + (chip_up_break_duration / chip_up_rate)));
    logger(ll::debug) << "total expected rounds: " << rounds_in_play << '\n';

    // calculate about 10% more rounds
    std::size_t count(rounds_in_play + rounds_in_play / 10 + 1);

    // first round small blind = smallest chip denomination
    auto first_round_sb(this->available_chips.begin()->denomination);

    // last round small blind
    auto last_round_sb(chips_in_play / (bb_at_end * 2));
    logger(ll::debug) << "first/last round small blind: " << first_round_sb << '/' << last_round_sb << '\n';

    // calculate increase factor that gets us from first round to last round sb
    // y = last sb
    // x = first sb
    // r = number of rounds
    // f = increase factor
    //
    // solve for f: y = x * f ^ (r-1)
    //
    // f = (y/x) ^ 1/(r-1)
    auto blind_increase_factor(std::pow(static_cast<double>(last_round_sb) / static_cast<double>(first_round_sb), 1.0/static_cast<double>(rounds_in_play-1)));

    // pass to other generator
    return this->gen_count_blind_levels(count, level_duration, chip_up_break_duration, blind_increase_factor, antes, ante_sb_ratio);
}

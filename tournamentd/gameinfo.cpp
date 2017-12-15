#include "gameinfo.hpp"
#include "logger.hpp"
#include "shared_instance.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>

// initialize game
gameinfo::gameinfo() :
    table_capacity(2),
    percent_seats_paid(1.0),
    round_payouts(false),
    payout_flatness(1.0),
    previous_blind_level_hold_duration(2000),
    tables(0),
    total_chips(0),
    total_cost(0.0),
    total_commission(0.0),
    total_equity(0.0),
    running(false),
    current_blind_level(0),
    time_remaining(duration_t::zero()),
    break_time_remaining(duration_t::zero()),
    action_clock_time_remaining(duration_t::zero()),
    elapsed_time(duration_t::zero())
{
    this->blind_levels.push_back(td::blind_level("Setup"));
}

void gameinfo::validate()
{
    logger(LOG_DEBUG) << "validating game configuration / state\n";

    // ensure we have at least one blind level, the setup level
    if(this->blind_levels.empty())
    {
        this->blind_levels.push_back(td::blind_level("Setup"));
    }
}

// load configuration from JSON (object or file)
void gameinfo::configure(const json& config)
{
    logger(LOG_INFO) << "loading tournament configuration\n";

    config.get_value("name", this->name);
    config.get_values("funding_sources", this->funding_sources);
    config.get_value("percent_seats_paid", this->percent_seats_paid);

    if(config.get_values("available_chips", this->available_chips))
    {
        // always sort chips by denomination
        std::sort(this->available_chips.begin(), this->available_chips.end(),
                  [](const td::chip& c0, const td::chip& c1)
                  {
                      return c0.denomination < c1.denomination;
                  });
    }

    // special handling for players, read into vector, then convert to map
    std::vector<td::player> players_vector;
    if(config.get_values("players", players_vector))
    {
        if(!this->seats.empty() || !this->players_finished.empty() || !this->buyins.empty() || !this->entries.empty())
        {
            logger(LOG_WARNING) << "re-coniguring players list while in play is not advised, deleted players may still be in the game\n";
        }

        this->players.clear();
        for(auto& player : players_vector)
        {
            this->players.emplace(player.player_id, player);
        }
    }

    // special handling for manual_payouts, read into vector, then convert to map
    std::vector<td::manual_payout> manual_payouts_vector;
    if(config.get_values("manual_payouts", manual_payouts_vector))
    {
        this->manual_payouts.clear();
        for(auto& manual_payout : manual_payouts_vector)
        {
            this->manual_payouts.emplace(manual_payout.buyins_count, manual_payout.payouts);
        }
    }

    if(config.get_value("table_capacity", this->table_capacity))
    {
        if(!this->seats.empty())
        {
            logger(LOG_WARNING) << "re-configuring table capacity will clear seating plan\n";
        }

        // if seats are already set up, replan based on new table capacity
        std::size_t total_seats = this->seats.size() + this->empty_seats.size();
        if(total_seats >= 2)
        {
            // re-plan seating, if needed
            this->plan_seating(total_seats);
        }
    }

    auto recalculate(false);
    recalculate = recalculate || config.get_value("manual_payouts");
    recalculate = recalculate || config.get_value("round_payouts", this->round_payouts);
    recalculate = recalculate || config.get_value("percent_seats_paid", this->percent_seats_paid);
    recalculate = recalculate || config.get_value("payout_flatness", this->payout_flatness);
    if(recalculate)
    {
        // after reconfiguring, we'll need to recalculate
        this->recalculate_payouts();
    }

    if(config.get_values("blind_levels", this->blind_levels))
    {
        if(this->is_started())
        {
            logger(LOG_WARNING) << "re-configuring blind levels while in play will stop the game\n";
            this->stop();
        }
    }

    validate();
}

// dump configuration to JSON
void gameinfo::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "dumping tournament configuration\n";

    config.set_value("name", this->name);
    config.set_value("players", json(this->players.begin(), this->players.end()));
    config.set_value("table_capacity", this->table_capacity);
    config.set_value("percent_seats_paid", this->percent_seats_paid);
    config.set_value("funding_sources", json(this->funding_sources.begin(), this->funding_sources.end()));
    config.set_value("blind_levels", json(this->blind_levels.begin(), this->blind_levels.end()));
    config.set_value("available_chips", json(this->available_chips.begin(), this->available_chips.end()));
    config.set_value("manual_payouts", json(this->manual_payouts.begin(), this->manual_payouts.end()));
}

// dump state to JSON
void gameinfo::dump_state(json& state) const
{
    logger(LOG_DEBUG) << "dumping tournament state\n";

    state.set_value("seats", json(this->seats.begin(), this->seats.end()));
    state.set_value("players_finished", json(this->players_finished.begin(), this->players_finished.end()));
    state.set_value("empty_seats", json(this->empty_seats.begin(), this->empty_seats.end()));
    state.set_value("tables", this->tables);
    state.set_value("buyins", json(this->buyins.begin(), this->buyins.end()));
    state.set_value("entries", json(this->entries.begin(), this->entries.end()));
    state.set_value("payouts", json(this->payouts.begin(), this->payouts.end()));
    state.set_value("total_chips", this->total_chips);
    state.set_value("total_cost", json(this->total_cost.begin(), this->total_cost.end()));
    state.set_value("total_commission", json(this->total_commission.begin(), this->total_commission.end()));
    state.set_value("total_equity", json(this->total_equity.begin(), this->total_equity.end()));
    state.set_value("running", this->running);
    state.set_value("current_blind_level", this->current_blind_level);
    state.set_value("time_remaining", this->time_remaining.count());
    state.set_value("break_time_remaining", this->break_time_remaining.count());
    state.set_value("action_clock_time_remaining", this->action_clock_time_remaining.count());
    state.set_value("elapsed_time", this->elapsed_time.count());
    this->dump_derived_state(state);
}

// duration to string
static std::ostream& operator<<(std::ostream& os, const std::chrono::milliseconds& milliseconds)
{
    auto duration(milliseconds.count());

    if(duration < /* DISABLES CODE */ (60000) && false) { // millisecond display turns out to be annoying
        // SS.MSS
        long long s = duration / 1000 % 60;
        long long ms = duration % 1000;
        os << s << '.' << std::setw(3) << std::setfill('0') << ms;
    } else if(duration < 3600000) {
        // MM:SS
        long long m = duration / 60000;
        long long s = duration / 1000 % 60;
        os << m << ':' << std::setw(2) << std::setfill('0') << s;
    } else {
        // HH:MM:SS
        long long h = duration / 3600000;
        long long m = duration / 60000 % 60;
        long long s = duration / 1000 % 60;
        os << h << ':' << std::setw(2) << std::setfill('0') << m << ':' << s;
    }

    return os;
}

// blind level to string
static std::ostream& operator<<(std::ostream& os, const td::blind_level& level)
{
    os << level.little_blind << '/' << level.big_blind;
    if(level.ante > 0)
    {
        os << " A:" << level.ante;
    }
    return os;
}

// funding source to string
static std::ostream& operator<<(std::ostream& os, const td::funding_source& src)
{
    os << src.name << ": " << src.cost_currency << src.cost;
    if(src.commission != 0.0)
    {
        if(src.commission_currency == src.cost_currency)
        {
            os << '+' << src.commission;
        }
        else
        {
            os << '+' << src.commission_currency << src.commission;
        }
    }
    return os;
}

// calculate derived state and dump to JSON
void gameinfo::dump_derived_state(json& state) const
{
    // has the tournament started?
    bool started(this->current_blind_level > 0 && this->current_blind_level < this->blind_levels.size());

    // are we on break?
    bool on_break(this->time_remaining == duration_t::zero() && this->break_time_remaining != duration_t::zero());
    state.set_value("on_break", on_break);

    std::ostringstream os;
    os.imbue(std::locale(""));

    // clock text
    auto clock_time(on_break ? this->break_time_remaining : this->time_remaining);
    if(this->running)
    {
        os << clock_time;
    }
    else
    {
        os << "PAUSED";
    }
    state.set_value("clock_text", os.str()); os.str("");

    // elapsed time text
    os << this->elapsed_time;
    state.set_value("elapsed_time_text", os.str()); os.str("");

    // current round number as text
    if(started)
    {
        os << this->current_blind_level;
    }
    else
    {
        os << '-';
    }
    state.set_value("current_round_number_text", os.str()); os.str("");

    // current game name text
    if(started && !on_break)
    {
        os << blind_levels[this->current_blind_level].game_name;
    }
    state.set_value("current_game_text", os.str()); os.str("");

    // current round text
    if(started)
    {
        if(on_break)
        {
            os << "BREAK";
        }
        else
        {
            os << this->blind_levels[this->current_blind_level];
        }
    }
    else
    {
        os << '-';
    }
    state.set_value("current_round_text", os.str()); os.str("");

    // next game name text
    if(started && this->current_blind_level+1 < this->blind_levels.size())
    {
        os << blind_levels[this->current_blind_level+1].game_name;
    }
    state.set_value("next_game_text", os.str()); os.str("");

    // next round text
    if(started && this->current_blind_level+1 < this->blind_levels.size())
    {
        if(on_break || this->blind_levels[this->current_blind_level].break_duration == 0)
        {
            os << this->blind_levels[this->current_blind_level+1];
        }
        else
        {
            os << "BREAK";
        }
    }
    else
    {
        os << '-';
    }
    state.set_value("next_round_text", os.str()); os.str("");

    // players left text
    if(!this->seats.empty())
    {
        os << this->seats.size();
    }
    else
    {
        os << '-';
    }
    state.set_value("players_left_text", os.str()); os.str("");

    // entries text
    if(!this->entries.empty())
    {
        os << this->entries.size();
    }
    else
    {
        os << '-';
    }
    state.set_value("entries_text", os.str()); os.str("");

    // average stack text
    if(!this->buyins.empty())
    {
        os << this->total_chips / this->buyins.size();
    }
    else
    {
        os << '-';
    }
    state.set_value("average_stack_text", os.str()); os.str("");

    // buyin text
    auto src_it(std::find_if(this->funding_sources.begin(), this->funding_sources.end(), [](const td::funding_source& s) { return s.type == td::buyin; }));
    if(src_it != this->funding_sources.end())
    {
        os << *src_it;
    }
    else
    {
        os << "NO BUYIN";
    }
    state.set_value("buyin_text", os.str()); os.str("");

    // payout currency (for now, it's the first equity_currency found. equity currencies are guaranteed to match)
    auto source(this->funding_sources.begin());

    // results
    std::vector<td::result> results;
    for(size_t j(0); j<this->seats.size(); j++)
    {
        td::result result(j+1);
        if(j < this->payouts.size() && source != this->funding_sources.end())
        {
            result.payout = this->payouts[j];
            result.payout_currency = source->equity_currency;
        }
        results.push_back(result);
    }
    for(size_t i(0); i<this->players_finished.size(); i++)
    {
        auto player_id(this->players_finished[i]);
        auto player_it(this->players.find(player_id));
        if(player_it == this->players.end())
        {
            throw std::runtime_error("failed to look up player: " + player_id);
        }
        size_t j(this->seats.size()+i);

        td::result result(j+1, player_it->second.name);
        if(j < this->payouts.size() && source != this->funding_sources.end())
        {
            result.payout = this->payouts[j];
            result.payout_currency = source->equity_currency;
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

void gameinfo::reset_seating()
{
    // clear all seating and remove all empty seats
    this->seats.clear();
    this->empty_seats.clear();

    // no tables means game is unplanned
    this->tables = 0;
}

const std::string gameinfo::player_description(const td::player_id_t& player_id) const
{
    auto player_it(this->players.find(player_id));
    if(player_it == this->players.end())
    {
        throw std::runtime_error("failed to look up player: " + player_id);
    }
    return player_id + " (" + player_it->second.name + ")";
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
        logger(LOG_DEBUG) << "table " << i << " has " << ret[i].size() << " players\n";
    }

    return ret;
}

std::size_t gameinfo::plan_seating(std::size_t max_expected_players)
{
    logger(LOG_INFO) << "planning tournament for " << max_expected_players << " players\n";

    // check arguments
    if(max_expected_players < 2)
    {
        throw td::protocol_error("expected players must be at least 2");
    }

    // check configuration: table capacity should be sane
    if(this->table_capacity < 2)
    {
        throw td::protocol_error("table capacity must be at least 2");
    }

    // reset funding, game state, and seating
    this->stop();
    this->reset_funding();
    this->reset_seating();

    // reset players finished
    this->players_finished.clear();

    // figure out how many tables needed
    this->tables = ((max_expected_players-1) / this->table_capacity) + 1;
    logger(LOG_INFO) << "tables needed: " << this->tables << "\n";

    // figure out how many seats should be first occupied
    std::size_t preferred_seats = (max_expected_players + this->tables - 1) / this->tables;
    logger(LOG_INFO) << "prefer: " << preferred_seats << " seats per table\n";

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

    // get randomization engine
    auto engine(get_shared_instance<std::default_random_engine>());

    // randomize preferred then extra seats separately
    std::shuffle(this->empty_seats.begin(), extra_it, *engine);
    std::shuffle(extra_it, this->empty_seats.end(), *engine);

    logger(LOG_INFO) << "created " << this->empty_seats.size() << " empty seats\n";

    // return number of tables needed
    return this->tables;
}

// add player to an existing game
td::seat gameinfo::add_player(const td::player_id_t& player_id)
{
    logger(LOG_INFO) << "adding player " << this->player_description(player_id) << " to game\n";

    // verify game state
    if(this->empty_seats.empty())
    {
        throw td::protocol_error("tried to add players with no empty seats");
    }

    auto seat_it(this->seats.find(player_id));

    if(seat_it != this->seats.end())
    {
        throw td::protocol_error("tried to add player already seated");
    }

    // seat player and remove from empty list
    auto seat(this->empty_seats.front());
    this->seats.insert(std::make_pair(player_id, seat));
    this->empty_seats.pop_front();

    logger(LOG_INFO) << "seated player " << this->player_description(player_id) << " at table " << seat.table_number << ", seat " << seat.seat_number << '\n';

    return seat;
}

// remove a player
td::seat gameinfo::remove_player(const td::player_id_t& player_id)
{
    logger(LOG_INFO) << "removing player " << this->player_description(player_id) << " from game\n";

    auto seat_it(this->seats.find(player_id));

    if(seat_it == this->seats.end())
    {
        throw td::protocol_error("tried to remove player not seated");
    }

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

    // remove the player
    this->remove_player(player_id);

    logger(LOG_INFO) << "busting player " << this->player_description(player_id) << " from the game\n";

    // add to the busted out list
    this->players_finished.push_front(player_id);

    // mark as no longer bought in
    this->buyins.erase(player_id);

    // try to break table or rebalance
    std::vector<td::player_movement> movements;
    this->try_break_table(movements);
    this->try_rebalance(movements);

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
        auto first_player_id(playing.front());

        // add to the busted out list
        this->players_finished.push_front(first_player_id);

        // remove the player
        this->remove_player(first_player_id);

        logger(LOG_INFO) << "winning player " << this->player_description(first_player_id) << '\n';

        // stop the game and reset all seating (stops additional entrants)
        this->stop();
        this->reset_seating();
    }

    return movements;
}

// move a player to a specific table
td::player_movement gameinfo::move_player(const td::player_id_t& player_id, std::size_t table)
{
    logger(LOG_INFO) << "moving player " << this->player_description(player_id) << " to table " << table << '\n';

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

    logger(LOG_INFO) << "choosing from " << candidates.size() << " free seats\n";

    // we should always have at least one seat free
    if(candidates.empty())
    {
        throw td::protocol_error("tried to move player to a full table");
    }

    // get randomization engine
    auto engine(get_shared_instance<std::default_random_engine>());

    // pick one at random
    auto index(std::uniform_int_distribution<std::size_t>(0, candidates.size()-1)(*engine));
    auto to_seat_it(candidates[index]);

    // move player
    auto player_seat_it(this->seats.find(player_id));
    auto from_seat(player_seat_it->second);
    player_seat_it->second = *to_seat_it;
    this->empty_seats.push_back(from_seat);
    this->empty_seats.erase(to_seat_it);

    logger(LOG_INFO) << "moved player " << this->player_description(player_id) << " from table " << from_seat.table_number << ", seat " << from_seat.seat_number << " to table " << player_seat_it->second.table_number << ", seat " << player_seat_it->second.seat_number << '\n';

    auto player_it(this->players.find(player_id));
    if(player_it == this->players.end())
    {
        throw std::runtime_error("failed to look up player: " + player_id);
    }

    return td::player_movement(player_id, player_it->second.name, from_seat, player_seat_it->second);
}

// move a player to the table with the smallest number of players
td::player_movement gameinfo::move_player(const td::player_id_t& player_id, const std::unordered_set<std::size_t>& avoid_tables)
{
    logger(LOG_INFO) << "moving player " << this->player_description(player_id) << " to a free table\n";

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

template <typename T>
static constexpr bool has_lower_size(const T& i0, const T& i1)
{
    return i0.size() < i1.size();
}

// re-balance by moving any player from a large table to a smaller one
// returns number of movements, or zero, if no players moved
std::size_t gameinfo::try_rebalance(std::vector<td::player_movement>& movements)
{
    logger(LOG_INFO) << "attemptying to rebalance tables\n";

    std::size_t ret(0);

    // get randomization engine
    auto engine(get_shared_instance<std::default_random_engine>());

    // build players-per-table vector
    auto ppt(this->players_at_tables());

    // find smallest and largest tables
    auto fewest_it(std::min_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id_t> >));
    auto most_it(std::max_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id_t> >));

    // if fewest has two fewer players than most (e.g. 6 vs 8), then rebalance
    while(!most_it->empty() && fewest_it->size() < most_it->size() - 1)
    {
        logger(LOG_INFO) << "largest table has " << most_it->size() << " players and smallest table has " << fewest_it->size() << " players\n";

        // pick a random player at the table with the most players
        auto index(std::uniform_int_distribution<std::size_t>(0, most_it->size()-1)(*engine));
        auto random_player((*most_it)[index]);

        // subtract iterator to find table number
        auto table(fewest_it - ppt.begin());
        movements.push_back(this->move_player(random_player, table));
        ret++;

        // update our lists to stay consistent
        (*most_it)[index] = most_it->back();
        most_it->pop_back();
        fewest_it->push_back(random_player);
    }

    return ret;
}

// break a table if possible
// returns number of movements, or zero, if no players moved
std::size_t gameinfo::try_break_table(std::vector<td::player_movement>& movements)
{
    logger(LOG_INFO) << "attemptying to break a table\n";

    std::size_t ret(0);

    if(this->tables > 1)
    {
        // break tables while (player_count-1) div table_capacity < tables
        while(this->seats.size() <= this->table_capacity * (this->tables-1))
        {
            logger(LOG_INFO) << "breaking a table. " << this->seats.size() << " players remain at " << this->tables << " tables of " << this->table_capacity << " capacity\n";

            // always break the highest-numbered table
            auto break_table(this->tables - 1);

            // get each player found to be sitting at table
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
                ret++;
            }

            // decrement number of tables
            this->tables--;

            // prune empty table from our open seat list, no need to seat people at unused tables
            this->empty_seats.erase(std::remove_if(this->empty_seats.begin(), this->empty_seats.end(), [&break_table](const td::seat& seat) { return seat.table_number == break_table; }));

            logger(LOG_INFO) << "broken table " << break_table << ". " << this->seats.size() << " players now at " << this->tables << " tables\n";
        }
    }

    return ret;
}

// fund a player, (re-)buyin or addon
void gameinfo::fund_player(const td::player_id_t& player_id, const td::funding_source_id_t& src)
{
    if(src >= this->funding_sources.size())
    {
        throw td::protocol_error("invalid funding source");
    }

    const td::funding_source& source(this->funding_sources[src]);

    if(this->current_blind_level > source.forbid_after_blind_level)
    {
        throw td::protocol_error("too late in the game for this funding source");
    }

    if(source.type != td::buyin && std::find(this->entries.begin(), this->entries.end(), player_id) == this->entries.end())
    {
        throw td::protocol_error("tried a non-buyin funding source but not bought in yet");
    }

    if(source.type == td::rebuy && this->current_blind_level < 1)
    {
        throw td::protocol_error("tried re-buying before tournamnet start");
    }

    if(!this->total_equity.empty() && source.equity_currency != this->total_equity.begin()->first)
    {
        throw td::protocol_error("tried mixing equity currencies, currently not supported");
    }

    logger(LOG_INFO) << "funding player " << this->player_description(player_id) << " with " << source.name << '\n';

    if(source.type == td::buyin)
    {
        // add player to buyin set
        this->buyins.insert(player_id);

        // add to entries
        this->entries.push_back(player_id);
    }
    else if(source.type == td::rebuy)
    {
        // add player to buyin set
        this->buyins.insert(player_id);
    }

    // update totals
    this->total_chips += source.chips;
    this->total_cost[source.cost_currency] += source.cost;
    this->total_commission[source.commission_currency] += source.commission;
    this->total_equity[source.equity_currency] += source.equity;

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

// calculate number of chips per denomination for this funding source, given totals and number of players
// this was tuned to produce a sensible result for the following chip denomination sets:
//  1/5/25/100/500
//  5/25/100/500/1000
//  25/100/500/1000/5000
std::vector<td::player_chips> gameinfo::chips_for_buyin(const td::funding_source_id_t& src, std::size_t max_expected_players) const
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
        while(count+q[d] > this->max_chips_for(d, max_expected_players))
        {
            count--;
        }
        q[d] += count;
        remain -= count*d;

        logger(LOG_INFO) << "initial chips: T" << d << ": " << q[d] << '\n';
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
                if(count+q[d0] > this->max_chips_for(d0, max_expected_players))
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
                    logger(LOG_DEBUG) << "move chips: T" << d1 << ": 1 -> T" << d0 << ": " << count << '\n';
                }

                logger(LOG_DEBUG) << "current chips: T" << d1 << ": " << q[d1] << '\n';
                logger(LOG_DEBUG) << "current chips: T" << d0 << ": " << q[d0] << '\n';
                logger(LOG_DEBUG) << "remaining value: T" << remain << '\n';
            }
        }

        // step 3: keep doing the above until no chips left to move
    }
    while(moved_a_chip);

    // check that we can represent buyin with our chip set
    if(remain != 0)
    {
        logger(LOG_INFO) << "done moving, remaining value: T" << remain << '\n';
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
    // manual payout:
    // look for a manual payout given this number of entries (TODO: should this use number of players or buyins instead?)
    auto manual_payout_it(this->manual_payouts.find(this->entries.size()));
    if(manual_payout_it != this->manual_payouts.end())
    {
        logger(LOG_INFO) << "applying manual payout: " << manual_payout_it->second.size() << " seats will be paid\n";

        // use found payout structure
        this->payouts = manual_payout_it->second;
        return;
    }

    // automatic calculation, if no manual payout found:
    // first, calculate how many places pay, given configuration and number of players bought in
    std::size_t seats_paid(static_cast<std::size_t>(this->entries.size() * this->percent_seats_paid + 0.5));
    bool round(this->round_payouts);
    double f(this->payout_flatness);

    logger(LOG_INFO) << "recalculating payouts: " << seats_paid << " seats will be paid\n";

    // resize our payout structure
    this->payouts.resize(seats_paid);

    if(!this->payouts.empty())
    {
        // ratio for each seat is comp[seat]:total
        std::vector<double> comp(seats_paid);
        double total(0.0);

        // generate proportional payouts based on harmonic series, N^-f / sum(1/k)
        for(size_t n(0); n<seats_paid; n++)
        {
            double c(std::pow(n+1,-f));
            comp[n] = c;
            total += c;
        }

        // next, loop through again generating payouts
        if(round)
        {
            std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c) { return std::round(this->total_equity.begin()->second * c / total); });

            // remainder (either positive or negative) adjusts first place
            auto remainder(this->total_equity.begin()->second - std::accumulate(this->payouts.begin(), this->payouts.end(), 0.0));
            this->payouts[0] += remainder;
        }
        else
        {
            std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c) { return this->total_equity.begin()->second * c / total; });
        }
    }
}

void gameinfo::reset_funding()
{
    logger(LOG_INFO) << "resetting tournament funding to zero\n";

    // cannot plan seating while game is in progress
    if(this->current_blind_level > 0)
    {
        throw td::protocol_error("cannot reset funding while tournament is running");
    }

    this->buyins.clear();
    this->entries.clear();
    this->payouts.clear();
    this->total_chips = 0;
    this->total_cost.clear();
    this->total_commission.clear();
    this->total_equity.clear();
}

// utility: start a blind level (optionally starting offset ms into the round)
void gameinfo::start_blind_level(std::size_t blind_level, duration_t offset)
{
    if(blind_level >= this->blind_levels.size())
    {
        throw td::protocol_error("not enough blind levels configured");
    }

    auto now(std::chrono::system_clock::now());

    this->current_blind_level = blind_level;
    this->time_remaining = duration_t(this->blind_levels[this->current_blind_level].duration) - offset;
    this->break_time_remaining = duration_t(this->blind_levels[this->current_blind_level].break_duration);
    this->end_of_round = now + this->time_remaining;
    this->end_of_break = this->end_of_round + this->break_time_remaining;
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

    auto starttime(std::chrono::system_clock::now());

    logger(LOG_INFO) << "starting the tournament\n";

    // start the blind level
    this->start_blind_level(1, duration_t::zero());

    // start the tournament
    this->running = true;

    // set tournament start time
    this->tournament_start = starttime;

    // set elapsed time
    this->elapsed_time = duration_t::zero();
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

    logger(LOG_INFO) << "starting the tournament in the future:" << datetime(starttime) << '\n';

    // start the tournament
    this->running = true;

    // set break end time
    this->end_of_break = starttime;

    // set tournament start time
    this->tournament_start = starttime;

    // set elapsed time
    this->elapsed_time = duration_t::zero();
}

// stop the game
void gameinfo::stop()
{
    logger(LOG_INFO) << "stopping the tournament\n";

    this->running = false;
    this->current_blind_level = 0;
    this->end_of_round = time_point_t();
    this->end_of_break = time_point_t();
    this->end_of_action_clock = time_point_t();
    this->time_remaining = duration_t::zero();
    this->break_time_remaining = duration_t::zero();
    this->action_clock_time_remaining = duration_t::zero();
    this->tournament_start = time_point_t();
    this->elapsed_time = duration_t::zero();
}

// pause
void gameinfo::pause()
{
    if(!this->is_started())
    {
        throw td::protocol_error("tournament not started");
    }

    logger(LOG_INFO) << "pausing the tournament\n";

    // update time remaining
    this->update_remaining();

    // pause
    this->running = false;
}

// resume
void gameinfo::resume()
{
    if(!this->is_started())
    {
        throw td::protocol_error("tournament not started");
    }

    logger(LOG_INFO) << "resuming the tournament\n";

    auto now(std::chrono::system_clock::now());

    // update end_of_xxx with values saved when we paused
    this->end_of_round = now + this->time_remaining;
    this->end_of_break = this->end_of_round + this->break_time_remaining;

    // resume
    this->running = true;
}

// toggle pause/remove
void gameinfo::toggle_pause_resume()
{
    if(this->running)
    {
        this->pause();
    }
    else
    {
        this->resume();
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
        logger(LOG_INFO) << "setting next blind level from " << this->current_blind_level << " to " << this->current_blind_level + 1 << '\n';

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

    // calculate elapsed time in this blind level
    auto blind_level_elapsed_time(this->blind_levels[this->current_blind_level].duration - this->time_remaining.count());

    // if elapsed time > 2 seconds, just restart current blind level
    if(blind_level_elapsed_time > this->previous_blind_level_hold_duration || this->current_blind_level == 1)
    {
        logger(LOG_INFO) << "restarting blind level " << this->current_blind_level << '\n';

        this->start_blind_level(this->current_blind_level, offset);
        return false;
    }
    else
    {
        logger(LOG_INFO) << "setting previous blind level from " << this->current_blind_level << " to " << this->current_blind_level - 1 << '\n';

        this->start_blind_level(this->current_blind_level - 1, offset);
        return true;
    }
}

// update time remaining
bool gameinfo::update_remaining()
{
    auto updated(false);
    auto now(std::chrono::system_clock::now());

    // update elapsed_time if we are past the tournament start
    if(this->tournament_start != time_point_t() && this->tournament_start < now)
    {
        this->elapsed_time = std::chrono::duration_cast<duration_t>(now - this->tournament_start);
        updated = true;
    }

    // always update action clock if ticking
    if(this->end_of_action_clock != time_point_t() && this->end_of_action_clock > now)
    {
        this->action_clock_time_remaining = std::chrono::duration_cast<duration_t>(this->end_of_action_clock - now);
        updated = true;
    }
    else
    {
        this->end_of_action_clock = time_point_t();
        this->action_clock_time_remaining = duration_t::zero();
    }

    if(this->running)
    {
        // set time remaining based on current clock
        if(now < this->end_of_round)
        {
            // within round, set time remaining
            this->time_remaining = std::chrono::duration_cast<duration_t>(this->end_of_round - now);
        }
        else if(now < this->end_of_break)
        {
            // within break, set time remaining to zero and set break time remaining
            this->time_remaining = duration_t::zero();
            this->break_time_remaining = std::chrono::duration_cast<duration_t>(this->end_of_break - now);
        }
        else
        {
            // advance to next blind
            auto offset(std::chrono::duration_cast<duration_t>(this->end_of_break - now));
            this->next_blind_level(offset);
        }
        updated = true;
    }

    return updated;
}

// set the action clock (when someone 'needs the clock called on them'
void gameinfo::set_action_clock(long duration)
{
    if(this->end_of_action_clock == time_point_t())
    {
        this->end_of_action_clock = std::chrono::system_clock::now() + duration_t(duration);
        this->action_clock_time_remaining = duration_t(duration);
    }
    else
    {
        throw td::protocol_error("only one action clock at a time");
    }
}

// reset the action clock
void gameinfo::reset_action_clock()
{
    this->end_of_action_clock = time_point_t();
    this->action_clock_time_remaining = duration_t::zero();
}

static unsigned long calculate_round_denomination(double ideal_small, const std::vector<td::chip>& chips)
{
    // round to denomination n if ideal small blind is at least 10x denomination n-1
    static const std::size_t multiplier(10);

    auto it(chips.rbegin());
    while(std::next(it) != chips.rend())
    {
        auto candidate(it->denomination);
        std::advance(it, 1);
        auto limit(it->denomination);

        logger(LOG_DEBUG) << "ideal_small: " << ideal_small << ", candidate: " << candidate << ", limitx10:" << limit * multiplier << '\n';

        if(ideal_small > limit * multiplier)
        {
            return candidate;
        }
    }

    return it->denomination;
}

// generate progressive blind levels, given available chip denominations
// level_duration: uniform duraiton for each level
// chip_up_break_duration: if not zero, add a break whenever we can chip up
// blind_increase_factor: amount to multiply to increase blinds each round (1.5 is usually good here)
// this was tuned to produce a sensible result for the following chip denomination sets:
//  1/5/25/100/500
//  5/25/100/500/1000
//  25/100/500/1000/5000
void gameinfo::gen_blind_levels(std::size_t count, long level_duration, long chip_up_break_duration, double blind_increase_factor)
{
    if(this->available_chips.empty())
    {
        throw td::protocol_error("tried to create a blind structure without chips defined");
    }

    logger(LOG_INFO) << "generating " << count << " blind levels\n";

    // resize structure, zero-initializing
    this->blind_levels.resize(count+1);

    // store last round denomination (to check when it changes)
    auto last_round_denom(this->available_chips.begin()->denomination);

    // starting small blind = smallest denomination
    double ideal_small(static_cast<double>(last_round_denom));

    for(size_t i(1); i<count+1; i++)
    {
        // calculate nearest chip denomination to round to
        const auto round_denom(calculate_round_denomination(ideal_small, this->available_chips));
        const auto little_blind(static_cast<unsigned long>(std::ceil(ideal_small / round_denom) * round_denom));

        logger(LOG_DEBUG) << "round: " << i << ", little blind will be: " << little_blind << '\n';

        // round up
        this->blind_levels[i].little_blind = little_blind;
        this->blind_levels[i].big_blind = this->blind_levels[i].little_blind * 2;
        this->blind_levels[i].ante = 0;
        this->blind_levels[i].duration = level_duration;
        if(i > 0 && round_denom != last_round_denom)
        {
            // 5 minute break to chip up after each minimum denomination change
            this->blind_levels[i].break_duration = chip_up_break_duration;
        }

        // next small blind should be about factor times bigger than previous one
        ideal_small *= blind_increase_factor;
    }
}

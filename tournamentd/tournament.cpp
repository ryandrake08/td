#include "tournament.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <iterator>

// ----- constants -----

const std::size_t tournament::funding_source::always_allow = std::numeric_limits<std::size_t>::max();

// ----- exception -----

tournament::exception::exception(const std::string& what_arg) : std::runtime_error(what_arg)
{
}

tournament::exception::exception(const char* what_arg) : std::runtime_error(what_arg)
{
}

// ----- configuration loading -----

template <typename T>
static void load(const json& config, const char* name, T& value)
{
    json child;
    if(config.get_value(name, child))
    {
        load(child, value);
    }
}

template <typename T>
static void load(const json& config, const char* name, std::vector<T>& value)
{
    std::vector<json> array;
    if(config.get_value(name, array))
    {
        value.resize(array.size());
        for(auto i(0); i<array.size(); i++)
        {
            load(array[i], value[i]);
        }
    }
}

static void load(const json& config, tournament::funding_source& value)
{
    logger(LOG_DEBUG) << "Loading tournament funding source\n";

    config.get_value("is_addon", value.is_addon);
    config.get_value("forbid_after_blind_level", value.forbid_after_blind_level);
    config.get_value("chips", value.chips);
    config.get_value("cost", value.cost);
    config.get_value("commission", value.commission);
    config.get_value("equity", value.equity);
}

static void load(const json& config, tournament::player& value)
{
    logger(LOG_DEBUG) << "Loading tournament player info\n";

    config.get_value("name", value.name);
}

static void load(const json& config, tournament::chip& value)
{
    logger(LOG_DEBUG) << "Loading tournament chip info\n";

    config.get_value("color", value.color);
    config.get_value("denomination", value.denomination);
    config.get_value("count_available", value.count_available);
}

static void load(const json& config, tournament::blind_level& value)
{
    logger(LOG_DEBUG) << "Loading tournament blind level\n";

    config.get_value("little_blind", value.little_blind);
    config.get_value("big_blind", value.big_blind);
    config.get_value("ante", value.ante);
    config.get_value("duration_ms", value.duration_ms);
    config.get_value("break_duration_ms", value.break_duration_ms);
}

static void load(const json& config, tournament::payout& value)
{
    logger(LOG_DEBUG) << "Loading tournament payout\n";

    config.get_value("derive_award", value.derive_award);
    config.get_value("percent_x100", value.percent_x100);
    config.get_value("award", value.award);
}

static void load(const json& config, tournament::configuration& value)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    config.get_value("name", value.name);
    config.get_value("cost_currency", value.cost_currency);
    config.get_value("equity_currency", value.equity_currency);
    config.get_value("table_capacity", value.table_capacity);
    load(config, "funding_sources", value.funding_sources);
    load(config, "players", value.players);
    load(config, "chips", value.chips);
    load(config, "blind_levels", value.blind_levels);
    load(config, "payouts", value.payouts);
}

void tournament::log(activity::event_t ev, player_id player)
{
    logger(LOG_DEBUG) << "Logging event " << ev << " for player " << player << '\n';

    this->activity_log.push_back(activity({ev, player, std::chrono::system_clock::now()}));
}

// ----- configuration -----

// load configuration from JSON (object or file)
void tournament::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    load(config, this->cfg);

    if(this->cfg.table_capacity < 2)
    {
        throw exception("table capacity must be at least 2");
    }

    if(this->cfg.blind_levels.empty())
    {
        throw exception("need at least the zero blind level (for planning)");
    }
}

// reset game back to planning state
void tournament::reset()
{
    logger(LOG_DEBUG) << "Resetting tournament state back to planning\n";

    this->activity_log.clear();
    this->running = false;
    this->current_blind_level = 0;
    this->end_of_round = std::chrono::system_clock::time_point();
    this->end_of_break = std::chrono::system_clock::time_point();
    this->time_remaining_ms = ms(0);
    this->break_time_remaining_ms = ms(0);

    this->seats.clear();
    this->players_finished.clear();
    this->empty_seats.clear();
    this->tables = 0;

    this->buyins.clear();
    this->total_chips = 0;
    this->total_cost = 0;
    this->total_commission = 0;
    this->total_equity = 0;
}

// ----- clock -----

// utility: start a blind level
void tournament::start_blind_level(std::size_t blind_level)
{
    if(blind_level >= this->cfg.blind_levels.size())
    {
        throw exception("not enough blind levels configured");
    }

    auto now(std::chrono::system_clock::now());

    this->current_blind_level = blind_level;
    this->time_remaining_ms = ms(this->cfg.blind_levels[this->current_blind_level].duration_ms);
    this->break_time_remaining_ms = ms(this->cfg.blind_levels[this->current_blind_level].break_duration_ms);
    this->end_of_round = now + this->time_remaining_ms;
    this->end_of_break = this->end_of_round + this->break_time_remaining_ms;
}

// has the game started?
bool tournament::is_started() const
{
    return this->current_blind_level > 0;
}

// start the game
void tournament::start()
{
    logger(LOG_DEBUG) << "Starting the tournament\n";

    // start the tournament
    this->running = true;

    // start the blind level
    this->start_blind_level(1);
}

// stop the game
void tournament::stop()
{
    logger(LOG_DEBUG) << "Stopping the tournament\n";

    // stop the tournament
    this->running = false;

    // set dummy blind level
    this->current_blind_level = this->cfg.blind_levels.size();
}

// toggle pause
void tournament::pause()
{
    logger(LOG_DEBUG) << "Pausing the tournament\n";

    auto now(std::chrono::system_clock::now());

    if(this->running)
    {
        // set time remaining based on current clock
        if(now < this->end_of_round)
        {
            // within round, set time remaining to fractional round and break time remaining to full
            this->time_remaining_ms = std::chrono::duration_cast<ms>(this->end_of_round - now);
            this->break_time_remaining_ms = ms(this->cfg.blind_levels[this->current_blind_level].break_duration_ms);
        }
        else if(now < this->end_of_break)
        {
            // within break, set time remaining to fractional break
            this->time_remaining_ms = ms::zero();
            this->break_time_remaining_ms = std::chrono::duration_cast<ms>(this->end_of_break - now);
        }
        else
        {
            throw exception("inconsistent internal state, current time is past current blind level");
        }

        // pause
        this->running = false;
    }
}

// toggle resume
void tournament::resume()
{
    logger(LOG_DEBUG) << "Resuming the tournament\n";

    auto now(std::chrono::system_clock::now());

    if(!this->running)
    {
        this->end_of_round = now + this->time_remaining_ms;
        this->end_of_break = this->end_of_round + this->break_time_remaining_ms;

        // resume
        this->running = true;
    }
}

// advance to next blind level
void tournament::advance_blind_level()
{
    logger(LOG_DEBUG) << "Advancing blind level from " << this->current_blind_level << " to " << this->current_blind_level + 1 << '\n';

    if(this->current_blind_level == 0)
    {
        throw exception("game not running");
    }

    if(this->current_blind_level + 1 < this->cfg.blind_levels.size())
    {
        this->start_blind_level(this->current_blind_level + 1);
    }
}

// restart current blind level
void tournament::restart_blind_level()
{
    logger(LOG_DEBUG) << "Restarting blind level " << this->current_blind_level << '\n';

    if(this->current_blind_level == 0)
    {
        throw exception("game not running");
    }

    this->start_blind_level(this->current_blind_level);
}

// return to prevous blind level
void tournament::previous_blind_level()
{
    logger(LOG_DEBUG) << "Returning blind level from " << this->current_blind_level << " to " << this->current_blind_level - 1 << '\n';

    if(this->current_blind_level == 0)
    {
        throw exception("game not running");
    }

    if(this->current_blind_level > 1)
    {
        this->start_blind_level(this->current_blind_level - 1);
    }
}

// ----- seating -----

std::vector<std::vector<tournament::player_id>> tournament::players_at_tables() const
{
    // build up two vectors, outer = tables, inner = players per table
    std::vector<std::vector<player_id>> ret(this->tables);

    for(auto seat : this->seats)
    {
        ret[seat.second.table_number].push_back(seat.first);
    }

    for(auto i(0); i<ret.size(); i++)
    {
        logger(LOG_DEBUG) << "Table " << i << " has " << ret[i].size() << " players\n";
    }

    return ret;
}

std::size_t tournament::plan_seating(std::size_t max_expected_players)
{
    logger(LOG_DEBUG) << "Planning tournament for " << max_expected_players << " players\n";

    // table capacity should be sane
    if(this->cfg.table_capacity < 2)
    {
        throw exception("table capacity must be at least 2");
    }

    // model pre-conditions
    assert(!this->is_started());
    assert(this->seats.empty());
    assert(this->players_finished.empty());
    assert(this->empty_seats.empty());

    // check arguments
    if(max_expected_players < 2)
    {
        throw exception("max_expected_players must be at least 2");
    }

    // restore to known quantities
    this->seats.clear();
    this->players_finished.clear();
    this->empty_seats.clear();
    
    // figure out how many tables needed
    this->tables = ((max_expected_players-1) / this->cfg.table_capacity) + 1;

    logger(LOG_DEBUG) << "Tables needed: " << this->tables << "\n";

    // build up seat list
    for(std::size_t t=0; t<this->tables; t++)
    {
        for(std::size_t s=0; s<this->cfg.table_capacity; s++)
        {
            this->empty_seats.push_back(seat({t,s}));
        }
    }
    std::random_shuffle(this->empty_seats.begin(), this->empty_seats.end());

    logger(LOG_DEBUG) << "Created " << this->seats.size() << " empty seats\n";

    // Return number of tables needed
    return this->tables;
}

// add player to an existing game
tournament::seat tournament::add_player(const player_id& player)
{
    logger(LOG_DEBUG) << "Adding player " << player << " to game\n";

    // verify game state
    if(this->empty_seats.empty())
    {
        throw exception("no empty seats");
    }

    if(player < this->cfg.players.size())
    {
        throw exception("unknown player id");
    }

    auto seat_it(this->seats.find(player));

    if(seat_it != this->seats.end())
    {
        throw exception("player already seated");
    }

    // seat player and remove from empty list
    auto seat(this->empty_seats.front());
    this->seats[player] = seat;
    this->empty_seats.pop_front();

    logger(LOG_DEBUG) << "Seated player at table " << seat.table_number << ", seat " << seat.seat_number << '\n';

    // log it
    log(activity::did_seat, player);

    return seat;
}

// remove a player
std::size_t tournament::remove_player(const player_id& player)
{
    logger(LOG_DEBUG) << "Removing player " << player << " from game\n";

    if(player < this->cfg.players.size())
    {
        throw exception("unknown player id");
    }

    auto seat_it(this->seats.find(player));

    if(seat_it == this->seats.end())
    {
        throw exception("player not seated");
    }

    // bust player and add seat to the end of the empty list
    this->seats.erase(player);
    this->players_finished.push_front(player);
    this->empty_seats.push_back(seat_it->second);

    // log it
    log(activity::did_bust, player);

    return this->players_finished.size();
}

// move a player to a specific table
tournament::player_movement tournament::move_player(const player_id& player, std::size_t table)
{
    logger(LOG_DEBUG) << "Moving player " << player << " to table " << table << '\n';

    if(player < this->cfg.players.size())
    {
        throw exception("unknown player id");
    }

    // build up a list of candidate seats
    std::vector<std::deque<seat>::iterator> candidates;
    for(auto it(this->empty_seats.begin()); it != this->empty_seats.end(); it++)
    {
        if(it->table_number == table)
        {
            // store iterator itself
            candidates.push_back(it);
        }
    }

    logger(LOG_DEBUG) << "Choosing from " << candidates.size() << " free seats\n";

    // we should always have at least one seat free
    if(candidates.empty())
    {
        throw exception("tried to move player to a full table");
    }

    // pick one at random
    auto index(std::rand() % candidates.size());
    auto to_seat(*candidates[index]);

    // move player
    auto player_seat_it(this->seats.find(player));
    auto from_seat(player_seat_it->second);
    this->empty_seats.push_back(from_seat);
    player_seat_it->second = to_seat;

    logger(LOG_DEBUG) << "Moved player " << player << " from table " << from_seat.table_number << ", seat " << from_seat.seat_number << " to table " << to_seat.table_number << ", seat " << to_seat.seat_number << '\n';

    // log it
    log(activity::did_change_seat, player);

    return { player, from_seat, to_seat };
}

// move a player to the table with the smallest number of players
tournament::player_movement tournament::move_player(const player_id& player, const std::unordered_set<std::size_t>& avoid_tables)
{
    logger(LOG_DEBUG) << "Moving player " << player << " to a free table\n";

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
    }

    // make sure at least one candidate table
    if(table >= ppt.size())
    {
        throw exception("all candidate tables in avoid_table set");
    }

    return this->move_player(player, table);
}

template <typename T>
static bool has_lower_size(const T& i0, const T& i1)
{
    return i0.size() < i1.size();
}

// re-balance by moving any player from a large table to a smaller one, returns true if player was moved
std::vector<tournament::player_movement> tournament::try_rebalance()
{
    logger(LOG_DEBUG) << "Attemptying to rebalance tables\n";

    // build players-per-table vector
    auto ppt(this->players_at_tables());

    // find smallest and largest tables
    auto fewest_it(std::min_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<tournament::player_id>>));
    auto most_it(std::max_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<tournament::player_id>>));

    // return all player movements
    std::vector<tournament::player_movement> ret;

    // if fewest has two fewer players than most (e.g. 6 vs 8), then rebalance
    while(fewest_it->size() < most_it->size() - 1)
    {
        logger(LOG_DEBUG) << "Largest table has " << most_it->size() << " players and smallest table has " << fewest_it->size() << " players\n";
        
        // pick a random player at the table with the most players
        auto index(std::rand() % most_it->size());
        auto random_player((*most_it)[index]);

        // subtract iterator to find table number
        auto table(fewest_it - ppt.begin());
        ret.push_back(this->move_player(random_player, table));

        // update our lists to stay consistent
        (*most_it)[index] = std::move(most_it->back());
        most_it->pop_back();
        fewest_it->push_back(random_player);
    }

    return ret;
}

std::vector<tournament::player_movement> tournament::try_break_table()
{
    logger(LOG_DEBUG) << "Attemptying to break a table\n";

    // return all player movements
    std::vector<tournament::player_movement> ret;

    if(this->tables > 1)
    {
        // break tables while (player_count-1) div table_capacity < tables
        while((this->seats.size() - 1) / this->cfg.table_capacity < this->tables)
        {
            logger(LOG_DEBUG) << "Breaking a table. " << this->seats.size() << " players remain at " << this->tables << " tables of " << this->cfg.table_capacity << " capacity\n";

            // always break the highest-numbered table
            auto break_table(this->tables - 1);

            // get each player found to be sitting at table
            std::vector<player_id> to_move;
            for(auto seat : this->seats)
            {
                if(seat.second.table_number == break_table)
                {
                    to_move.push_back(seat.first);
                }
            }

            // move each player in list
            const std::unordered_set<std::size_t> avoid({break_table});
            for(auto player : to_move)
            {
                ret.push_back(this->move_player(player, avoid));
            }

            // decrement number of tables
            this->tables--;

            // prune empty table from our open seat list, no need to seat people at unused tables
            this->empty_seats.erase(std::remove_if(this->empty_seats.begin(), this->empty_seats.end(), [&break_table](const seat& seat) { return seat.table_number == break_table; }));

            logger(LOG_DEBUG) << "Broken table " << break_table << ". " << this->seats.size() << " players now at " << this->tables << " tables\n";
        }
    }

    return ret;
}

// ----- chips -----

static bool operator==(const tournament::funding_source& f0, const tournament::funding_source& f1)
{
    return f0.is_addon == f1.is_addon &&
           f0.forbid_after_blind_level == f1.forbid_after_blind_level &&
           f0.chips == f1.chips &&
           f0.cost == f1.cost &&
           f0.commission == f1.commission &&
           f0.equity == f1.equity;
}

// fund a player, (re-)buyin or addon
void tournament::fund_player(const player_id& player, const funding_source& source)
{
    logger(LOG_DEBUG) << "Funding player " << player << '\n';

    if(this->current_blind_level > source.forbid_after_blind_level)
    {
        throw exception("too late in the game for this funding source");
    }

    if(source.is_addon && this->buyins.find(player) == this->buyins.end())
    {
        throw exception("tried to addon but not bought in yet");
    }

    if(player < this->cfg.players.size())
    {
        throw exception("unknown player id");
    }

    if(this->buyins.find(player) != this->buyins.end())
    {
        throw exception("player already bought in");
    }

    if(std::find(this->cfg.funding_sources.begin(), this->cfg.funding_sources.end(), source) == this->cfg.funding_sources.end())
    {
        throw exception("funding source not allowed");
    }

    auto seat_it(this->seats.find(player));

    if(seat_it == this->seats.end())
    {
        throw exception("player not seated");
    }

    // buy in player
    if(!source.is_addon)
    {
        this->buyins.insert(player);
    }

    // update totals
    this->total_chips += source.chips;
    this->total_cost += source.cost;
    this->total_commission += source.commission;
    this->total_equity += source.equity;

    // log it
    log(source.is_addon ? activity::did_addon : activity::did_buyin, player);
}

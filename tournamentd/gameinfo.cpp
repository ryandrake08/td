#include "gameinfo.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

// random number generator
static std::default_random_engine engine;

// initialize game
gameinfo::gameinfo() :
    table_capacity(0),
    percent_seats_paid(1.0),
    round_payouts(false),
    tables(0),
    total_chips(0),
    total_cost(0.0),
    total_commission(0.0),
    total_equity(0.0),
    running(false),
    current_blind_level(0),
    time_remaining(0),
    break_time_remaining(0),
    action_clock_remaining(0),
    elapsed(0)
{
}

// load configuration from JSON (object or file)
void gameinfo::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading tournament configuration\n";

    config.get_value("cost_currency", this->cost_currency);
    config.get_value("equity_currency", this->equity_currency);
    config.get_values("funding_sources", this->funding_sources);

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
        if(!this->seats.empty() || !this->players_finished.empty() || !this->buyins.empty())
        {
            logger(LOG_WARNING) << "Re-coniguring players list while in play is not advised, deleted players may still be in the game\n";
        }

        this->players.clear();
        for(auto player : players_vector)
        {
            this->players.emplace(std::hash<td::player>()(player), player);
        }
    }

    if(config.get_value("table_capacity", this->table_capacity))
    {
        if(!this->seats.empty())
        {
            logger(LOG_WARNING) << "Re-configuring table capacity will clear seating plan\n";
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
    recalculate = recalculate || config.get_value("round_payouts", this->round_payouts);
    recalculate = recalculate || config.get_value("percent_seats_paid", this->percent_seats_paid);
    if(recalculate)
    {
        // after reconfiguring, we'll need to recalculate
        this->recalculate_payouts(this->round_payouts);
    }

    if(config.get_values("blind_levels", this->blind_levels))
    {
        if(this->is_started())
        {
            logger(LOG_WARNING) << "Re-configuring blind levels while in play will stop the game\n";
            this->stop();
        }
    }
}

// dump configuration to JSON
void gameinfo::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping tournament configuration\n";

    config.set_values("players", this->players);
    config.set_value("table_capacity", this->table_capacity);
    config.set_value("cost_currency", this->cost_currency);
    config.set_value("equity_currency", this->equity_currency);
    config.set_value("percent_seats_paid", this->percent_seats_paid);
    config.set_values("funding_sources", this->funding_sources);
    config.set_values("blind_levels", this->blind_levels);
    config.set_values("available_chips", this->available_chips);
}

// dump state to JSON
void gameinfo::dump_state(json& state) const
{
    logger(LOG_DEBUG) << "Dumping tournament state\n";

    state.set_values("seats", this->seats);
    state.set_value("players_finished", json(this->players_finished));
    state.set_values("empty_seats", this->empty_seats);
    state.set_value("tables", this->tables);
    state.set_value("buyins", json(this->buyins));
    state.set_value("payouts", json(this->payouts));
    state.set_value("total_chips", this->total_chips);
    state.set_value("total_cost", this->total_cost);
    state.set_value("total_commission", this->total_commission);
    state.set_value("total_equity", this->total_equity);
    state.set_value("running", this->running);
    state.set_value("current_blind_level", this->current_blind_level);
    state.set_value("time_remaining", this->time_remaining.count());
    state.set_value("break_time_remaining", this->break_time_remaining.count());
    state.set_value("action_clock_remaining", this->action_clock_remaining.count());
    state.set_value("elapsed", this->elapsed.count());
}

std::vector<std::vector<td::player_id_t>> gameinfo::players_at_tables() const
{
    // build up two vectors, outer = tables, inner = players per table
    std::vector<std::vector<td::player_id_t>> ret(this->tables);

    for(auto seat : this->seats)
    {
        ret[seat.second.table_number].push_back(seat.first);
    }

    for(std::size_t i(0); i<ret.size(); i++)
    {
        logger(LOG_DEBUG) << "Table " << i << " has " << ret[i].size() << " players\n";
    }

    return ret;
}

std::size_t gameinfo::plan_seating(std::size_t max_expected_players)
{
    logger(LOG_DEBUG) << "Planning tournament for " << max_expected_players << " players\n";

    // check arguments
    if(max_expected_players < 2)
    {
        throw td::runtime_error("expected players must be at least 2");
    }

    // check configuration: table capacity should be sane
    if(this->table_capacity < 2)
    {
        throw td::runtime_error("table capacity must be at least 2");
    }

    // reset to known quantities
    this->seats.clear();
    this->players_finished.clear();
    this->empty_seats.clear();

    // figure out how many tables needed
    this->tables = ((max_expected_players-1) / this->table_capacity) + 1;

    logger(LOG_DEBUG) << "Tables needed: " << this->tables << "\n";

    // build up seat list
    for(std::size_t t(0); t<this->tables; t++)
    {
        for(std::size_t s(0); s<this->table_capacity; s++)
        {
            this->empty_seats.push_back(td::seat(t,s));
        }
    }

    // randomize it
    std::shuffle(this->empty_seats.begin(), this->empty_seats.end(), engine);

    logger(LOG_DEBUG) << "Created " << this->seats.size() << " empty seats\n";

    // return number of tables needed
    return this->tables;
}

// add player to an existing game
td::seat gameinfo::add_player(const td::player_id_t& player_id)
{
    logger(LOG_DEBUG) << "Adding player " << player_id << " to game\n";

    // verify game state
    if(this->empty_seats.empty())
    {
        throw td::runtime_error("tried to add players with no empty seats");
    }

    auto seat_it(this->seats.find(player_id));

    if(seat_it != this->seats.end())
    {
        throw td::runtime_error("tried to add player already seated");
    }

    // seat player and remove from empty list
    auto seat(this->empty_seats.front());
    this->seats.insert(std::make_pair(player_id, seat));
    this->empty_seats.pop_front();

    logger(LOG_DEBUG) << "Seated player at table " << seat.table_number << ", seat " << seat.seat_number << '\n';

    return seat;
}

// remove a player
std::vector<td::player_movement> gameinfo::remove_player(const td::player_id_t& player_id)
{
    logger(LOG_DEBUG) << "Removing player " << player_id << " from game\n";

    auto seat_it(this->seats.find(player_id));

    if(seat_it == this->seats.end())
    {
        throw td::runtime_error("tried to remove player not seated");
    }

    // bust player and add seat to the end of the empty list
    this->empty_seats.push_back(seat_it->second);
    this->seats.erase(seat_it);
    this->players_finished.push_front(player_id);

    // try to break table or rebalance
    std::vector<td::player_movement> movements;
    this->try_break_table(movements);
    this->try_rebalance(movements);
    return movements;
}

// move a player to a specific table
td::player_movement gameinfo::move_player(const td::player_id_t& player_id, std::size_t table)
{
    logger(LOG_DEBUG) << "Moving player " << player_id << " to table " << table << '\n';

    // build up a list of candidate seats
    std::vector<std::deque<td::seat>::iterator> candidates;
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
        throw td::runtime_error("tried to move player to a full table");
    }

    // pick one at random
    auto index(std::uniform_int_distribution<std::size_t>(0, candidates.size()-1)(engine));
    auto to_seat(*candidates[index]);

    // move player
    auto player_seat_it(this->seats.find(player_id));
    auto from_seat(player_seat_it->second);
    this->empty_seats.push_back(from_seat);
    player_seat_it->second = to_seat;

    logger(LOG_DEBUG) << "Moved player " << player_id << " from table " << from_seat.table_number << ", seat " << from_seat.seat_number << " to table " << to_seat.table_number << ", seat " << to_seat.seat_number << '\n';

    return td::player_movement(player_id, from_seat, to_seat);
}

// move a player to the table with the smallest number of players
td::player_movement gameinfo::move_player(const td::player_id_t& player_id, const std::unordered_set<std::size_t>& avoid_tables)
{
    logger(LOG_DEBUG) << "Moving player " << player_id << " to a free table\n";

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
        throw td::runtime_error("tried to move player to another table but no candidate tables");
    }

    return this->move_player(player_id, table);
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
    logger(LOG_DEBUG) << "Attemptying to rebalance tables\n";

    std::size_t ret(0);

    // build players-per-table vector
    auto ppt(this->players_at_tables());

    // find smallest and largest tables
    auto fewest_it(std::min_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id_t>>));
    auto most_it(std::max_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id_t>>));

    // if fewest has two fewer players than most (e.g. 6 vs 8), then rebalance
    while(fewest_it->size() < most_it->size() - 1)
    {
        logger(LOG_DEBUG) << "Largest table has " << most_it->size() << " players and smallest table has " << fewest_it->size() << " players\n";

        // pick a random player at the table with the most players
        auto index(std::uniform_int_distribution<std::size_t>(0, most_it->size()-1)(engine));
        auto random_player((*most_it)[index]);

        // subtract iterator to find table number
        auto table(fewest_it - ppt.begin());
        movements.push_back(this->move_player(random_player, table));
        ret++;

        // update our lists to stay consistent
        (*most_it)[index] = std::move(most_it->back());
        most_it->pop_back();
        fewest_it->push_back(random_player);
    }

    return ret;
}

// break a table if possible
// returns number of movements, or zero, if no players moved
std::size_t gameinfo::try_break_table(std::vector<td::player_movement>& movements)
{
    logger(LOG_DEBUG) << "Attemptying to break a table\n";

    std::size_t ret(0);

    if(this->tables > 1)
    {
        // break tables while (player_count-1) div table_capacity < tables
        while((this->seats.size() - 1) / this->table_capacity < this->tables)
        {
            logger(LOG_DEBUG) << "Breaking a table. " << this->seats.size() << " players remain at " << this->tables << " tables of " << this->table_capacity << " capacity\n";

            // always break the highest-numbered table
            auto break_table(this->tables - 1);

            // get each player found to be sitting at table
            std::vector<td::player_id_t> to_move;
            for(auto seat : this->seats)
            {
                if(seat.second.table_number == break_table)
                {
                    to_move.push_back(seat.first);
                }
            }

            // move each player in list
            const std::unordered_set<std::size_t> avoid = {break_table};
            for(auto player : to_move)
            {
                movements.push_back(this->move_player(player, avoid));
                ret++;
            }

            // decrement number of tables
            this->tables--;

            // prune empty table from our open seat list, no need to seat people at unused tables
            this->empty_seats.erase(std::remove_if(this->empty_seats.begin(), this->empty_seats.end(), [&break_table](const td::seat& seat) { return seat.table_number == break_table; }));
            
            logger(LOG_DEBUG) << "Broken table " << break_table << ". " << this->seats.size() << " players now at " << this->tables << " tables\n";
        }
    }
    
    return ret;
}

// fund a player, (re-)buyin or addon
void gameinfo::fund_player(const td::player_id_t& player_id, const td::funding_source_id_t& src, std::size_t current_blind_level)
{
    if(src >= this->funding_sources.size())
    {
        throw td::runtime_error("invalid funding source");
    }

    const td::funding_source& source(this->funding_sources[src]);

    if(current_blind_level > source.forbid_after_blind_level)
    {
        throw td::runtime_error("too late in the game for this funding source");
    }

    if(source.is_addon && this->buyins.find(player_id) == this->buyins.end())
    {
        throw td::runtime_error("tried to addon but not bought in yet");
    }

    logger(LOG_DEBUG) << "Funding player " << player_id << " with " << source.name << '\n';

    //add player to buyin set
    this->buyins.insert(player_id);

    // update totals
    this->total_chips += source.chips;
    this->total_cost += source.cost;
    this->total_commission += source.commission;
    this->total_equity += source.equity;

    // automatically recalculate
    this->recalculate_payouts(this->round_payouts);
}

// return the maximum number of chips available per player for a given denomination
unsigned long gameinfo::max_chips_for(unsigned long denomination, std::size_t players_count) const
{
    if(players_count == 0)
    {
        throw std::invalid_argument("players_count must be non-zero");
    }

    // find denomination in available_chips
    auto it(std::find_if(this->available_chips.begin(), this->available_chips.end(), [denomination](const td::chip& c) { return c.denomination == denomination; }));
    if(it == this->available_chips.end())
    {
        throw td::runtime_error("denomination not found in chip set");
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
        throw td::runtime_error("invalid funding source");
    }

    const td::funding_source& source(this->funding_sources[src]);

    if(this->blind_levels.size() < 2)
    {
        throw td::runtime_error("tried to calculate chips for a buyin without blind levels defined");
    }

    if(this->available_chips.empty())
    {
        throw td::runtime_error("tried to calculate chips for a buyin without chips defined");
    }

    // ensure our smallest available chip can play the smallest small blind
    if(this->available_chips[0].denomination > this->blind_levels[1].little_blind)
    {
        throw td::runtime_error("smallest chip available is larger than the smallest little blind");
    }

    // counts for each denomination
    std::unordered_map<unsigned long,unsigned long> q;

    // step 1: fund using highest denominaton chips available
    long remain(source.chips);
    auto cit(this->available_chips.rbegin());
    while(remain)
    {
        // find highest denomination chip less than what remains
        while(cit->denomination > remain)
        {
            cit++;
        }

        auto d(cit->denomination);

        // add chips and remove from remainder
        auto count(remain / d);
        while(count+q[d] > this->max_chips_for(d, max_expected_players)) count--;
        q[d] += count;
        remain -= count*d;

        logger(LOG_DEBUG) << "Initial chips: T" << d << ": " << q[d] << '\n';
    }

    // check that we can represent buyin with our chip set
    if(remain != 0)
    {
        throw td::runtime_error("buyin is not a multiple of the smallest chip available");
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
                    logger(LOG_DEBUG) << "Move chips: T" << d1 << ": 1 -> T" << d0 << ": " << count << '\n';
                }

                logger(LOG_DEBUG) << "Current chips: T" << d1 << ": " << q[d1] << '\n';
                logger(LOG_DEBUG) << "Current chips: T" << d0 << ": " << q[d0] << '\n';
                logger(LOG_DEBUG) << "Remaining value: T" << remain << '\n';
            }
        }

        // step 3: keep doing the above until no chips left to move
    }
    while(moved_a_chip);

    // check that we can represent buyin with our chip set
    if(remain != 0)
    {
        logger(LOG_DEBUG) << "Done moving, remaining value: T" << remain << '\n';
        return std::vector<td::player_chips>();
    }

    // convert to vector to return
    std::vector<td::player_chips> ret;
    for(auto p : q)
    {
        if(p.second)
        {
            ret.push_back(td::player_chips(p.first, p.second));
        }
    }

    return ret;
}

// re-calculate payouts
void gameinfo::recalculate_payouts(bool round)
{
    // first, calculate how many places pay, given configuration and number of players bought in
    std::size_t seats_paid(static_cast<std::size_t>(this->buyins.size() * this->percent_seats_paid + 0.5));

    logger(LOG_DEBUG) << "Recalculating payouts: " << seats_paid << " seats will be paid\n";

    // resize our payout structure
    this->payouts.resize(seats_paid);

    if(!this->payouts.empty())
    {
        // ratio for each seat is comp[seat]:total
        std::vector<double> comp(seats_paid);
        double total(0.0);

        // generate proportional payouts based on harmonic series, 1/N / sum(1/k)
        for(auto n(0); n<seats_paid; n++)
        {
            double c(1.0/(n+1));
            comp[n] = c;
            total += c;
        }

        // next, loop through again generating payouts
        if(round)
        {
            std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c) { return std::round(this->total_equity * c / total); });

            // remainder (either positive or negative) adjusts first place
            auto remainder(this->total_equity - std::accumulate(this->payouts.begin(), this->payouts.end(), 0.0));
            this->payouts[0] += remainder;
        }
        else
        {
            std::transform(comp.begin(), comp.end(), this->payouts.begin(), [&](double c) { return this->total_equity * c / total; });
        }
    }
}

// utility: start a blind level (optionally starting offset ms into the round)
void gameinfo::start_blind_level(std::size_t blind_level, duration_t offset)
{
    if(blind_level >= this->blind_levels.size())
    {
        throw td::runtime_error("not enough blind levels configured");
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
        throw td::runtime_error("tournament already started");
    }

    if(this->blind_levels.size() < 2)
    {
        throw td::runtime_error("cannot start without blind levels configured");
    }

    logger(LOG_DEBUG) << "Starting the tournament\n";

    // start the blind level
    this->start_blind_level(1, duration_t::zero());

    // start the tournament
    this->running = true;

    // set elapsed time
    this->elapsed = duration_t::zero();
}

void gameinfo::start(const time_point_t& starttime)
{
    if(this->is_started())
    {
        throw td::runtime_error("tournament already started");
    }

    if(this->blind_levels.size() < 2)
    {
        throw td::runtime_error("cannot start without blind levels configured");
    }

    logger(LOG_DEBUG) << "Starting the tournament in the future\n";

    // start the tournament
    this->running = true;

    // set break end time
    this->end_of_break = starttime;

    // set elapsed time
    this->elapsed = duration_t::zero();
}

// stop the game
void gameinfo::stop()
{
    if(!this->is_started())
    {
        throw td::runtime_error("tournament not started");
    }

    logger(LOG_DEBUG) << "Stopping the tournament\n";

    this->running = false;
    this->current_blind_level = 0;
    this->end_of_round = time_point_t();
    this->end_of_break = time_point_t();
    this->end_of_action_clock = time_point_t();
    this->time_remaining = duration_t::zero();
    this->break_time_remaining = duration_t::zero();
    this->action_clock_remaining = duration_t::zero();
    this->elapsed = duration_t::zero();
}

// pause
void gameinfo::pause()
{
    if(!this->is_started())
    {
        throw td::runtime_error("tournament not started");
    }

    logger(LOG_DEBUG) << "Pausing the tournament\n";

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
        throw td::runtime_error("tournament not started");
    }

    logger(LOG_DEBUG) << "Resuming the tournament\n";

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
        throw td::runtime_error("tournament not started");
    }

    if(this->current_blind_level + 1 < this->blind_levels.size())
    {
        logger(LOG_DEBUG) << "Setting next blind level from " << this->current_blind_level << " to " << this->current_blind_level + 1 << '\n';

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
        throw td::runtime_error("tournament not started");
    }

    // if elapsed time > 2 seconds, just restart current blind level
    // TODO: configurable?
    static auto max_elapsed_time_ms(2000);

    // calculate elapsed time in this blind level
    auto elapsed_time(this->blind_levels[this->current_blind_level].duration - this->time_remaining.count());
    if(elapsed_time > max_elapsed_time_ms || this->current_blind_level == 1)
    {
        logger(LOG_DEBUG) << "Restarting blind level " << this->current_blind_level << '\n';

        this->start_blind_level(this->current_blind_level, offset);
        return false;
    }
    else
    {
        logger(LOG_DEBUG) << "Setting previous blind level from " << this->current_blind_level << " to " << this->current_blind_level - 1 << '\n';

        this->start_blind_level(this->current_blind_level - 1, offset);
        return true;
    }
}

// update time remaining
bool gameinfo::update_remaining()
{
    if(this->running)
    {
        auto now(std::chrono::system_clock::now());

        // always update action clock if ticking
        if(this->end_of_action_clock != time_point_t())
        {
            if(this->end_of_action_clock > now)
            {
                this->action_clock_remaining = std::chrono::duration_cast<duration_t>(this->end_of_action_clock - now);
            }
            else
            {
                this->end_of_action_clock = time_point_t();
                this->action_clock_remaining = duration_t::zero();
            }
        }
        else
        {
            this->action_clock_remaining = duration_t::zero();
        }

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
        return true;
    }
    return false;
}

// set the action clock (when someone 'needs the clock called on them'
void gameinfo::set_action_clock(long duration)
{
    if(this->end_of_action_clock == time_point_t())
    {
        this->end_of_action_clock = std::chrono::system_clock::now() + duration_t(duration);
        this->action_clock_remaining = duration_t(duration);
    }
    else
    {
        throw td::runtime_error("only one action clock at a time");
    }
}

// reset the action clock
void gameinfo::reset_action_clock()
{
    this->end_of_action_clock = time_point_t();
    this->action_clock_remaining = duration_t::zero();
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
        throw td::runtime_error("tried to create a blind structure without chips defined");
    }

    // resize structure, zero-initializing
    this->blind_levels.resize(count+1);

    // store last round denomination (to check when it changes)
    auto last_round_denom(this->available_chips.begin()->denomination);

    // starting small blind = smallest denomination
    double ideal_small(static_cast<double>(last_round_denom));

    for(auto i(1); i<count+1; i++)
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
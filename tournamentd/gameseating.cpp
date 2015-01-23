#include "gameseating.hpp"
#include "logger.hpp"
#include <algorithm>

// random number generator
static std::default_random_engine engine;

// initialize game seating chart
gameseating::gameseating() : table_capacity(0), tables(0)
{
}

// reset game state
void gameseating::reset()
{
    logger(LOG_DEBUG) << "Resetting all seating information\n";

    seats.clear();
    players_finished.clear();
    empty_seats.clear();
    tables = 0;
}

// load configuration from JSON (object or file)
void gameseating::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading game seating configuration\n";

    if(config.get_value("table_capacity", this->table_capacity))
    {
        if(!this->seats.empty())
        {
            logger(LOG_WARNING) << "Re-configuring table capacity while in play will clear seating plan\n";
        }
        
        // if seats are already set up, replan based on new table capacity
        std::size_t total_seats = this->seats.size() + this->empty_seats.size();
        if(total_seats >= 2)
        {
            // re-plan seating, if needed
            this->plan_seating(total_seats);
        }
    }

    // check if we're changing the players list
    if(config.has_object("players"))
    {
        if(!this->seats.empty() || !this->players_finished.empty())
        {
            logger(LOG_WARNING) << "Re-coniguring players list while in play is not advised, deleted players may still be in the game\n";
        }
    }
}

// dump configuration to JSON
void gameseating::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping game seating configuration\n";

    config.set_value("table_capacity", this->table_capacity);
}

// dump state to JSON
void gameseating::dump_state(json& state) const
{
    logger(LOG_DEBUG) << "Dumping game seating state\n";

    // players without seats or busted out
    state.set_value("players_finished", json(this->players_finished));
    state.set_values("seats", this->seats);
    state.set_values("empty_seats", this->empty_seats);
    state.set_value("tables", this->tables);
}

std::vector<std::vector<td::player_id>> gameseating::players_at_tables() const
{
    // build up two vectors, outer = tables, inner = players per table
    std::vector<std::vector<td::player_id>> ret(this->tables);

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

std::size_t gameseating::plan_seating(std::size_t max_expected_players)
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
td::seat gameseating::add_player(const td::player_id& player)
{
    logger(LOG_DEBUG) << "Adding player " << player << " to game\n";

    // verify game state
    if(this->empty_seats.empty())
    {
        throw td::runtime_error("tried to add players with no empty seats");
    }

    auto seat_it(this->seats.find(player));

    if(seat_it != this->seats.end())
    {
        throw td::runtime_error("tried to add player already seated");
    }

    // seat player and remove from empty list
    auto seat(this->empty_seats.front());
    this->seats.insert(std::make_pair(player, seat));
    this->empty_seats.pop_front();

    logger(LOG_DEBUG) << "Seated player at table " << seat.table_number << ", seat " << seat.seat_number << '\n';

    return seat;
}

// remove a player
std::vector<td::player_movement> gameseating::remove_player(const td::player_id& player)
{
    logger(LOG_DEBUG) << "Removing player " << player << " from game\n";

    auto seat_it(this->seats.find(player));

    if(seat_it == this->seats.end())
    {
        throw td::runtime_error("tried to remove player not seated");
    }

    // bust player and add seat to the end of the empty list
    this->empty_seats.push_back(seat_it->second);
    this->seats.erase(seat_it);
    this->players_finished.push_front(player);

    // try to break table or rebalance
    std::vector<td::player_movement> movements;
    this->try_break_table(movements);
    this->try_rebalance(movements);
    return movements;
}

// move a player to a specific table
td::player_movement gameseating::move_player(const td::player_id& player, std::size_t table)
{
    logger(LOG_DEBUG) << "Moving player " << player << " to table " << table << '\n';

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
    auto player_seat_it(this->seats.find(player));
    auto from_seat(player_seat_it->second);
    this->empty_seats.push_back(from_seat);
    player_seat_it->second = to_seat;

    logger(LOG_DEBUG) << "Moved player " << player << " from table " << from_seat.table_number << ", seat " << from_seat.seat_number << " to table " << to_seat.table_number << ", seat " << to_seat.seat_number << '\n';

    return td::player_movement(player, from_seat, to_seat);
}

// move a player to the table with the smallest number of players
td::player_movement gameseating::move_player(const td::player_id& player, const std::unordered_set<std::size_t>& avoid_tables)
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
        throw td::runtime_error("tried to move player to another table but no candidate tables");
    }

    return this->move_player(player, table);
}

template <typename T>
static constexpr bool has_lower_size(const T& i0, const T& i1)
{
    return i0.size() < i1.size();
}

// re-balance by moving any player from a large table to a smaller one
// returns number of movements, or zero, if no players moved
std::size_t gameseating::try_rebalance(std::vector<td::player_movement>& movements)
{
    logger(LOG_DEBUG) << "Attemptying to rebalance tables\n";

    std::size_t ret(0);

    // build players-per-table vector
    auto ppt(this->players_at_tables());

    // find smallest and largest tables
    auto fewest_it(std::min_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id>>));
    auto most_it(std::max_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<td::player_id>>));

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
std::size_t gameseating::try_break_table(std::vector<td::player_movement>& movements)
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
            std::vector<td::player_id> to_move;
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
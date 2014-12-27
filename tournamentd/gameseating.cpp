#include "gameseating.hpp"
#include "logger.hpp"
#include <algorithm>

// initialize game seating chart
gameseating::gameseating() : table_capacity(0), tables(0)
{
}

// load configuration from JSON (object or file)
void gameseating::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading game seating configuration\n";

    config.get_value("table_capacity", this->table_capacity);
}

// dump configuration to JSON
void gameseating::dump_configuration(json& config) const
{
    config.set_value("table_capacity", this->table_capacity);
}

// dump state to JSON
void gameseating::dump_state(json& state) const
{
    std::vector<json> array;
    for(auto seatpair : seats)
    {
        json obj;
        obj.set_value("player_id", seatpair.first);
        obj.set_value("table_number", seatpair.second.table_number);
        obj.set_value("seat_number", seatpair.second.seat_number);
        array.push_back(obj);
    }
    state.set_value("seats", array);

    // players without seats or busted out
    std::vector<player_id> tmp_finished(this->players_finished.begin(), this->players_finished.end());
    state.set_value("players_finished", json(tmp_finished));

    array.clear();
    for(auto seating : this->empty_seats)
    {
        json obj;
        obj.set_value("table_number", seating.table_number);
        obj.set_value("seat_number", seating.seat_number);
        array.push_back(obj);
    }
    state.set_value("empty_seats", array);
}

std::vector<std::vector<player_id>> gameseating::players_at_tables() const
{
    // build up two vectors, outer = tables, inner = players per table
    std::vector<std::vector<player_id>> ret(this->tables);

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
        throw tournament_error("expected players must be at least 2");
    }

    // check configuration: table capacity should be sane
    if(this->table_capacity < 2)
    {
        throw tournament_error("table capacity must be at least 2");
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
            this->empty_seats.push_back(seat({t,s}));
        }
    }

    // randomize it
    std::shuffle(this->empty_seats.begin(), this->empty_seats.end(), engine);

    logger(LOG_DEBUG) << "Created " << this->seats.size() << " empty seats\n";

    // return number of tables needed
    return this->tables;
}

// add player to an existing game
gameseating::seat gameseating::add_player(const player_id& player)
{
    logger(LOG_DEBUG) << "Adding player " << player << " to game\n";

    // verify game state
    if(this->empty_seats.empty())
    {
        throw tournament_error("tried to add players with no empty seats");
    }

    auto seat_it(this->seats.find(player));

    if(seat_it != this->seats.end())
    {
        throw tournament_error("tried to add player already seated");
    }

    // seat player and remove from empty list
    auto seat(this->empty_seats.front());
    this->seats[player] = seat;
    this->empty_seats.pop_front();

    logger(LOG_DEBUG) << "Seated player at table " << seat.table_number << ", seat " << seat.seat_number << '\n';

    return seat;
}

// remove a player
std::size_t gameseating::remove_player(const player_id& player)
{
    logger(LOG_DEBUG) << "Removing player " << player << " from game\n";

    auto seat_it(this->seats.find(player));

    if(seat_it == this->seats.end())
    {
        throw tournament_error("tried to remove player not seated");
    }

    // bust player and add seat to the end of the empty list
    this->seats.erase(player);
    this->players_finished.push_front(player);
    this->empty_seats.push_back(seat_it->second);

    return this->players_finished.size();
}

// move a player to a specific table
gameseating::player_movement gameseating::move_player(const player_id& player, std::size_t table)
{
    logger(LOG_DEBUG) << "Moving player " << player << " to table " << table << '\n';

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
        throw tournament_error("tried to move player to a full table");
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

    return { player, from_seat, to_seat };
}

// move a player to the table with the smallest number of players
gameseating::player_movement gameseating::move_player(const player_id& player, const std::unordered_set<std::size_t>& avoid_tables)
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
        throw tournament_error("tried to move player to another table but no candidate tables");
    }

    return this->move_player(player, table);
}

template <typename T>
static constexpr bool has_lower_size(const T& i0, const T& i1)
{
    return i0.size() < i1.size();
}

// re-balance by moving any player from a large table to a smaller one, returns true if player was moved
std::vector<gameseating::player_movement> gameseating::try_rebalance()
{
    logger(LOG_DEBUG) << "Attemptying to rebalance tables\n";

    // build players-per-table vector
    auto ppt(this->players_at_tables());

    // find smallest and largest tables
    auto fewest_it(std::min_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<player_id>>));
    auto most_it(std::max_element(ppt.begin(), ppt.end(), has_lower_size<std::vector<player_id>>));

    // return all player movements
    std::vector<gameseating::player_movement> ret;

    // if fewest has two fewer players than most (e.g. 6 vs 8), then rebalance
    while(fewest_it->size() < most_it->size() - 1)
    {
        logger(LOG_DEBUG) << "Largest table has " << most_it->size() << " players and smallest table has " << fewest_it->size() << " players\n";
        
        // pick a random player at the table with the most players
        auto index(std::uniform_int_distribution<std::size_t>(0, most_it->size()-1)(engine));
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

std::vector<gameseating::player_movement> gameseating::try_break_table()
{
    logger(LOG_DEBUG) << "Attemptying to break a table\n";

    // return all player movements
    std::vector<gameseating::player_movement> ret;

    if(this->tables > 1)
    {
        // break tables while (player_count-1) div table_capacity < tables
        while((this->seats.size() - 1) / this->table_capacity < this->tables)
        {
            logger(LOG_DEBUG) << "Breaking a table. " << this->seats.size() << " players remain at " << this->tables << " tables of " << this->table_capacity << " capacity\n";

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
            const std::unordered_set<std::size_t> avoid = {break_table};
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
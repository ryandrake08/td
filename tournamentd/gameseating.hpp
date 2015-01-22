#pragma once
#include "json.hpp"
#include "types.hpp"
#include <deque>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class gameseating
{
    // configuration: number of players per table
    std::size_t table_capacity;

    // players seated in the game
    std::unordered_map<td::player_id,td::seat> seats;

    // players without seats or busted out
    std::deque<td::player_id> players_finished;

    // empty seats
    std::deque<td::seat> empty_seats;

    // number of tables total
    std::size_t tables;

    // reset game state
    void reset();

    // utility: arrange tables with lists of players
    std::vector<std::vector<td::player_id>> players_at_tables() const;

    // move a player to a specific table
    // returns player's original seat and new seat
    td::player_movement move_player(const td::player_id& player, std::size_t table);

    // move a player to the table with the smallest number of players, optionally avoiding a particular table
    // returns player's movement
    td::player_movement move_player(const td::player_id& player, const std::unordered_set<std::size_t>& avoid_tables);

    // re-balance by moving any player from a large table to a smaller one
    // returns number of movements, or zero, if no players moved
    std::size_t try_rebalance(std::vector<td::player_movement>& movements);

    // break a table if possible
    // returns number of movements, or zero, if no players moved
    std::size_t try_break_table(std::vector<td::player_movement>& movements);

public:
    // initialize game seating chart
    gameseating();

    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;

    // dump state to JSON
    void dump_state(json& state) const;

    // pre-game player seeting, with expected number of players (to predict table count)
    // returns number of tables needed
    std::size_t plan_seating(std::size_t max_expected_players);

    // add player to an existing game
    // returns player's seat
    td::seat add_player(const td::player_id& player);

    // remove a player from the game
    // returns any player movements that happened
    std::vector<td::player_movement> remove_player(const td::player_id& player);
};
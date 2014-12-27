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
public:
    struct seat
    {
        std::size_t table_number;
        std::size_t seat_number;
    };

    struct player_movement
    {
        player_id player;
        seat from_seat;
        seat to_seat;
    };

private:
    // random number generator
    std::default_random_engine engine;

    // configuration: number of players per table
    std::size_t table_capacity;

    // players seated in the game
    std::unordered_map<player_id,seat> seats;

    // players without seats or busted out
    std::deque<player_id> players_finished;

    // empty seats
    std::deque<seat> empty_seats;

    // number of tables total
    std::size_t tables;

    // utility: arrange tables with lists of players
    std::vector<std::vector<player_id>> players_at_tables() const;

public:
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
    seat add_player(const player_id& player);

    // remove a player from the game
    // returns bust-out order (1 being the first to bust)
    std::size_t remove_player(const player_id& player);

    // move a player to a specific table
    // returns player's original seat and new seat
    player_movement move_player(const player_id& player, std::size_t table);

    // move a player to the table with the smallest number of players, optionally avoiding a particular table
    // returns player's movement
    player_movement move_player(const player_id& player, const std::unordered_set<std::size_t>& avoid_tables);

    // re-balance by moving any player from a large table to a smaller one, returns true if player was moved
    // returns all player movements
    std::vector<player_movement> try_rebalance();

    // break a table if possible
    // returns all player movements
    std::vector<player_movement> try_break_table();
};
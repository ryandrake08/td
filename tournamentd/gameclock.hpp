#pragma once
#include "json.hpp"
#include "types.hpp"
#include <string>
#include <vector>

class gameclock
{
    // configuration: blind structure for this game
    std::vector<td::blind_level> blind_levels;

    // configuration: description of each chip (for display)
    std::vector<td::chip> available_chips;

    // is the game running or paused?
    bool running;

    // Current bind level (index into above vector)
    // Note: blind level 0 is reserved for setup/planning
    std::size_t current_blind_level;

    // represents a point in time
    typedef std::chrono::system_clock::time_point time_point_t;

    // represents a time duration
    typedef std::chrono::milliseconds duration_t;

    // end of period (valid when running)
    time_point_t end_of_round;
    time_point_t end_of_break;

    // ms remaining
    duration_t time_remaining;
    duration_t break_time_remaining;

    // action clock
    time_point_t end_of_action_clock;
    duration_t action_clock_remaining;

    // elapsed time
    duration_t elapsed;

    // reset game state
    void reset();

    // utility: start a blind level
    void start_blind_level(std::size_t blind_level, duration_t offset);

public:
    // initialize game clock
    gameclock();

    // load configuration from JSON (object or file)
    void configure(const json& config);

    // dump configuration to JSON
    void dump_configuration(json& config) const;

    // dump state to JSON
    void dump_state(json& state) const;

    // has the game started?
    bool is_started() const;

    // return current blind level
    std::size_t get_current_blind_level() const;

    // start the game (optionally at certain time);
    void start();
    void start(const time_point_t& starttime);

    // stop the game
    void stop();

    // pause
    void pause();

    // resume
    void resume();

    // toggle pause/remove
    void toggle_pause_resume();

    // advance to next blind level
    // returns: true if blind level changed, false if we are at end of levels
    bool next_blind_level(duration_t offset=duration_t::zero());

    // return to previous blind level
    // returns: true if blind level changed, false if blind level was just reset
    bool previous_blind_level(duration_t offset=duration_t::zero());

    // update time remaining
    bool update_remaining();

    // set the action clock (when someone 'needs the clock called on them'
    void set_action_clock(long duration);

    // reset the action clock
    void reset_action_clock();

    // generate progressive blind levels, given available chip denominations
    void gen_blind_levels(std::size_t count, long level_duration, long chip_up_break_duration, double blind_increase_factor);
};
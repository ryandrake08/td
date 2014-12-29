#pragma once
#include "json.hpp"
#include "types.hpp"
#include <chrono>
#include <string>
#include <vector>

class gameclock
{
public:
    typedef std::chrono::system_clock::time_point tp;
    typedef std::chrono::milliseconds ms;

    struct blind_level
    {
        std::size_t little_blind;
        std::size_t big_blind;
        std::size_t ante;
        long duration;
        long break_duration;
    };

    struct chip
    {
        std::string color;
        std::size_t denomination;
        std::size_t count_available;
    };

    // configuration: blind increase factor
    double blind_increase_factor;

    // configuration: blind structure for this game
    std::vector<blind_level> blind_levels;

    // configuration: description of each chip (for display)
    std::vector<chip> chips;

    // is the game running or paused?
    bool running;

    // Current bind level (index into above vector)
    // Note: blind level 0 is reserved for setup/planning
    std::size_t current_blind_level;

    // end of period (valid when running)
    tp end_of_round;
    tp end_of_break;

    // ms remaining
    ms time_remaining;
    ms break_time_remaining;

    // action clock
    tp end_of_action_clock;
    ms action_clock_remaining;

    // utility: start a blind level
    void start_blind_level(std::size_t blind_level, ms offset);

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

    // start the game (optionally at certain time);
    void start();
    void start(const tp& starttime);

    // stop the game
    void stop();

    // toggle pause
    void pause();

    // toggle resume
    void resume();

    // advance to next blind level
    bool next_blind_level(ms offset=ms::zero());

    // return to previous blind level
    bool previous_blind_level(ms offset=ms::zero());

    // update time remaining
    bool update_remaining();

    // set the action clock (when someone 'needs the clock called on them'
    void set_action_clock(long duration);

    // reset the action clock
    void reset_action_clock();

    // generate progressive blind levels, given available chip denominations
    void gen_blind_levels(std::size_t count, long level_duration);
};
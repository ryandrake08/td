#include "gameclock.hpp"
#include "logger.hpp"
#include <algorithm>
#include <cmath>

// initialize game clock
gameclock::gameclock() : running(false), current_blind_level(0), time_remaining(0), break_time_remaining(0), action_clock_remaining(0), elapsed(0)
{
}

// reset game state
void gameclock::reset()
{
    logger(LOG_DEBUG) << "Resetting all clock information\n";

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

// load configuration from JSON (object or file)
void gameclock::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading game clock configuration\n";

    config.get_values("available_chips", this->available_chips);

    if(config.get_values("blind_levels", this->blind_levels))
    {
        if(this->current_blind_level > 0)
        {
            logger(LOG_WARNING) << "Re-configuring blind levels while in play\n";
        }
    }

    // reset game state
    this->reset();
}

// dump configuration to JSON
void gameclock::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping game clock configuration\n";

    config.set_values("blind_levels", this->blind_levels);
    config.set_values("available_chips", this->available_chips);
}

// dump state to JSON
void gameclock::dump_state(json& state) const
{
    logger(LOG_DEBUG) << "Dumping game clock state\n";

    state.set_value("running", this->running);
    state.set_value("current_blind_level", this->current_blind_level);
    state.set_value("time_remaining", this->time_remaining.count());
    state.set_value("break_time_remaining", this->break_time_remaining.count());
    state.set_value("action_clock_remaining", this->action_clock_remaining.count());
}

// utility: start a blind level (optionally starting offset ms into the round)
void gameclock::start_blind_level(std::size_t blind_level, duration_t offset)
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
bool gameclock::is_started() const
{
    return this->current_blind_level > 0;
}

// return current blind level
std::size_t gameclock::get_current_blind_level() const
{
    return this->current_blind_level;
}

// start the game
void gameclock::start()
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

void gameclock::start(const time_point_t& starttime)
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
void gameclock::stop()
{
    if(!this->is_started())
    {
        throw td::runtime_error("tournament not started");
    }

    logger(LOG_DEBUG) << "Stopping the tournament\n";

    this->reset();
}

// pause
void gameclock::pause()
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
void gameclock::resume()
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
void gameclock::toggle_pause_resume()
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
bool gameclock::next_blind_level(duration_t offset)
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
bool gameclock::previous_blind_level(duration_t offset)
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
bool gameclock::update_remaining()
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
void gameclock::set_action_clock(long duration)
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
void gameclock::reset_action_clock()
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
void gameclock::gen_blind_levels(std::size_t count, long level_duration, long chip_up_break_duration, double blind_increase_factor)
{
    if(this->available_chips.empty())
    {
        throw td::runtime_error("tried to create a blind structure without chips defined");
    }

    // resize structure, zero-initializing
    this->blind_levels.resize(count+1);

    // sort our vector
    std::sort(this->available_chips.begin(), this->available_chips.end(), [](const td::chip& c0, const td::chip& c1) { return c0.denomination < c1.denomination; });

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
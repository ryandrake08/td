#include "gameclock.hpp"
#include "logger.hpp"
#include <cmath>

// initialize game clock
gameclock::gameclock() : blind_increase_factor(1.5), running(false), current_blind_level(0), time_remaining(0), break_time_remaining(0)
{
}

// load configuration from JSON (object or file)
void gameclock::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading game clock configuration\n";

    config.get_value("blind_increase_factor", this->blind_increase_factor);

    std::vector<json> array;
    if(config.get_value("blind_levels", array))
    {
        this->blind_levels.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            array[i].get_value("little_blind", this->blind_levels[i].little_blind);
            array[i].get_value("big_blind", this->blind_levels[i].big_blind);
            array[i].get_value("ante", this->blind_levels[i].ante);
            array[i].get_value("duration_ms", this->blind_levels[i].duration);
            array[i].get_value("break_duration_ms", this->blind_levels[i].break_duration);
        }
    }

    if(config.get_value("chips", array))
    {
        this->chips.resize(array.size());
        for(std::size_t i(0); i<array.size(); i++)
        {
            array[i].get_value("color", this->chips[i].color);
            array[i].get_value("denomination", this->chips[i].denomination);
            array[i].get_value("count_available", this->chips[i].count_available);
        }
    }
}

// dump configuration to JSON
void gameclock::dump_configuration(json& config) const
{
    config.set_value("blind_increase_factor", this->blind_increase_factor);

    std::vector<json> array;
    for(auto level : this->blind_levels)
    {
        json obj;
        obj.set_value("little_blind", level.little_blind);
        obj.set_value("big_blind", level.big_blind);
        obj.set_value("ante", level.ante);
        obj.set_value("duration_ms", level.duration);
        obj.set_value("break_duration_ms", level.break_duration);
        array.push_back(obj);
    }
    config.set_value("blind_levels", array);

    array.clear();
    for(auto chip : this->chips)
    {
        json obj;
        obj.set_value("color", chip.color);
        obj.set_value("denomination", chip.denomination);
        obj.set_value("count_available", chip.count_available);
        array.push_back(obj);
    }
    config.set_value("chips", array);
}

// dump state to JSON
void gameclock::dump_state(json& state) const
{
    state.set_value("running", this->running);
    state.set_value("current_blind_level", this->current_blind_level);
    state.set_value("time_remaining", this->time_remaining.count());
    state.set_value("break_time_remaining", this->break_time_remaining.count());
}

// utility: start a blind level (optionally starting offset ms into the round)
void gameclock::start_blind_level(std::size_t blind_level, ms offset)
{
    if(blind_level >= this->blind_levels.size())
    {
        throw tournament_error("not enough blind levels configured");
    }

    auto now(std::chrono::system_clock::now());

    this->current_blind_level = blind_level;
    this->time_remaining = ms(this->blind_levels[this->current_blind_level].duration) - offset;
    this->break_time_remaining = ms(this->blind_levels[this->current_blind_level].break_duration);
    this->end_of_round = now + this->time_remaining;
    this->end_of_break = this->end_of_round + this->break_time_remaining;
}

// has the game started?
bool gameclock::is_started() const
{
    return this->current_blind_level > 0;
}

// start the game
void gameclock::start()
{
    if(!this->is_started())
    {
        logger(LOG_DEBUG) << "Starting the tournament\n";

        // start the tournament
        this->running = true;

        // start the blind level
        this->start_blind_level(1, ms::zero());
    }
}

void gameclock::start(const tp& starttime)
{
    if(!this->is_started())
    {
        logger(LOG_DEBUG) << "Starting the tournament in the future\n";

        // start the tournament
        this->running = true;

        // set break end time
        this->end_of_break = starttime;
    }
}

// stop the game
void gameclock::stop()
{
    if(this->is_started())
    {
        logger(LOG_DEBUG) << "Stopping the tournament\n";

        this->running = false;
        this->current_blind_level = 0;
        this->end_of_round = tp();
        this->end_of_break = tp();
        this->time_remaining = ms(0);
        this->break_time_remaining = ms(0);
    }
}

// toggle pause
void gameclock::pause()
{
    if(this->is_started())
    {
        logger(LOG_DEBUG) << "Pausing the tournament\n";

        // update time remaining
        this->update_remaining();

        // pause
        this->running = false;
    }
}

// toggle resume
void gameclock::resume()
{
    if(this->is_started())
    {
        logger(LOG_DEBUG) << "Resuming the tournament\n";

        auto now(std::chrono::system_clock::now());

        // update end_of_xxx with values saved when we paused
        this->end_of_round = now + this->time_remaining;
        this->end_of_break = this->end_of_round + this->break_time_remaining;

        // resume
        this->running = true;
    }
}

// advance to next blind level
bool gameclock::advance_blind_level(ms offset)
{
    if(this->is_started())
    {
        logger(LOG_DEBUG) << "Advancing blind level from " << this->current_blind_level << " to " << this->current_blind_level + 1 << '\n';

        if(this->current_blind_level + 1 < this->blind_levels.size())
        {
            this->start_blind_level(this->current_blind_level + 1, offset);
            return true;
        }
    }

    return false;
}

// restart current blind level
void gameclock::restart_blind_level(ms offset)
{
    if(this->is_started())
    {
        logger(LOG_DEBUG) << "Restarting blind level " << this->current_blind_level << '\n';

        this->start_blind_level(this->current_blind_level, offset);
    }
}

// return to prevous blind level
bool gameclock::previous_blind_level(ms offset)
{
    if(this->is_started())
    {
        logger(LOG_DEBUG) << "Returning blind level from " << this->current_blind_level << " to " << this->current_blind_level - 1 << '\n';

        if(this->current_blind_level > 1)
        {
            this->start_blind_level(this->current_blind_level - 1, offset);
            return true;
        }
    }

    return false;
}

// update time remaining
bool gameclock::update_remaining()
{
    if(this->running)
    {
        auto now(std::chrono::system_clock::now());

        // set time remaining based on current clock
        if(now < this->end_of_round)
        {
            // within round, set time remaining
            this->time_remaining = std::chrono::duration_cast<ms>(this->end_of_round - now);
        }
        else if(now < this->end_of_break)
        {
            // within break, set time remaining to zero and set break time remaining
            this->time_remaining = ms::zero();
            this->break_time_remaining = std::chrono::duration_cast<ms>(this->end_of_break - now);
        }
        else
        {
            // advance to next blind
            auto offset(std::chrono::duration_cast<ms>(this->end_of_break - now));
            this->advance_blind_level(offset);
        }

        return true;
    }
    return false;
}

static constexpr bool operator<(const gameclock::chip& c0, const gameclock::chip& c1)
{
    return c0.denomination < c1.denomination;
}

static std::size_t calculate_round_denomination(double ideal_small, const std::vector<gameclock::chip>& chips)
{
    // round to denomination n if ideal small blind is at least 10x denomination n-1
    static const std::size_t multiplier(10);

    std::size_t chip_index(chips.size());
    while(--chip_index > 0)
    {
        if(ideal_small > chips[chip_index-1].denomination * multiplier)
        {
            return chips[chip_index].denomination;
        }
    }

    return chips[0].denomination;
}

// generate progressive blind levels, given available chip denominations
// this was tuned to produce a sensible result for the following chip denomination sets:
//  1/5/25/100/500
//  5/25/100/500/1000
//  25/100/500/1000/5000
std::vector<gameclock::blind_level> gameclock::generate_blind_levels(std::size_t count, long level_duration)
{
    if(this->chips.empty())
    {
        throw tournament_error("tried to create a blind structure without chips defined");
    }

    // sort chip denominations
    std::sort(this->chips.begin(), this->chips.end());

    // resize structure
    this->blind_levels.resize(count);

    // starting small blind = smallest denomination
    double ideal_small(static_cast<double>(this->chips.front().denomination));

    // store last round denomination (to check when it changes)
    std::size_t last_round_denom(0);

    for(auto i(0); i<count; i++)
    {
        // calculate nearest chip denomination to round to
        const auto round_denom(calculate_round_denomination(ideal_small, this->chips));

        // round up
        this->blind_levels[i].little_blind = std::ceil(ideal_small / round_denom) * round_denom;
        this->blind_levels[i].big_blind = this->blind_levels[i].little_blind * 2;
        this->blind_levels[i].ante = 0;
        this->blind_levels[i].duration = level_duration;

        if(i>0 && round_denom != last_round_denom)
        {
            // 5 minute break to chip up after each minimum denomination change
            this->blind_levels[i].break_duration = 300000;
        }
        else
        {
            this->blind_levels[i].break_duration = 0;
        }

        // next small blind should be about factor times bigger than previous one
        ideal_small *= this->blind_increase_factor;
    }

    return this->blind_levels;
}
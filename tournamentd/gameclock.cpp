#include "gameclock.hpp"
#include "logger.hpp"
#include <algorithm>
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
    config.get_value("blind_levels", this->blind_levels);
    config.get_value("available_chips", this->available_chips);
}

// dump configuration to JSON
void gameclock::dump_configuration(json& config) const
{
    logger(LOG_DEBUG) << "Dumping game clock configuration\n";

    config.set_value("blind_increase_factor", this->blind_increase_factor);
    config.set_value("blind_levels", this->blind_levels);
    config.set_value("available_chips", this->available_chips);
}

// dump state to JSON
void gameclock::dump_state(json& state) const
{
    logger(LOG_DEBUG) << "Dumping game clock state\n";

    state.set_value("running", this->running);
    if(this->current_blind_level != 0)
    {
        state.set_value("current_blind_level", this->current_blind_level);
    }
    if(this->end_of_round != td::tp())
    {
        state.set_value("time_remaining", this->time_remaining.count());
    }
    if(this->end_of_break != td::tp())
    {
        state.set_value("break_time_remaining", this->break_time_remaining.count());
    }
    if(this->end_of_action_clock != td::tp())
    {
        state.set_value("action_clock_remaining", this->action_clock_remaining.count());
    }
}

// utility: start a blind level (optionally starting offset ms into the round)
void gameclock::start_blind_level(std::size_t blind_level, td::ms offset)
{
    if(blind_level >= this->blind_levels.size())
    {
        throw td::runtime_error("not enough blind levels configured");
    }

    auto now(std::chrono::system_clock::now());

    this->current_blind_level = blind_level;
    this->time_remaining = td::ms(this->blind_levels[this->current_blind_level].duration) - offset;
    this->break_time_remaining = td::ms(this->blind_levels[this->current_blind_level].break_duration);
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
    this->start_blind_level(1, td::ms::zero());

    // start the tournament
    this->running = true;
}

void gameclock::start(const td::tp& starttime)
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
}

// stop the game
void gameclock::stop()
{
    if(!this->is_started())
    {
        throw td::runtime_error("tournament not started");
    }

    logger(LOG_DEBUG) << "Stopping the tournament\n";

    this->running = false;
    this->current_blind_level = 0;
    this->end_of_round = td::tp();
    this->end_of_break = td::tp();
    this->end_of_action_clock = td::tp();
    this->time_remaining = td::ms::zero();
    this->break_time_remaining = td::ms::zero();
    this->action_clock_remaining = td::ms::zero();
}

// toggle pause
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

// toggle resume
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

// advance to next blind level
bool gameclock::next_blind_level(td::ms offset)
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
bool gameclock::previous_blind_level(td::ms offset)
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
        if(this->end_of_action_clock != td::tp())
        {
            this->action_clock_remaining = std::chrono::duration_cast<td::ms>(this->end_of_action_clock - now);
        }
        else
        {
            this->action_clock_remaining = td::ms::zero();
        }

        // set time remaining based on current clock
        if(now < this->end_of_round)
        {
            // within round, set time remaining
            this->time_remaining = std::chrono::duration_cast<td::ms>(this->end_of_round - now);
        }
        else if(now < this->end_of_break)
        {
            // within break, set time remaining to zero and set break time remaining
            this->time_remaining = td::ms::zero();
            this->break_time_remaining = std::chrono::duration_cast<td::ms>(this->end_of_break - now);
        }
        else
        {
            // advance to next blind
            auto offset(std::chrono::duration_cast<td::ms>(this->end_of_break - now));
            this->next_blind_level(offset);
        }
        return true;
    }
    return false;
}

// set the action clock (when someone 'needs the clock called on them'
void gameclock::set_action_clock(long duration)
{
    if(this->end_of_action_clock != td::tp())
    {
        this->end_of_action_clock = std::chrono::system_clock::now() + td::ms(duration);
        this->action_clock_remaining = td::ms(duration);
    }
    else
    {
        throw td::runtime_error("only one action clock at a time");
    }
}

// reset the action clock
void gameclock::reset_action_clock()
{
    this->end_of_action_clock = td::tp();
    this->action_clock_remaining = td::ms::zero();
}

static std::size_t calculate_round_denomination(double ideal_small, const std::vector<td::chip>& chips)
{
    // round to denomination n if ideal small blind is at least 10x denomination n-1
    static const std::size_t multiplier(10);

    auto it(chips.rbegin());
    while(std::next(it) != chips.rend())
    {
        std::size_t candidate(it->denomination);
        std::advance(it, 1);
        std::size_t limit(it->denomination);

        logger(LOG_DEBUG) << "ideal_small: " << ideal_small << ", candidate: " << candidate << ", limitx10:" << limit * multiplier << '\n';

        if(ideal_small > limit * multiplier)
        {
            return candidate;
        }
    }

    return it->denomination;
}

// generate progressive blind levels, given available chip denominations
// this was tuned to produce a sensible result for the following chip denomination sets:
//  1/5/25/100/500
//  5/25/100/500/1000
//  25/100/500/1000/5000
void gameclock::gen_blind_levels(std::size_t count, long level_duration)
{
    if(this->available_chips.empty())
    {
        throw td::runtime_error("tried to create a blind structure without chips defined");
    }

    // resize structure
    this->blind_levels.resize(count+1);

    // dummy round for planning phase
    this->blind_levels[0] = {0};

    // sort our vector
    std::sort(this->available_chips.begin(), this->available_chips.end(), [](const td::chip& c0, const td::chip& c1) { return c0.denomination < c1.denomination; });

    // starting small blind = smallest denomination
    double ideal_small(static_cast<double>(this->available_chips.begin()->denomination));

    // store last round denomination (to check when it changes)
    std::size_t last_round_denom(0);

    for(auto i(1); i<count+1; i++)
    {
        // calculate nearest chip denomination to round to
        const auto round_denom(calculate_round_denomination(ideal_small, this->available_chips));
        const auto little_blind(std::ceil(ideal_small / round_denom) * round_denom);

        logger(LOG_DEBUG) << "round: " << i << ", little blind will be: " << static_cast<std::size_t>(little_blind) << '\n';

        // round up
        this->blind_levels[i].little_blind = little_blind;
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
}
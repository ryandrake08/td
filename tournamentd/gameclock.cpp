#include "gameclock.hpp"
#include "logger.hpp"

// load configuration from JSON (object or file)
void gameclock::configure(const json& config)
{
    logger(LOG_DEBUG) << "Loading game clock configuration\n";

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
            return this->advance_blind_level(offset);
        }
    }
    return false;
}

#pragma once
#include <chrono>

template<typename T>
class basic_stopwatch
{
    // start and end points
    typename T::time_point begin_time;
    typename T::time_point last_time;

public:
    // construction
    basic_stopwatch() : begin_time(T::now()), last_time(begin_time) {}

    // get total elapsed time since construction
    typename T::duration elapsed() const
    {
        auto now(T::now());
        auto dur(now - this->begin_time);
        return dur;
    }

    // duration_cast helper
    template<typename D>
    std::chrono::duration<D> elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::duration<D>>(elapsed());
    }

    // get elapsed time since construction or since last calling lap
    typename T::duration lap()
    {
        auto now(T::now());
        auto dur(now - this->last_time);
        this->last_time = now;
        return dur;
    }

    // duration_cast helper
    template<typename D>
    std::chrono::duration<D> lap()
    {
        return std::chrono::duration_cast<std::chrono::duration<D>>(lap());
    }
};

typedef basic_stopwatch<std::chrono::steady_clock> stopwatch;

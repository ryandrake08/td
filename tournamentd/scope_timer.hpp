#pragma once
#include "logger.hpp"
#include "stopwatch.hpp"
#include <string>

class scope_timer
{
    // stopwatch for counting elapsed time
    stopwatch sw;

    // log message
    std::string log_message;

public:
    // destruction (will log when object goes out of scope)
    ~scope_timer()
    {
        // log
        std::chrono::duration<long long, std::nano> duration(this->sw.elapsed());
        logger(ll::debug) << this->log_message << duration.count() << " nanoseconds\n";
    }

    // attributes
    void set_message(const std::string& message)
    {
        this->log_message = message;
    }
};

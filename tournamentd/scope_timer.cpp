//
//  scope_timer.cpp
//  td
//
//  Created by Ryan Drake on 9/15/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#include "scope_timer.hpp"
#include "logger.hpp"
#include <chrono>

struct scope_timer::impl
{
    // start and end points
    std::chrono::high_resolution_clock::time_point begin_time;
    std::chrono::high_resolution_clock::time_point end_time;

    // log message
    std::string log_message;

    impl() : begin_time(std::chrono::high_resolution_clock::now())
    {
    }

    impl(const std::string& message) : begin_time(std::chrono::high_resolution_clock::now()), log_message(message)
    {
    }

    ~impl()
    {
        // mark end of timer
        this->end_time = std::chrono::high_resolution_clock::now();

        // log
        std::chrono::duration<long long, std::nano> duration(this->end_time - this->begin_time);
        logger(LOG_DEBUG) << this->log_message << duration.count() << " nanoseconds\n";
    }
};

scope_timer::scope_timer() : pimpl(new impl)
{
}

scope_timer::scope_timer(const std::string& message) : pimpl(new impl(message))
{
}

scope_timer::~scope_timer() = default;

void scope_timer::set_message(const std::string& message)
{
    this->pimpl->log_message = message;
}

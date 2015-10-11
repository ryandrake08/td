//
//  scope_timer.cpp
//  td
//
//  Created by Ryan Drake on 9/15/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#include "scope_timer.hpp"
#include "logger.hpp"

#if defined(__clang__)
#include "chrono/chrono_io"
#endif

scope_timer::scope_timer() : begin_time(std::chrono::high_resolution_clock::now())
{
}

scope_timer::scope_timer(const std::string& message) : begin_time(std::chrono::high_resolution_clock::now()), log_message(message)
{
}

scope_timer::~scope_timer()
{
    // mark end of timer
    end_time = std::chrono::high_resolution_clock::now();

    // log
#if defined(__clang__)
    logger(LOG_DEBUG) << this->log_message << (this->end_time - this->begin_time) << '\n';
#else
    logger(LOG_DEBUG) << this->log_message << "scope_timer only available when built using clang\n";
#endif
}

void scope_timer::set_message(const std::string& message)
{
    this->log_message = message;
}

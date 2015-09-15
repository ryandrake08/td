//
//  scope_timer.cpp
//  td
//
//  Created by Ryan Drake on 9/15/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#include "scope_timer.hpp"
#include "logger.hpp"
#include "chrono/chrono_io"

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
    logger(LOG_DEBUG) << this->log_message << (this->end_time - this->begin_time) << '\n';
}

void scope_timer::set_message(const std::string& message)
{
    this->log_message = message;
}

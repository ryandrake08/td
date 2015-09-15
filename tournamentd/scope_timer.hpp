#pragma once
#include <chrono>
#include <string>

class scope_timer
{
    // start and end points
    std::chrono::high_resolution_clock::time_point begin_time;
    std::chrono::high_resolution_clock::time_point end_time;

    // log message
    std::string log_message;

public:
    // construction
    scope_timer();
    explicit scope_timer(const std::string& message);

    // destruction (will log when object goes out of scope)
    ~scope_timer();

    // attributes
    void set_message(const std::string& message);
};
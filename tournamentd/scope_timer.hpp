#pragma once
#include <memory>
#include <string>

class scope_timer
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    // construction
    scope_timer();
    explicit scope_timer(const std::string& message);

    // destruction (will log when object goes out of scope)
    ~scope_timer();

    // attributes
    void set_message(const std::string& message);
};
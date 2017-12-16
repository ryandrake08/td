#pragma once
#include <memory>

class bonjour_publisher
{
    class impl;

    std::unique_ptr<impl> pimpl;

public:
    // empty destructor, so we can use unique_ptr above
    bonjour_publisher();
    ~bonjour_publisher();

    void publish(const char* name, int port);
};
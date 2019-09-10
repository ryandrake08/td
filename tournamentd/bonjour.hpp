#pragma once
#include <memory>
#include <string>

class bonjour_publisher
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    bonjour_publisher();
    ~bonjour_publisher();

    void publish(const std::string& name, int port);
};

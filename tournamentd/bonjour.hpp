#pragma once
#include <memory>
#include <string>

class bonjour_impl;

class bonjour_publisher
{
    std::unique_ptr<bonjour_impl> impl;

public:
    // empty destructor, so we can use unique_ptr above
    bonjour_publisher();
    ~bonjour_publisher();

    void publish(const std::string& name, int port);
};
#pragma once
#include <memory>
#include <string>

class bonjour_impl;

class bonjour_publisher
{
    std::shared_ptr<bonjour_impl> impl;

public:
    void publish(const std::string& name, int port);
};
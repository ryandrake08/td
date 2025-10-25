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

    // Non-copyable, non-movable (manages unique resources)
    bonjour_publisher(const bonjour_publisher&) = delete;
    bonjour_publisher& operator=(const bonjour_publisher&) = delete;
    bonjour_publisher(bonjour_publisher&&) = delete;
    bonjour_publisher& operator=(bonjour_publisher&&) = delete;

    void publish(const std::string& name, int port);
};

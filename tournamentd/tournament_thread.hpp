#pragma once

#include "tournament_service.hpp"

#include <memory>
#include <string>

class tournament_thread
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    tournament_thread();
    ~tournament_thread();

    // start the daemon, pre-authorizing given client code, returning service
    tournament_service start(int code);

    // publish over Bojour using name
    void publish(const std::string& name);

    // stop the daemon (and stop publishing if doing so)
    void stop();
};

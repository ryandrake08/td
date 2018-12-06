#pragma once

#include "TournamentService.hpp"

#include <memory>
#include <string>

class TournamentDaemon
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    TournamentDaemon();
    ~TournamentDaemon();

    // start the daemon, pre-authorizing given client code, returning service
    TournamentService start(int code);

    // publish over Bojour using name
    void publish(const std::string& name);

    // stop the daemon (and stop publishing if doing so)
    void stop();
};

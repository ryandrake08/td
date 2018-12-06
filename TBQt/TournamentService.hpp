#pragma once

#include <memory>
#include <string>

class TournamentService
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    // constants
    static constexpr const char* type = "_pokerbuddy._tcp.";
    static constexpr const char* domain = "local.";
    static const int default_port = 25600;

    // construct from address and port
    TournamentService(const std::string& address, int port);

    // construct from unix socket path
    TournamentService(const std::string& path);

    // construct from bonjour service
    // TODO: tournament_service(const ???& net_service);

    // move constructor
    TournamentService(TournamentService&& other);

    // destructor
    ~TournamentService();

    // is this a remote service?
    bool is_remote() const;

    // service name
    std::string name() const;

    // retrieve the bonjour service, if applicable
    // TODO: ??? net_service() const;
};

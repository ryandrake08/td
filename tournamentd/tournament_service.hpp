#pragma once

#include <memory>
#include <string>

class tournament_service
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
    tournament_service(const std::string& address, int port);

    // construct from unix socket path
    tournament_service(const std::string& path);

    // construct from bonjour service
    // TODO: tournament_service(const ???& net_service);

    // move constructor
    tournament_service(tournament_service&& other);

    // destructor
    ~tournament_service();

    // is this a remote service?
    bool is_remote() const;

    // service name
    std::string name() const;

    // retrieve the bonjour service, if applicable
    // TODO: ??? net_service() const;
};

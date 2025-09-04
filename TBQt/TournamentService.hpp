#pragma once

#include <memory>
#include <string>

namespace QMdnsEngine
{
    class Service;
}

class TournamentService
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    // constants
    static constexpr const char* type = "_pokerbuddy._tcp.local.";
    static const int default_port = 25600;

    // construct from address and port
    TournamentService(const std::string& address, int port);

    // construct from unix socket path
    TournamentService(const std::string& path);

    // construct from qmdnsengine service
    TournamentService(const QMdnsEngine::Service& service);

    // move constructor
    TournamentService(TournamentService&& other);

    // destructor
    ~TournamentService();

    // is this a remote service?
    bool is_remote() const;

    // service attributes
    std::string path() const;
    std::string address() const;
    int port() const;
    std::string name() const;

    // comparison operators
    bool operator==(const TournamentService& other) const;
    bool operator==(const QMdnsEngine::Service& service) const;
};

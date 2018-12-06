#include "TournamentService.hpp"

struct TournamentService::impl
{
    std::string path;
    std::string address;
    int port;
    std::string name;
};

// construct from address and port
TournamentService::TournamentService(const std::string& address, int port) : pimpl(new impl)
{
    this->pimpl->address = address;
    this->pimpl->port = port;
    this->pimpl->name = address + ":" + std::to_string(port);
}

// construct from unix socket path
TournamentService::TournamentService(const std::string& path) : pimpl(new impl)
{
    this->pimpl->path = path;
    this->pimpl->name = path.substr(path.find_last_of("/\\") + 1);
}

// default move constructor
TournamentService::TournamentService(TournamentService&& other) = default;

// default destructor
TournamentService::~TournamentService() = default;

// is this a remote service?
bool TournamentService::is_remote() const
{
    return this->pimpl->path.empty();
}

// service name
std::string TournamentService::name() const
{
    return this->pimpl->name;
}

#include "tournament_service.hpp"

struct tournament_service::impl
{
    std::string path;
    std::string address;
    int port;
    std::string name;
};

// construct from address and port
tournament_service::tournament_service(const std::string& address, int port) : pimpl(new impl)
{
    this->pimpl->address = address;
    this->pimpl->port = port;
    this->pimpl->name = address + ":" + std::to_string(port);
}

// construct from unix socket path
tournament_service::tournament_service(const std::string& path) : pimpl(new impl)
{
    this->pimpl->path = path;
    this->pimpl->name = path.substr(path.find_last_of("/\\") + 1);
}

// default move constructor
tournament_service::tournament_service(tournament_service&& other) = default;

// default destructor
tournament_service::~tournament_service() = default;

// is this a remote service?
bool tournament_service::is_remote() const
{
    return this->pimpl->path.empty();
}

// service name
std::string tournament_service::name() const
{
    return this->pimpl->name;
}

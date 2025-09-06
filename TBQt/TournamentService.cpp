#include "TournamentService.hpp"

#include <qmdnsengine/service.h>

#include <QString>

struct TournamentService::impl
{
    std::string path;
    std::string address;
    int port {};
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

// construct from qmdnsengine service
TournamentService::TournamentService(const QMdnsEngine::Service& service) : pimpl(new impl)
{
    this->pimpl->address = service.hostname().toStdString();
    this->pimpl->port = service.port();
    this->pimpl->name = QString::fromUtf8(service.name()).toStdString();
}

// default move constructor
TournamentService::TournamentService(TournamentService&& other) noexcept = default;

// default destructor
TournamentService::~TournamentService() = default;

// is this a remote service?
bool TournamentService::is_remote() const
{
    return this->pimpl->path.empty();
}

// service path
std::string TournamentService::path() const
{
    return this->pimpl->path;
}

// service address
std::string TournamentService::address() const
{
    return this->pimpl->address;
}

// service port
int TournamentService::port() const
{
    return this->pimpl->port;
}

// service name
std::string TournamentService::name() const
{
    return this->pimpl->name;
}

// comparison operators
bool TournamentService::operator==(const TournamentService& other) const
{
    if(this->is_remote() != other.is_remote())
    {
        return false;
    }

    if(this->is_remote())
    {
        return this->pimpl->address == other.pimpl->address && this->pimpl->port == other.pimpl->port;
    }
    else
    {
        return this->pimpl->path == other.pimpl->path;
    }
}

bool TournamentService::operator==(const QMdnsEngine::Service& service) const
{
    if(!this->is_remote())
    {
        return false;
    }

    return this->pimpl->name == QString::fromUtf8(service.name()).toStdString() &&
           this->pimpl->address == service.hostname().toStdString() &&
           this->pimpl->port == service.port();
}

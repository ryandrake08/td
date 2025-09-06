#include "server.hpp"
#include "logger.hpp"
#include "socket.hpp"
#include "socketstream.hpp"
#include <set>

struct server::impl
{
    std::set<common_socket> all;
    std::set<common_socket> listeners;
    std::set<common_socket> clients;
};

server::server() : pimpl(new impl)
{
}

server::~server() = default;

// listen for clients on given unix socket path and optional internet service
void server::listen(const char* unix_socket_path, const char* inet_service)
{
    // erase any existing listeners
    for(const auto& it : this->pimpl->listeners)
    {
        this->pimpl->all.erase(it);
    }

    // clear set of iterators
    this->pimpl->listeners.clear();

    if(inet_service != nullptr)
    {
        // add socket to listen for ipv4
        inet4_socket sock4(inet_service);
        this->pimpl->all.insert(sock4);
        this->pimpl->listeners.insert(sock4);

        // add socket to listen for ipv6
        inet6_socket sock6(inet_service);
        this->pimpl->all.insert(sock6);
        this->pimpl->listeners.insert(sock6);
    }

    if(unix_socket_path != nullptr)
    {
        // add unix socket
        unix_socket socku(unix_socket_path);
        this->pimpl->all.insert(socku);
        this->pimpl->listeners.insert(socku);
    }
}

// close client connection
void server::close(const common_socket& sock)
{
    this->pimpl->clients.erase(sock);
    this->pimpl->all.erase(sock);
}

// poll the server with given timeout
bool server::poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec)
{
    auto selected(common_socket::select(this->pimpl->all, usec));

    // handle each selected socket
    auto it(selected.begin());
    while(it != selected.end())
    {
        auto sock(it++);

        if(this->pimpl->listeners.find(*sock) != this->pimpl->listeners.end())
        {
            logger(ll::info) << "new client connection\n";

            // accept new client from listening socket
            const auto& client(sock->accept());

            // greet new client
            socketstream ss(client);
            if(!handle_new_client(ss) && ss.good())
            {
                // if all is well, add to our lists
                this->pimpl->clients.insert(client);
                this->pimpl->all.insert(client);
            }
        }
        else
        {
            logger(ll::debug) << "handling client communication\n";

            // handle client i/o
            socketstream ss(*sock);
            auto in_avail(ss.rdbuf()->in_avail());
            while(in_avail > 0 && ss.good())
            {
                if(handle_client(ss))
                {
                    logger(ll::info) << "closing client connection gracefully\n";
                    this->close(*sock);
                }

                // keep checking until no pending io
                in_avail = ss.rdbuf()->in_avail();
            }

            if(!ss.good())
            {
                logger(ll::info) << "closing client connection: stream error\n";
                this->close(*sock);
            }
            else if(in_avail < 0)
            {
                logger(ll::info) << "closing client connection: no input available\n";
                this->close(*sock);
            }
        }
    }

    return false;
}

// broadcast message to all clients
void server::broadcast(const std::string& message) const
{
    for(const auto& client : this->pimpl->clients)
    {
        // handle client i/o
        socketstream ss(client);
        ss << message << std::endl;
    }
}

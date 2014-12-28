#include "json.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "socketstream.hpp"
#include <sstream>

// create the server, listening on given port
server::server(std::uint16_t port) : listener(port)
{
    this->all_open.insert(listener);
}

// disconnect a client
void server::disconnect(const inet_socket& client)
{
    logger(LOG_DEBUG) << "closing client connection\n";
    this->all_open.erase(client);
    this->all_clients.erase(client);
}

// poll the server with given timeout
bool server::poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec)
{
    auto selected(inet_socket::select(this->all_open, usec));

    // first, service the listening socket, if selected
    if(selected.find(this->listener) != selected.end())
    {
        logger(LOG_DEBUG) << "new client connection\n";

        // accept new client from listening socket
        auto client(this->listener.accept());

        // greet new client
        socketstream ss(client);
        if(!handle_new_client(ss) && ss.good())
        {
            // if all is well, add to our lists, and erase from our clients-to-be-handled list
            this->all_open.insert(client);
            this->all_clients.insert(client);
            selected.erase(this->listener);
        }
    }

    // then, handle each existing client, if selected
    for(auto c : selected)
    {
        logger(LOG_DEBUG) << "handling client communication\n";

        // handle client i/o
        socketstream ss(c);
        if(handle_client(ss) || !ss.good())
        {
            disconnect(c);
        }
    }

    return false;
}

// call back handler for each client
void server::each_client(const std::function<bool(std::ostream&)>& handler)
{
    for(auto c : this->all_clients)
    {
        // handle client i/o
        socketstream ss(c);
        if(handler(ss) || !ss.good())
        {
            disconnect(c);
        }
    }
}
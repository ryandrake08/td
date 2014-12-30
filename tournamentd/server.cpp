#include "server.hpp"
#include "logger.hpp"
#include "socketstream.hpp"

// listen on given port
void server::listen(std::uint16_t port)
{
    // erase any existing listeners
    for(auto it : this->listeners)
    {
        this->all.erase(it);
    }

    // clear set of iterators
    this->listeners.clear();

    // create new socket
    auto sock((inet_socket(port)));

    // add it to both sets
    this->all.insert(sock);
    this->listeners.insert(sock);
}

// poll the server with given timeout
bool server::poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec)
{
    auto selected(inet_socket::select(this->all, usec));

    // handle each selected socket
    auto it(selected.begin());
    while(it != selected.end())
    {
        auto sock(it++);

        if(this->listeners.find(*sock) != this->listeners.end())
        {
            logger(LOG_DEBUG) << "new client connection\n";

            // accept new client from listening socket
            auto client(sock->accept());

            // greet new client
            socketstream ss(client);
            if(!handle_new_client(ss) && ss.good())
            {
                // if all is well, add to our lists
                this->clients.insert(client);
                this->all.insert(client);
            }
        }
        else
        {
            logger(LOG_DEBUG) << "handling client communication\n";

            auto client(*sock);

            // handle client i/o
            socketstream ss(*sock);
            if(handle_client(ss) || !ss.good())
            {
                logger(LOG_DEBUG) << "closing client connection\n";
                this->clients.erase(client);
                this->all.erase(client);
            }
        }
    }

    return false;
}

// broadcast message to all clients
void server::broadcast(const std::string& message) const
{
    for(auto client : this->clients)
    {
        // handle client i/o
        socketstream ss(client);
        ss << message;
    }
}

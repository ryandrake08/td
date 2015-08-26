#include "server.hpp"
#include "logger.hpp"
#include "socketstream.hpp"

// listen for clients on given unix socket path and optional internet service
void server::listen(const char* unix_socket_path, const char* inet_service)
{
    // erase any existing listeners
    for(auto& it : this->listeners)
    {
        this->all.erase(it);
    }

    // clear set of iterators
    this->listeners.clear();

    if(inet_service != nullptr)
    {
#if 0
        // add socket to listen for ipv4 (unnecessary on systems supporting dual-stack sockets)
        inet4_socket sock4(inet_service);
        this->all.insert(sock4);
        this->listeners.insert(sock4);
#endif

        // add socket to listen for ipv6
        inet6_socket sock6(inet_service);
        this->all.insert(sock6);
        this->listeners.insert(sock6);
    }

#if !defined(_WIN32)
    // add unix socket
    unix_socket socku(unix_socket_path);
    this->all.insert(socku);
    this->listeners.insert(socku);
#endif
}

// poll the server with given timeout
bool server::poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec)
{
    auto selected(common_socket::select(this->all, usec));

    // handle each selected socket
    auto it(selected.begin());
    while(it != selected.end())
    {
        auto sock(it++);

        if(this->listeners.find(*sock) != this->listeners.end())
        {
            logger(LOG_INFO) << "new client connection\n";

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
            logger(LOG_INFO) << "handling client communication\n";

            // handle client i/o
            socketstream ss(*sock);
            if(handle_client(ss) || !ss.good())
            {
                logger(LOG_INFO) << "closing client connection\n";
                this->clients.erase(*sock);
                this->all.erase(*sock);
            }
        }
    }

    return false;
}

// broadcast message to all clients
void server::broadcast(const std::string& message) const
{
    for(auto& client : this->clients)
    {
        // handle client i/o
        socketstream ss(client);
        ss << message << std::endl;
    }
}

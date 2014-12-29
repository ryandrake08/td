#include "server.hpp"
#include "logger.hpp"
#include "socketstream.hpp"
#include <sstream>

server::server() : listener(all.end())
{
}

// listen on given port
void server::listen(std::uint16_t port)
{
    if(this->listener != all.end())
    {
        // destroy old socket
        this->all.erase(this->listener);
    }

    // create new socket
    this->listener = this->all.insert(inet_socket(port)).first;
}

// poll the server with given timeout
bool server::poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec)
{
    auto selected(inet_socket::select(this->all, usec));

    // handle each selected socket
    for(auto it(selected.begin()); it != selected.end(); it++)
    {
        if(*it == *this->listener)
        {
            logger(LOG_DEBUG) << "new client connection\n";

            // accept new client from listening socket
            auto client(it->accept());

            // greet new client
            socketstream ss(client);
            if(!handle_new_client(ss) && ss.good())
            {
                // if all is well, add to our lists, and erase from our clients-to-be-handled list
                this->all.insert(client);
            }
        }
        else
        {
            logger(LOG_DEBUG) << "handling client communication\n";

            // handle client i/o
            socketstream ss(*it);
            if(handle_client(ss) || !ss.good())
            {
                logger(LOG_DEBUG) << "closing client connection\n";
                this->all.erase(*it);
            }
        }
    }

    return false;
}

// call back handler for each client
void server::each_client(const std::function<bool(std::ostream&)>& handler)
{
    for(auto it(this->all.begin()); it != this->all.end(); it++)
    {
        if(*it != *this->listener)
        {
            // handle client i/o
            socketstream ss(*it);
            if(handler(ss) || !ss.good())
            {
                logger(LOG_DEBUG) << "closing client connection\n";
                it = this->all.erase(it);
            }
            else
            {
                it++;
            }
        }
        else
        {
            it++;
        }
    }
}
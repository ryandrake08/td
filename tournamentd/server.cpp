#include "json.hpp"
#include "logger.hpp"
#include "server.hpp"
#include "socketstream.hpp"

// create the server, listening on given port
server::server(std::uint16_t port) : listener(port)
{
    this->all_open.insert(listener);
}

// poll the server with given timeout
void server::poll(long usec)
{
    auto selected(inet_socket::select(this->all_open));

    // first, service the listening socket, if selected
    if(selected.find(this->listener) != selected.end())
    {
        // accept and add to our 'all' set
        auto client(this->listener.accept());
        this->all_open.insert(client);
        selected.erase(this->listener);

        // greet client
        json greeting;
        greeting.set_value("ready", true);
        socketstream(client) << greeting;
    }

    // then, service the remaining (clients)
    for(auto client : selected)
    {
        std::string buffer;
        std::getline(socketstream(client), buffer);
        logger(LOG_DEBUG) << "received: " << buffer << '\n';
    }
}

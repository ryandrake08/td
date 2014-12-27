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

// poll the server with given timeout
bool server::poll(const std::function<void(std::iostream&)>& handle_new_client, const std::function<void(std::iostream&)>& handle_client, long usec)
{
    auto selected(inet_socket::select(this->all_open, usec));

    // first, service the listening socket, if selected
    if(selected.find(this->listener) != selected.end())
    {
        // accept and add to our 'all' set
        auto client(this->listener.accept());
        this->all_open.insert(client);
        this->all_clients.insert(client);
        selected.erase(this->listener);

        socketstream ss(client);
        handle_new_client(ss);
    }

    // then, handle each existing client, if selected
    for(auto c : selected)
    {
        socketstream ss(c);
        handle_client(ss);
    }

    return false;
}

// call back handler for each client
void server::each_client(const std::function<void(std::ostream&)>& handler)
{
    for(auto c : this->all_clients)
    {
        socketstream ss(c);
        handler(ss);
    }
}
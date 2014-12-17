#include "program.hpp"
#include "logger.hpp"
#include <iostream>
#include <system_error>

#include "socketstream.hpp"

program::program(const std::vector<std::string>& cmdline) : sv_sock(25600)
{
    this->sockets.insert(sv_sock);
}

bool program::run()
{
    auto ready(inet_socket::select(this->sockets));
    for(auto s : ready)
    {
        if(s == this->sv_sock)
        {
            auto new_socket(inet_socket(this->sv_sock.accept()));
            socketstream ss(new_socket);
            ss << "hello\n";
            this->sockets.insert(new_socket);
        }
        else
        {
            socketstream ss(s);
            std::string buf;
            ss >> buf;
            ss << buf;
        }
    }

    return false;
}
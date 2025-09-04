#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <string>

class common_socket;

class server
{
    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // close client connection
    void close(const common_socket& sock);

public:
    server();
    ~server();

    // listen on given unix socket path and optional internet service
    void listen(const char* unix_socket_path, const char* inet_service = nullptr);

    // poll the server with given timeout, handling both new clients and clients with input
    bool poll(const std::function<bool(std::ostream&)>& handle_new_client, const std::function<bool(std::iostream&)>& handle_client, long usec = -1);

    // broadcast message to all clients
    void broadcast(const std::string& message) const;
};

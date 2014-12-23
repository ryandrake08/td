#pragma once
#include <cstddef> // for size_t
#include <cstdint>
#include <memory>
#include <set>

struct inet_socket_impl;

class inet_socket
{
    std::shared_ptr<inet_socket_impl> impl;

    // ensure socket has a valid impl
    void validate() const;

public:
    // create an uninitialized socket
    inet_socket();

    // create a socket with a given impl
    inet_socket(inet_socket_impl* imp);

    // create and connect socket and connect to an address at a port
    inet_socket(std::uint32_t addr, std::uint16_t port);

    // create and connect socket and connect to a host at a port
    inet_socket(const char* host, std::uint16_t port);

    // create a listening socket by binding to a port
    inet_socket(std::uint16_t port, int backlog=5);

    // create a new socket by accepting on a listening socket
    inet_socket accept();

    // select on this socket
    bool select(long usec=-1);

    // select on multiple sockets
    static std::set<inet_socket> select(const std::set<inet_socket>& sockets, long usec=-1);

    // data transfer
    std::size_t recv(void* buf, std::size_t bytes);
    std::size_t send(const void* buf, std::size_t bytes);

    // various operators
    bool operator<(const inet_socket& other) const;
    bool operator==(const inet_socket& other) const;

    // Stream insertion
    friend std::ostream& operator<<(std::ostream& os, const inet_socket& sock);
};

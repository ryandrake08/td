#pragma once
#include <cstddef> // for size_t
#include <cstdint>
#include <memory>
#include <set>

struct common_socket_impl;

class common_socket
{
protected:
    std::shared_ptr<common_socket_impl> impl;

    // ensure socket has a valid impl
    void validate() const;

    // empty constructor
    common_socket();

    // create a socket with a given impl
    common_socket(common_socket_impl* imp);

public:
    // create a new socket by accepting on a listening socket
    common_socket accept() const;

    // select on this socket
    bool select(long usec=-1);

    // select on multiple sockets
    static std::set<common_socket> select(const std::set<common_socket>& sockets, long usec=-1);

    // data transfer
    std::size_t recv(void* buf, std::size_t bytes);
    std::size_t send(const void* buf, std::size_t bytes);

    // is socket listening
    bool listening() const;

    // various operators
    bool operator<(const common_socket& other) const;
    bool operator==(const common_socket& other) const;
    bool operator!=(const common_socket& other) const;

    // Stream insertion
    friend std::ostream& operator<<(std::ostream& os, const common_socket& sock);
};

class inet_socket : public common_socket
{
public:
    // create and connect socket and connect to a host at a port
    inet_socket(const char* host, const char* service, int family);

    // create a listening socket by binding to a port
    inet_socket(const char* service, int family, int backlog);
};

class inet4_socket : public inet_socket
{
public:
    // create and connect socket and connect to a host at a port
    inet4_socket(const char* host, const char* service);

    // create a listening socket by binding to a port
    inet4_socket(const char* service, int backlog=5);
};

class inet6_socket : public inet_socket
{
public:
    // create and connect socket and connect to a host at a port
    inet6_socket(const char* host, const char* service);

    // create a listening socket by binding to a port
    inet6_socket(const char* service, int backlog=5);
};

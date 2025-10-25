#pragma once
#include <cstddef> // for size_t
#include <cstdint>
#include <memory>
#include <set>

class socket_initializer;

class common_socket
{
    std::shared_ptr<socket_initializer> socket_subsystem;

protected:
    struct impl;
    std::shared_ptr<impl> pimpl;

    // empty constructor
    common_socket();

    // create a socket with a given impl (needed for accept)
    explicit common_socket(impl* imp);

public:
    // destructor
    ~common_socket();

    // create a new socket by accepting on a listening socket
    common_socket accept() const;

    // select on multiple sockets
    static std::set<common_socket> select(const std::set<common_socket>& sockets, long usec = -1);

    // does socket have data available
    long peek(void* buf, std::size_t bytes) const;

    // data transfer
    long recv(void* buf, std::size_t bytes);
    long send(const void* buf, std::size_t bytes);

    // is socket listening
    bool listening() const;

    // various operators
    bool operator<(const common_socket& other) const;
    bool operator==(const common_socket& other) const;
    bool operator!=(const common_socket& other) const;

    // Stream insertion
    friend std::ostream& operator<<(std::ostream& os, const common_socket& sock);
};

class unix_socket : public common_socket
{
public:
    // create a unix socket by either connecting or binding/listening
    explicit unix_socket(const char* path, bool connecting = false);
};

class inet_socket : public common_socket
{
public:
    // create an inet socket by either connecting or binding/listening
    inet_socket(const char* host, const char* service, int family, bool connecting);
};

class inet4_socket : public inet_socket
{
public:
    // create an inet4 socket by either connecting or binding/listening
    inet4_socket(const char* host, const char* service, bool connecting = false);
};

class inet6_socket : public inet_socket
{
public:
    // create an inet6 socket by either connecting or binding/listening
    inet6_socket(const char* host, const char* service, bool connecting = false);
};

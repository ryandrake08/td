#include "socket.hpp"
#include "logger.hpp"
#include <algorithm>
#include <system_error>
#include <cerrno> // for errno
#include <cassert>
#include <iterator>

#if defined(_WIN32)
#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winsock2.h>
typedef int socklen_t;
#endif
#if defined(__unix) || defined(__APPLE__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
typedef int SOCKET;
static const SOCKET INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;
#endif

// initializer
static struct socket_initializer
{
    socket_initializer()
    {
#if defined(_WIN32)
        WORD ver(MAKEWORD(1,1));
        WSADATA data;
        auto res = WSAStartup(ver, &data);
        assert(res == 0);
        assert(LOBYTE(data.wVersion) == 1 && HIBYTE(data.wVersion) == 1);
#endif
    }

    ~socket_initializer()
    {
#if defined(_WIN32)
        WSACleanup();
#endif
    }

    // No copy constructors/assignment
    socket_initializer(const socket_initializer& other) = delete;
    socket_initializer& operator=(const socket_initializer& other) = delete;

} initializer;

// pimpl (fd wrapper)

struct inet_socket_impl
{
    SOCKET fd;

    // construct with given fd
    inet_socket_impl(SOCKET newfd) : fd(newfd)
    {
        // check socket
        if(newfd == INVALID_SOCKET)
        {
            throw(std::system_error(errno, std::system_category(), "inet_socket_impl: invalid socket"));
        }

        logger(LOG_DEBUG) << "created socket from fd: " << this->fd << '\n';
    }

    inet_socket_impl() : inet_socket_impl(static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0)))
    {
    }
    
    ~inet_socket_impl()
    {
        logger(LOG_DEBUG) << "closing socket: " << this->fd << '\n';
#if defined(_WIN32) // thank you, Microsoft
        ::closesocket(this->fd);
#else
        ::sync();
        ::close(this->fd);
#endif
    }
};

void inet_socket::validate() const
{
    if(!this->impl)
    {
        throw(std::logic_error("inet_socket: invalid socket impl"));
    }
}

// create a socket without a fd
inet_socket::inet_socket()
{
    logger(LOG_DEBUG) << "creating empty socket\n";
}

// create a socket with a given impl
inet_socket::inet_socket(inet_socket_impl* imp) : impl(imp)
{
    validate();

    logger(LOG_DEBUG) << "creating socket with impl: " << *this << '\n';
}

// create and connect socket and connect to an address at a port
inet_socket::inet_socket(std::uint32_t addr, std::uint16_t port) : impl(new inet_socket_impl)
{
    validate();

    sockaddr_in sad;
    sad.sin_family = AF_INET;
    sad.sin_port = htons(port);
    sad.sin_addr.s_addr = htonl(addr);

    logger(LOG_DEBUG) << "connecting " << *this << " to: " << addr << ':' << port << '\n';

    // connect to remote address
    if(::connect(this->impl->fd, reinterpret_cast<sockaddr*>(&sad), sizeof(sad)) == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: connect"));
    }
}

inet_socket::inet_socket(const char* host, std::uint16_t port) : impl(new inet_socket_impl)
{
    validate();

    logger(LOG_DEBUG) << "looking up host: " << host << '\n';

    // look up hostname
    hostent* he(::gethostbyname(host));
    if(he == nullptr)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: gethostbyname"));
    }

    sockaddr_in sad;
    sad.sin_family = AF_INET;
    sad.sin_port = htons(port);
    std::copy(reinterpret_cast<char*>(he->h_addr),
              reinterpret_cast<char*>(he->h_addr) + he->h_length,
              reinterpret_cast<char*>(&sad.sin_addr.s_addr));

    logger(LOG_DEBUG) << "connecting " << *this << " to: " << host << ':' << port << '\n';

    // connect to remote address
    if(::connect(this->impl->fd, reinterpret_cast<sockaddr*>(&sad), sizeof(sad)) == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: connect"));
    }
}

inet_socket::inet_socket(std::uint16_t port, int backlog) : impl(new inet_socket_impl)
{
    validate();

    // set SO_REUSADDR option
    int opt(1);
#if defined(_WIN32)
    if(::setsockopt(this->impl->fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR)
#else
    if(::setsockopt(this->impl->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == SOCKET_ERROR)
#endif
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: setsockopt"));
    }

    logger(LOG_DEBUG) << "binding " << *this << " to port: " << port << '\n';

    // bind to server port
    sockaddr_in sad;
    sad.sin_family = AF_INET;
    sad.sin_port = htons(port);
    sad.sin_addr.s_addr = INADDR_ANY;
    if(::bind(this->impl->fd, reinterpret_cast<sockaddr*>(&sad), sizeof(sad)) == SOCKET_ERROR) 
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: bind"));
    }

    logger(LOG_DEBUG) << "listening on " << *this << " with backlog: " << backlog << '\n';

    // begin listening on the socket
    if(::listen(this->impl->fd, backlog) == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: listen"));
    }
}

inet_socket inet_socket::accept()
{
    validate();

    logger(LOG_DEBUG) << "accepting from " << *this << '\n';

    sockaddr_in sad;
    socklen_t len;
    auto ret(::accept(this->impl->fd, reinterpret_cast<sockaddr*>(&sad), &len));
    if(ret == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: accept"));
    }

    return inet_socket(new inet_socket_impl(ret));
}

// select on this socket
bool inet_socket::select(long usec)
{
    validate();

    // timeout
    timeval tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;

    // handle infinite timeout
    timeval* ptv(usec < 0 ? nullptr : &tv);

    logger(LOG_DEBUG) << "selecting with timeout " << usec << " on " << *this << '\n';

    // single fd
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(this->impl->fd, &fds);

    // do the select
    auto ready(::select(this->impl->fd+1, &fds, nullptr, nullptr, ptv));
    if(ready == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket(S): select"));
    }

    return ready > 0;
}

// select on multiple sockets
std::set<inet_socket> inet_socket::select(const std::set<inet_socket>& sockets, long usec)
{
    // validate all passed in sockets
    std::for_each(sockets.begin(), sockets.end(), [](const inet_socket& s) { s.validate(); });

    // timeout
    timeval tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;

    // handle infinite timeout
    timeval* ptv(usec < 0 ? nullptr : &tv);

    // iterate through set, and add to our fd_set
    fd_set fds;
    FD_ZERO(&fds);
    std::for_each(sockets.begin(), sockets.end(), [&fds](const inet_socket& s) { FD_SET(s.impl->fd, &fds); });

    // set is ordered, so max fd is the last element
    int max_fd(0);
    if(!sockets.empty())
    {
        max_fd = sockets.rbegin()->impl->fd;
    }

    // do the select
    auto ready(::select(max_fd+1, &fds, nullptr, nullptr, ptv));
    if(ready == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket(M): select"));
    }

    // copy sockets returned
    std::set<inet_socket> ret;
    std::copy_if(sockets.begin(), sockets.end(), std::inserter(ret, ret.end()), [&fds](const inet_socket& s) { return FD_ISSET(s.impl->fd, &fds); } );

    return ret;
}

std::size_t inet_socket::recv(void* buf, std::size_t bytes)
{
    validate();

    logger(LOG_DEBUG) << "receiving " << bytes << " on " << *this << "...\n";

    // read bytes from a fd
#if defined(_WIN32)
    auto len(::recv(this->impl->fd, reinterpret_cast<char*>(buf), bytes, 0));
#else
    auto len(::recv(this->impl->fd, buf, bytes, 0));
#endif
    if(len == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: recv: recv"));
    }

    logger(LOG_DEBUG) << "received " << len << " bytes\n";

    return static_cast<std::size_t>(len);
}

std::size_t inet_socket::send(const void* buf, std::size_t bytes)
{
    validate();

    logger(LOG_DEBUG) << "sending " << bytes << " on " << *this << "...\n";

    // write bytes to a fd
#if defined(_WIN32)
    auto len(::send(this->impl->fd, reinterpret_cast<const char*>(buf), bytes, 0));
#else
    auto len(::send(this->impl->fd, buf, bytes, 0));
#endif
    if(len == SOCKET_ERROR)
    {
        throw(std::system_error(errno, std::system_category(), "inet_socket: send: send"));
    }

    logger(LOG_DEBUG) << "sent " << len << " bytes\n";

    return static_cast<std::size_t>(len);
}

bool inet_socket::operator<(const inet_socket& other) const
{
    validate();
    other.validate();

    return this->impl->fd < other.impl->fd;
}

bool inet_socket::operator==(const inet_socket& other) const
{
    validate();
    other.validate();

    return this->impl->fd == other.impl->fd;
}

std::ostream& operator<<(std::ostream& os, const inet_socket& sock)
{
    sock.validate();

    return os << "s(" << sock.impl->fd << ')';
}
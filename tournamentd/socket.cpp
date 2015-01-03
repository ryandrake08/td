#include "socket.hpp"
#include "logger.hpp"
#include <algorithm>
#include <system_error>
#include <cerrno> // for errno
#include <cassert>
#include <cstring>
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
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
typedef int SOCKET;
static const SOCKET INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;
#endif

class eai_error_category : public std::error_category
{
public:
    virtual const char* name() const throw()
    {
        return "eai";
    }

    virtual bool equivalent (const std::error_code& code, int condition) const throw()
    {
        return *this==code.category() && static_cast<int>(default_error_condition(code.value()).value())==condition;
    }

    virtual std::string message(int ev) const
    {
        return gai_strerror(ev);
    }
};

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

// exception-safe wrapper for addrinfo

struct addrinfo_ptr
{
    addrinfo* ptr;
    addrinfo_ptr() : ptr(nullptr) {}
    ~addrinfo_ptr() { if(ptr) ::freeaddrinfo(ptr); }
    addrinfo_ptr(const addrinfo_ptr& other) = delete;
    addrinfo_ptr& operator=(const addrinfo_ptr&) = delete;
};

// pimpl (fd wrapper)

struct common_socket_impl
{
    SOCKET fd;

    // construct with given fd
    common_socket_impl(SOCKET newfd) : fd(newfd)
    {
        logger(LOG_DEBUG) << "wrapped fd: " << this->fd << '\n';
    }

    ~common_socket_impl()
    {
        logger(LOG_DEBUG) << "closing fd: " << this->fd << '\n';
#if defined(_WIN32) // thank you, Microsoft
        ::closesocket(this->fd);
#else
        ::sync();
        ::close(this->fd);
#endif
    }
};

void common_socket::validate() const
{
    if(!this->impl)
    {
        throw std::logic_error("common_socket: invalid socket impl");
    }
}

// empty constructor
common_socket::common_socket()
{
}

// create a socket with a given impl
common_socket::common_socket(common_socket_impl* imp) : impl(imp)
{
    validate();

    logger(LOG_DEBUG) << "creating socket from existing impl: " << *this << '\n';
}

common_socket common_socket::accept() const
{
    validate();

    logger(LOG_DEBUG) << "accepting with " << *this << '\n';

    // accept connection
    sockaddr_storage addr;
    socklen_t addrlen(sizeof(addr));
    auto sock(::accept(this->impl->fd, reinterpret_cast<sockaddr*>(&addr), &addrlen));
    if(sock == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "accept");
    }

    logger(LOG_DEBUG) << "accepted connection on " << *this << '\n';

    // set SO_NOSIGPIPE option
    int yes(1);
#if defined(_WIN32)
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }

    return common_socket(new common_socket_impl(sock));
}

int do_select(int max_fd, fd_set* fds, long usec)
{
    // timeout
    timeval tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;

    // handle infinite timeout
    timeval* ptv(usec < 0 ? nullptr : &tv);

    // do the select
    auto ready(::select(max_fd, fds, nullptr, nullptr, ptv));
    if(ready == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "select");
    }

    return ready;
}

// select on this socket
bool common_socket::select(long usec)
{
    validate();

    // single fd
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(this->impl->fd, &fds);

    return do_select(this->impl->fd+1, &fds, usec) > 0;
}

// select on multiple sockets
std::set<common_socket> common_socket::select(const std::set<common_socket>& sockets, long usec)
{
    // validate all passed in sockets
    std::for_each(sockets.begin(), sockets.end(), [](const common_socket& s) { s.validate(); });

    // iterate through set, and add to our fd_set
    fd_set fds;
    FD_ZERO(&fds);
    std::for_each(sockets.begin(), sockets.end(), [&fds](const common_socket& s) { FD_SET(s.impl->fd, &fds); });

    // set is ordered, so max fd is the last element
    int max_fd(sockets.begin() == sockets.end() ? 0 : sockets.rbegin()->impl->fd+1);

    // do the select
    do_select(max_fd, &fds, usec);

    // copy sockets returned
    std::set<common_socket> ret;
    std::copy_if(sockets.begin(), sockets.end(), std::inserter(ret, ret.end()), [&fds](const common_socket& s) { return FD_ISSET(s.impl->fd, &fds); } );
    return ret;
}

std::size_t common_socket::recv(void* buf, std::size_t bytes)
{
    validate();

    logger(LOG_DEBUG) << "receiving " << bytes << " on " << *this << '\n';

    // read bytes from a fd
#if defined(_WIN32)
    auto len(::recv(this->impl->fd, reinterpret_cast<char*>(buf), bytes, 0));
#else
    auto len(::recv(this->impl->fd, buf, bytes, 0));
#endif
    if(len == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "recv");
    }

    logger(LOG_DEBUG) << "received " << len << " bytes\n";

    return static_cast<std::size_t>(len);
}

std::size_t common_socket::send(const void* buf, std::size_t bytes)
{
    validate();

    logger(LOG_DEBUG) << "sending " << bytes << " on " << *this << '\n';

    // write bytes to a fd
#if defined(_WIN32)
    auto len(::send(this->impl->fd, reinterpret_cast<const char*>(buf), bytes, 0));
#else
    auto len(::send(this->impl->fd, buf, bytes, 0));
#endif
    if(len == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "send");
    }

    logger(LOG_DEBUG) << "sent " << len << " bytes\n";

    return static_cast<std::size_t>(len);
}

// is socket listening
bool common_socket::listening() const
{
    validate();

    int val(0);
    socklen_t len(sizeof(val));
    if(::getsockopt(this->impl->fd, SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "getsockopt");
    }

    return val != 0;
}

bool common_socket::operator<(const common_socket& other) const
{
    validate();
    other.validate();

    return this->impl->fd < other.impl->fd;
}

bool common_socket::operator==(const common_socket& other) const
{
    validate();
    other.validate();

    return this->impl->fd == other.impl->fd;
}

bool common_socket::operator!=(const common_socket& other) const
{
    validate();
    other.validate();

    return this->impl->fd != other.impl->fd;
}

std::ostream& operator<<(std::ostream& os, const common_socket& sock)
{
    sock.validate();

    sockaddr_storage addr;
    socklen_t addrlen(sizeof(addr));
    auto ret(::getpeername(sock.impl->fd, reinterpret_cast<sockaddr*>(&addr), &addrlen));
    if(ret != SOCKET_ERROR)
    {
        // get info
        char buffer[NI_MAXHOST];
        auto err(::getnameinfo(reinterpret_cast<sockaddr*>(&addr), addrlen, buffer, sizeof(buffer), nullptr, 0, 0));
        if(err == 0)
        {
            return os << "socket: " << sock.impl->fd << ", peer: " << buffer;
        }
    }

    return os << "socket: " << sock.impl->fd;
}

unix_socket::unix_socket(const char* path, bool client, int backlog)
{
    sockaddr_un addr;

    // first check size of path
    if(std::strlen(path) > sizeof(addr.sun_path)-1)
    {
        throw std::invalid_argument("unix_socket: path length too long");
    }

    // set up addr
    std::strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    addr.sun_family = AF_UNIX;
    addr.sun_len = SUN_LEN(&addr);

    // create the socket
    auto sock(::socket(PF_UNIX, SOCK_STREAM, 0));
    if(sock == INVALID_SOCKET)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // wrap the socket and store
    this->impl = std::shared_ptr<common_socket_impl>(new common_socket_impl(sock));

    logger(LOG_DEBUG) << "setting SO_NOSIGPIPE\n";

    // set SO_NOSIGPIPE option
    int yes(1);
#if defined(_WIN32)
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }

    // unlink old socket path
    ::unlink(path);

    logger(LOG_DEBUG) << "creating a socket\n";

    if(client)
    {
        logger(LOG_DEBUG) << "connecting " << *this << '\n';

        // connect to remote address
        if(::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::system_error(errno, std::system_category(), "connect");
        }
    }
    else
    {
        // bind to server port
        if(::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::system_error(errno, std::system_category(), "bind");
        }

        logger(LOG_DEBUG) << "listening on " << *this << " with backlog: " << backlog << '\n';

        // begin listening on the socket
        if(::listen(sock, backlog) == SOCKET_ERROR)
        {
            throw std::system_error(errno, std::system_category(), "listen");
        }
    }

    validate();
}

inet_socket::inet_socket(const char* host, const char* service, int family) : common_socket()
{
    // set up hints
    addrinfo hints = {0};
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    logger(LOG_DEBUG) << "looking up host: " << host << ", service: " << service << '\n';

    // fill in addrinfo
    addrinfo_ptr result;
    auto err(::getaddrinfo(host, service, &hints, &result.ptr));
    if(err != 0)
    {
        throw std::system_error(err, eai_error_category(), "getaddrinfo");
    }

    logger(LOG_DEBUG) << "creating a socket\n";

    // create the socket
    auto sock(::socket(result.ptr->ai_family, result.ptr->ai_socktype, result.ptr->ai_protocol));
    if(sock == INVALID_SOCKET)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // wrap the socket and store
    this->impl = std::shared_ptr<common_socket_impl>(new common_socket_impl(sock));

    logger(LOG_DEBUG) << "setting SO_NOSIGPIPE\n";

    // set SO_NOSIGPIPE option
    int yes(1);
#if defined(_WIN32)
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }

    logger(LOG_DEBUG) << "connecting " << *this << " to host: " << host << ", service: " << service << '\n';

    // connect to remote address
    if(::connect(sock, result.ptr->ai_addr, result.ptr->ai_addrlen) == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "connect");
    }

    validate();
}

inet_socket::inet_socket(const char* service, int family, int backlog) : common_socket()
{
    // set up hints
    addrinfo hints = {0};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    logger(LOG_DEBUG) << "looking up service: " << service << '\n';

    // fill in addrinfo
    addrinfo_ptr result;
    auto err(::getaddrinfo(nullptr, service, &hints, &result.ptr));
    if(err != 0)
    {
        throw std::system_error(err, eai_error_category(), "getaddrinfo");
    }

    logger(LOG_DEBUG) << "creating a socket\n";

    // create the socket
    auto sock(::socket(result.ptr->ai_family, result.ptr->ai_socktype, result.ptr->ai_protocol));
    if(sock == INVALID_SOCKET)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // wrap the socket and store
    this->impl = std::shared_ptr<common_socket_impl>(new common_socket_impl(sock));

    logger(LOG_DEBUG) << "setting SO_NOSIGPIPE\n";

    // set SO_NOSIGPIPE option
    int yes(1);
#if defined(_WIN32)
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
    if(::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }
    
    logger(LOG_DEBUG) << "setting SO_REUSEADDR\n";

    // set SO_REUSADDR option
#if defined(_WIN32)
    if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
    if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }

    logger(LOG_DEBUG) << "binding " << *this << " to service: " << service << '\n';

    // bind to server port
    if(::bind(sock, result.ptr->ai_addr, result.ptr->ai_addrlen) == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "bind");
    }

    logger(LOG_DEBUG) << "listening on " << *this << " with backlog: " << backlog << '\n';

    // begin listening on the socket
    if(::listen(sock, backlog) == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "listen");
    }

    validate();
}

inet4_socket::inet4_socket(const char* host, const char* service) : inet_socket(host, service, PF_INET)
{
}

inet4_socket::inet4_socket(const char* service, int backlog) : inet_socket(service, PF_INET, backlog)
{
}

inet6_socket::inet6_socket(const char* host, const char* service) : inet_socket(host, service, PF_INET6)
{
}

inet6_socket::inet6_socket(const char* service, int backlog) : inet_socket(service, PF_INET6, backlog)
{
}

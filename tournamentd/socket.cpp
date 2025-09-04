#include "socket.hpp"
#include "logger.hpp"
#include "shared_instance.hpp"
#include <cassert>
#include <cerrno> // for errno
#include <cstring>
#include <iterator>
#include <system_error>

#if defined(_WIN32)
#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <WS2tcpip.h>
#include <windows.h>
#include <winsock2.h>
typedef int socklen_t;
#endif
#if defined(__unix) || defined(__APPLE__)
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
typedef int SOCKET;
static const SOCKET INVALID_SOCKET = -1;
static const int SOCKET_ERROR = -1;
#endif

class eai_error_category : public std::error_category
{
public:
    const char* name() const throw() override
    {
        return "eai";
    }

    bool equivalent(const std::error_code& code, int condition) const throw() override
    {
        return *this == code.category() && static_cast<int>(default_error_condition(code.value()).value()) == condition;
    }

    std::string message(int ev) const override
    {
        return gai_strerror(ev);
    }
};

// socket_initializer init/terminate
class socket_initializer
{
public:
    socket_initializer()
    {
#if defined(_WIN32)
        WORD ver(MAKEWORD(1, 1));
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

    // no copy constructors/assignment
    socket_initializer(const socket_initializer& other) = delete;
    socket_initializer& operator=(const socket_initializer& other) = delete;
};

// exception-safe wrapper for addrinfo

struct addrinfo_ptr
{
    addrinfo* ptr;
    addrinfo_ptr() : ptr(nullptr) {}
    ~addrinfo_ptr()
    {
        if(ptr != nullptr)
        {
            ::freeaddrinfo(ptr);
        }
    }
    addrinfo_ptr(const addrinfo_ptr& other) = delete;
    addrinfo_ptr& operator=(const addrinfo_ptr&) = delete;
};

// pimpl (fd wrapper)

struct common_socket::impl
{
    // raii object to init/terminate socket subsystem
    std::shared_ptr<socket_initializer> socket_subsystem;

    SOCKET fd;

    // construct with given fd
    explicit impl(SOCKET newfd) : socket_subsystem(get_shared_instance<socket_initializer>()), fd(newfd)
    {
        logger(ll::debug) << "wrapped fd: " << this->fd << '\n';

#if defined(SO_NOSIGPIPE)
        logger(ll::debug) << "setting SO_NOSIGPIPE\n";

        // set SO_NOSIGPIPE option
        int yes(1);
#if defined(_WIN32)
        if(::setsockopt(newfd, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
        if(::setsockopt(newfd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
        {
            throw std::system_error(errno, std::system_category(), "setsockopt");
        }
#endif
    }

    ~impl()
    {
        logger(ll::debug) << "closing fd: " << this->fd << '\n';

#if defined(_WIN32) // thank you, Microsoft
        ::closesocket(this->fd);
#else
        // get the socket path (in case it's a unix socket)
        sockaddr_un addr;
        socklen_t addrlen(sizeof(addr));
        auto ret(::getsockname(this->fd, reinterpret_cast<sockaddr*>(&addr), &addrlen));

        // close the socket
        ::close(this->fd);

        // unlink if this was a unix socket
        if((ret != SOCKET_ERROR) && (addr.sun_family == AF_UNIX))
        {
            logger(ll::debug) << "unlinking unix socket " << this->fd << " path: " << addr.sun_path << '\n';
            unlink(addr.sun_path);
        }
#endif
    }
};

// empty constructor
common_socket::common_socket()
{
}

// create a socket with a given impl (needed for accept)
common_socket::common_socket(impl* imp) : pimpl(imp)
{
    logger(ll::debug) << "creating socket from existing impl: " << *this << '\n';
    if(!this->pimpl)
    {
        logger(ll::warning) << "creating socket from invalid impl\n";
    }
}

common_socket::~common_socket() = default;

common_socket common_socket::accept() const
{
    logger(ll::debug) << "accepting with " << *this << '\n';
    if(!this->pimpl)
    {
        logger(ll::warning) << "accepting on invalid socket impl\n";
        return common_socket();
    }

    // accept connection
    sockaddr_storage addr;
    socklen_t addrlen(sizeof(addr));
    auto sock(::accept(this->pimpl->fd, reinterpret_cast<sockaddr*>(&addr), &addrlen));
    if(sock == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "accept");
    }

    logger(ll::debug) << "accepted connection on " << *this << '\n';
    return common_socket(new impl(sock));
}

// select on multiple sockets
std::set<common_socket> common_socket::select(const std::set<common_socket>& sockets, long usec)
{
    // iterate through set, and add to our fd_set
    fd_set fds;
    FD_ZERO(&fds);
    std::for_each(sockets.begin(), sockets.end(), [&fds](const common_socket& s)
    {
        if(s.pimpl)
        {
            FD_SET(s.pimpl->fd, &fds);
        }
    });

    // set is ordered, so max fd is the last element
    auto max_fd(sockets.begin() == sockets.end() ? 0 : sockets.rbegin()->pimpl->fd + 1);

    // set the timeout
    timeval tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_usec = usec % 1000000;

    // handle infinite timeout
    timeval* ptv(usec < 0 ? nullptr : &tv);

    // do the select
    auto err(::select(max_fd, &fds, nullptr, nullptr, ptv));
    if(err == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "select");
    }

    // copy sockets returned
    std::set<common_socket> sockets_selected;
    std::copy_if(sockets.begin(), sockets.end(), std::inserter(sockets_selected, sockets_selected.end()), [&fds](const common_socket& s)
    {
        return FD_ISSET(s.pimpl->fd, &fds);
    });
    return sockets_selected;
}

long common_socket::peek(void* buf, std::size_t bytes) const
{
    logger(ll::debug) << "peeking " << bytes << " on " << *this << '\n';
    if(!this->pimpl)
    {
        logger(ll::warning) << "peeking from invalid socket impl\n";
        return 0;
    }

    // temporarily set socket to nonblocking
    unsigned long nonblocking(1);
#if defined(_WIN32)
    if(::ioctlsocket(this->pimpl->fd, FIONBIO, &nonblocking) != 0)
#else
    if(::ioctl(this->pimpl->fd, FIONBIO, &nonblocking) != 0)
#endif
    {
        throw std::system_error(errno, std::system_category(), "ioctl");
    }

    // peek at bytes from a fd
#if defined(_WIN32)
    auto len(::recv(this->pimpl->fd, reinterpret_cast<char*>(buf), (int)bytes, MSG_PEEK));
#else
    auto len(::recv(this->pimpl->fd, buf, bytes, MSG_PEEK));
#endif

    // store errno
    int recv_errno(errno);

    // set socket back to blocking
    unsigned long blocking(0);
#if defined(_WIN32)
    if(::ioctlsocket(this->pimpl->fd, FIONBIO, &blocking) != 0)
#else
    if(::ioctl(this->pimpl->fd, FIONBIO, &blocking) != 0)
#endif
    {
        throw std::system_error(errno, std::system_category(), "ioctl");
    }

    if(len == SOCKET_ERROR)
    {
        if(recv_errno == EAGAIN)
        {
            logger(ll::debug) << "peek: no bytes available (EAGAIN)\n";
        }
#if defined(_WIN32)
        else if(recv_errno == 42 || recv_errno == 0)
        {
            logger(ll::debug) << "peek: no bytes available\n";
        }
#endif
        else if(recv_errno == ECONNRESET)
        {
            logger(ll::debug) << "peek: connection reset by peer\n";
            return -1;
        }
        else
        {
            throw std::system_error(recv_errno, std::system_category(), "recv");
        }
        return 0;
    }
    else if(len == 0)
    {
        logger(ll::debug) << "peek: received zero bytes: connection shutdown gracefully\n";
        return -1;
    }
    else
    {
        logger(ll::debug) << "peek: " << len << " bytes available\n";
        return len;
    }
}

long common_socket::recv(void* buf, std::size_t bytes)
{
    logger(ll::debug) << "receiving " << bytes << " on " << *this << '\n';
    if(!this->pimpl)
    {
        logger(ll::warning) << "receiving from invalid socket impl\n";
        return 0;
    }

    // read bytes from a fd
#if defined(_WIN32)
    auto len(::recv(this->pimpl->fd, reinterpret_cast<char*>(buf), (int)bytes, 0));
#else
    auto len(::recv(this->pimpl->fd, buf, bytes, 0));
#endif
    if(len == SOCKET_ERROR)
    {
        if(errno == EPIPE)
        {
            logger(ll::debug) << "recv: broken pipe\n";
            return -1;
        }

        throw std::system_error(errno, std::system_category(), "recv");
    }

    logger(ll::debug) << "received " << len << " bytes\n";

    return len;
}

long common_socket::send(const void* buf, std::size_t bytes)
{
    logger(ll::debug) << "sending " << bytes << " on " << *this << '\n';
    if(!this->pimpl)
    {
        logger(ll::warning) << "sending to invalid socket impl\n";
        return 0;
    }

    // write bytes to a fd
#if defined(_WIN32)
    auto len(::send(this->pimpl->fd, reinterpret_cast<const char*>(buf), (int)bytes, 0));
#else
    auto len(::send(this->pimpl->fd, buf, bytes, 0));
#endif
    if(len == SOCKET_ERROR)
    {
        if(errno == EPIPE)
        {
            logger(ll::debug) << "send: connection broken pipe\n";
            return -1;
        }

        throw std::system_error(errno, std::system_category(), "send");
    }

    logger(ll::debug) << "sent " << len << " bytes\n";

    return len;
}

// is socket listening
bool common_socket::listening() const
{
    if(!this->pimpl)
    {
        logger(ll::warning) << "invalid socket impl is never listening\n";
        return false;
    }

    int val(0);
    socklen_t len(sizeof(val));
    if(::getsockopt(this->pimpl->fd, SOL_SOCKET, SO_ACCEPTCONN, reinterpret_cast<char*>(&val), &len) == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "getsockopt");
    }

    return val != 0;
}

bool common_socket::operator<(const common_socket& other) const
{
    if(!this->pimpl || !other.pimpl)
    {
        return false;
    }
    else
    {
        return this->pimpl->fd < other.pimpl->fd;
    }
}

bool common_socket::operator==(const common_socket& other) const
{
    if(!this->pimpl || !other.pimpl)
    {
        return !this->pimpl && !other.pimpl;
    }
    else
    {
        return this->pimpl->fd == other.pimpl->fd;
    }
}

bool common_socket::operator!=(const common_socket& other) const
{
    if(!this->pimpl || !other.pimpl)
    {
        return this->pimpl || other.pimpl;
    }
    else
    {
        return this->pimpl->fd != other.pimpl->fd;
    }
}

std::ostream& operator<<(std::ostream& os, const common_socket& sock)
{
    if(!sock.pimpl)
    {
        return os << "socket: invalid";
    }

#if 0 // remove peer name lookup, as it can sometimes take 5 seconds if ipv6 resolution times out
    sockaddr_storage addr;
    socklen_t addrlen(sizeof(addr));
    auto ret(::getpeername(sock.pimpl->fd, reinterpret_cast<sockaddr*>(&addr), &addrlen));
    if(ret != SOCKET_ERROR)
    {
        // get info
        char buffer[NI_MAXHOST];
        auto err(::getnameinfo(reinterpret_cast<sockaddr*>(&addr), addrlen, buffer, sizeof(buffer), nullptr, 0, 0));
        if(err == 0)
        {
            return os << "socket: " << sock.pimpl->fd << ", peer: " << buffer;
        }
    }
#endif
    return os << "socket: " << sock.pimpl->fd;
}

unix_socket::unix_socket(const char* path, bool client, int backlog)
{
#if !defined(_WIN32)
    sockaddr_un addr;

    // first check size of path
    if(std::strlen(path) == 0)
    {
        throw std::invalid_argument("unix_socket: path cannot be empty");
    }
    if(std::strlen(path) > sizeof(addr.sun_path) - 1)
    {
        throw std::invalid_argument("unix_socket: path length too long");
    }

    // set up addr
    std::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    addr.sun_family = AF_UNIX;
#if defined(__APPLE__)
    addr.sun_len = static_cast<unsigned char>(SUN_LEN(&addr));
#endif

    // create the socket
    auto sock(::socket(PF_UNIX, SOCK_STREAM, 0));
    if(sock == INVALID_SOCKET)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // wrap the socket and store
    this->pimpl = std::make_shared<impl>(sock);

    if(client)
    {
        logger(ll::debug) << "connecting " << *this << '\n';

        // connect to remote address
        if(::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::system_error(errno, std::system_category(), "connect");
        }
    }
    else
    {
        // unlink old socket path, if it exists
        ::unlink(path);

        // bind to server port
        if(::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            throw std::system_error(errno, std::system_category(), "bind");
        }

        logger(ll::debug) << "listening on " << *this << " with backlog: " << backlog << '\n';

        // begin listening on the socket
        if(::listen(sock, backlog) == SOCKET_ERROR)
        {
            throw std::system_error(errno, std::system_category(), "listen");
        }
    }
#endif
}

inet_socket::inet_socket(const char* host, const char* service, int family) : common_socket()
{
    // validate parameters
    if(!host || std::strlen(host) == 0)
    {
        throw std::invalid_argument("inet_socket: host cannot be empty");
    }
    if(!service || std::strlen(service) == 0)
    {
        throw std::invalid_argument("inet_socket: service cannot be empty");
    }

    // set up hints
    addrinfo hints = { 0, 0, 0, 0, 0, nullptr, nullptr, nullptr };
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    logger(ll::debug) << "looking up host: " << host << ", service: " << service << '\n';

    // fill in addrinfo
    addrinfo_ptr result;
    auto err(::getaddrinfo(host, service, &hints, &result.ptr));
    if(err != 0)
    {
        throw std::system_error(err, eai_error_category(), "getaddrinfo");
    }

    logger(ll::debug) << "creating a socket\n";

    // create the socket
    auto sock(::socket(result.ptr->ai_family, result.ptr->ai_socktype, result.ptr->ai_protocol));
    if(sock == INVALID_SOCKET)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // wrap the socket and store
    this->pimpl = std::make_shared<impl>(sock);

    logger(ll::debug) << "connecting " << *this << " to host: " << host << ", service: " << service << '\n';

    // connect to remote address
#if defined(_WIN32)
    if(::connect(sock, result.ptr->ai_addr, (int)result.ptr->ai_addrlen) == SOCKET_ERROR)
#else
    if(::connect(sock, result.ptr->ai_addr, result.ptr->ai_addrlen) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "connect");
    }
}

inet_socket::inet_socket(const char* service, int family, int backlog) : common_socket()
{
    // validate parameters
    if(!service || std::strlen(service) == 0)
    {
        throw std::invalid_argument("inet_socket: service cannot be empty");
    }

    // validate port number if it's numeric
    char* endptr;
    long port = std::strtol(service, &endptr, 10);
    if(*endptr == '\0') // service is purely numeric
    {
        if(port < 0 || port > 65535)
        {
            throw std::invalid_argument("inet_socket: port number out of valid range (0-65535)");
        }
    }

    // set up hints
    addrinfo hints = { 0, 0, 0, 0, 0, nullptr, nullptr, nullptr };
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    logger(ll::debug) << "looking up service: " << service << '\n';

    // fill in addrinfo
    addrinfo_ptr result;
    auto err(::getaddrinfo(nullptr, service, &hints, &result.ptr));
    if(err != 0)
    {
        throw std::system_error(err, eai_error_category(), "getaddrinfo");
    }

    logger(ll::debug) << "creating a socket\n";

    // create the socket
    auto sock(::socket(result.ptr->ai_family, result.ptr->ai_socktype, result.ptr->ai_protocol));
    if(sock == INVALID_SOCKET)
    {
        throw std::system_error(errno, std::system_category(), "socket");
    }

    // wrap the socket and store
    this->pimpl = std::make_shared<impl>(sock);

    logger(ll::debug) << "setting SO_REUSEADDR\n";

    // set SO_REUSADDR option
    int yes(1);
#if defined(_WIN32)
    if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
    if(::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "setsockopt");
    }

    if(family == PF_INET6)
    {
        logger(ll::debug) << "setting IPV6_V6ONLY\n";

        // set IPV6_V6ONLY option. some systems don't support dual-stack. if ipv4 is needed, create a separate ipv4 socket
        yes = 1;
#if defined(_WIN32)
        if(::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&yes), sizeof(yes)) == SOCKET_ERROR)
#else
        if(::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) == SOCKET_ERROR)
#endif
        {
            throw std::system_error(errno, std::system_category(), "setsockopt");
        }
    }

    logger(ll::debug) << "binding " << *this << " to service: " << service << '\n';

    // bind to server port
#if defined(_WIN32)
    if(::bind(sock, result.ptr->ai_addr, (int)result.ptr->ai_addrlen) == SOCKET_ERROR)
#else
    if(::bind(sock, result.ptr->ai_addr, result.ptr->ai_addrlen) == SOCKET_ERROR)
#endif
    {
        throw std::system_error(errno, std::system_category(), "bind");
    }

    logger(ll::debug) << "listening on " << *this << " with backlog: " << backlog << '\n';

    // begin listening on the socket
    if(::listen(sock, backlog) == SOCKET_ERROR)
    {
        throw std::system_error(errno, std::system_category(), "listen");
    }
}

inet_socket::~inet_socket() = default;

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

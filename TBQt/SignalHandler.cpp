#include "SignalHandler.hpp"

#if !defined(_WIN32)

#include <QDebug>
#include <QSocketNotifier>

#include <array>
#include <csignal>
#include <stdexcept>
#include <unordered_set>

#include <sys/socket.h>
#include <unistd.h>

// Internal class to manage socket pair for signal handling
struct socket_pair
{
    std::array<int, 2> fds;

    socket_pair()
    {
        if(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) != 0)
        {
            throw std::runtime_error("Failed to create socket pair for signal handling");
        }
    }

    ~socket_pair()
    {
        if(fds[0] != -1)
        {
            close(fds[0]);
        }

        if(fds[1] != -1)
        {
            close(fds[1]);
        }
    }

    void notify()
    {
        char sig = 1;
        ssize_t result = write(fds[0], &sig, 1);
        (void)result; // Suppress unused variable warning
    }

    bool read_notification()
    {
        char signal = '\0';
        ssize_t result = read(fds[1], &signal, 1);
        return result > 0;
    }

    int get_read_fd() const
    {
        return fds[1];
    }
};

namespace
{
    // Static registry of all active socket_pair instances
    std::unordered_set<socket_pair*> socketInstances;
}

SignalHandler::SignalHandler(QObject* parent)
try :
    QObject(parent),
    sockets(new socket_pair()),
    notifier(new QSocketNotifier(this->sockets->get_read_fd(), QSocketNotifier::Read, this))
{
    // Connect the SocketNotifier to our signal handler
    QObject::connect(this->notifier, &QSocketNotifier::activated, this, &SignalHandler::handleSignal);

    // Register this instance
    socketInstances.insert(this->sockets.get());

    // Install signal handlers only once (when first instance is created)
    if(socketInstances.size() == 1)
    {
        struct sigaction term_action {};
        sigemptyset(&term_action.sa_mask);
        term_action.sa_flags = SA_RESTART;
        term_action.sa_handler = [](int /* signum */)
        {
            // Write to all registered instances' socket pairs
            for(socket_pair* sockets : socketInstances)
            {
                sockets->notify();
            }
        };

        if(sigaction(SIGINT, &term_action, nullptr) == -1)
        {
            qWarning() << "Failed to install SIGINT handler";
        }

        if(sigaction(SIGTERM, &term_action, nullptr) == -1)
        {
            qWarning() << "Failed to install SIGTERM handler";
        }

        if(sigaction(SIGQUIT, &term_action, nullptr) == -1)
        {
            qWarning() << "Failed to install SIGQUIT handler";
        }
    }
}
catch(const std::exception& e)
{
    qWarning() << "SignalHandler initialization failed:" << e.what();
}

SignalHandler::~SignalHandler()
{
    socketInstances.erase(this->sockets.get());
    this->notifier->setEnabled(false);
}

void SignalHandler::handleSignal()
{
    this->notifier->setEnabled(false);
    this->sockets->read_notification();
    Q_EMIT shutdownRequested();
    this->notifier->setEnabled(true);
}

#else

SignalHandler::SignalHandler(QObject* parent) : QObject(parent)
{
    // Windows: Signal handling not implemented
    // Qt applications on Windows handle shutdown via native event loop
}

SignalHandler::~SignalHandler()
{
}

void SignalHandler::handleSignal()
{
}

#endif
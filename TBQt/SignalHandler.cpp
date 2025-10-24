#include "SignalHandler.hpp"

#include <QDebug>
#include <QSocketNotifier>

#include <array>
#include <csignal>

#if !defined(_WIN32)
#include <sys/socket.h>
#include <unistd.h>

namespace
{

    // File-local static variable for Unix signal handling
    std::array<int, 2> sigtermFd = { -1, -1 };

} // anonymous namespace
#endif

SignalHandler::SignalHandler(QObject* parent) : QObject(parent)
{
#if !defined(_WIN32)
    // Create a socket pair for self-pipe trick
    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd.data()) == -1)
    {
        qWarning() << "Failed to create signal socket pair";
        return;
    }

    // Create socket notifier to monitor the read end
    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    QObject::connect(snTerm, &QSocketNotifier::activated, this, &SignalHandler::handleSignal);

    // Install signal handlers for SIGTERM and SIGINT
    struct sigaction term_action {};
    sigemptyset(&term_action.sa_mask);
    term_action.sa_flags = SA_RESTART;
    term_action.sa_handler = [](int /* signum */)
    {
        // Write to socket to wake up Qt event loop
        char sig = 1;
        ssize_t result = write(sigtermFd[0], &sig, 1);
        (void)result; // Suppress unused variable warning
    };

    if(sigaction(SIGINT, &term_action, nullptr) == -1)
    {
        qWarning() << "Failed to install SIGINT handler";
        return;
    }

    if(sigaction(SIGTERM, &term_action, nullptr) == -1)
    {
        qWarning() << "Failed to install SIGTERM handler";
        return;
    }

    if(sigaction(SIGQUIT, &term_action, nullptr) == -1)
    {
        qWarning() << "Failed to install SIGQUIT handler";
        return;
    }
#else
    // Windows doesn't support Unix signals in the same way
    // Signal handling could be implemented using Windows-specific mechanisms if needed
    snTerm = nullptr;
#endif
}

SignalHandler::~SignalHandler()
{
#if !defined(_WIN32)
    if(snTerm)
    {
        snTerm->setEnabled(false);
    }

    if(sigtermFd[0] != -1)
    {
        close(sigtermFd[0]);
    }

    if(sigtermFd[1] != -1)
    {
        close(sigtermFd[1]);
    }
#endif
}

void SignalHandler::handleSignal()
{
#if !defined(_WIN32)
    snTerm->setEnabled(false);

    char signal = '\0';
    ssize_t result = read(sigtermFd[1], &signal, 1);
    (void)result; // Suppress unused variable warning

    Q_EMIT shutdownRequested();

    snTerm->setEnabled(true);
#endif
}
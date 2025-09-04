#include "SignalHandler.hpp"

#include <QDebug>
#include <QSocketNotifier>

#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

// Static member definition
int SignalHandler::sigtermFd[2];

SignalHandler::SignalHandler(QObject* parent) : QObject(parent)
{
    // Create a socket pair for self-pipe trick
    if(::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd) == -1)
    {
        qWarning() << "Failed to create signal socket pair";
        return;
    }

    // Create socket notifier to monitor the read end
    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    QObject::connect(snTerm, &QSocketNotifier::activated, this, &SignalHandler::handleSignal);

    // Install signal handlers for SIGTERM and SIGINT
    struct sigaction term_action;
    term_action.sa_handler = SignalHandler::unixSignalHandler;
    sigemptyset(&term_action.sa_mask);
    term_action.sa_flags = SA_RESTART;

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
}

SignalHandler::~SignalHandler()
{
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
}

void SignalHandler::unixSignalHandler(int)
{
    // Write to socket to wake up Qt event loop
    char sig = 1;
    ssize_t result = write(sigtermFd[0], &sig, 1);
    (void)result; // Suppress unused variable warning
}

void SignalHandler::handleSignal()
{
    snTerm->setEnabled(false);

    char signal;
    ssize_t result = read(sigtermFd[1], &signal, 1);
    (void)result; // Suppress unused variable warning

    Q_EMIT shutdownRequested();

    snTerm->setEnabled(true);
}
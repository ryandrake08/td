#pragma once

#include <QObject>
#include <memory>

class QSocketNotifier;

#if !defined(_WIN32)
struct socket_pair;
#endif

class SignalHandler : public QObject
{
    Q_OBJECT

#if !defined(_WIN32)
    std::unique_ptr<socket_pair> sockets;
    QSocketNotifier* notifier;
#endif

public:
    explicit SignalHandler(QObject* parent = nullptr);
    ~SignalHandler();

Q_SIGNALS:
    void shutdownRequested();

private Q_SLOTS:
    void handleSignal();
};
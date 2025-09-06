#pragma once

#include <QObject>
#include <array>

class QSocketNotifier;

class SignalHandler : public QObject
{
    Q_OBJECT

    // Raw signal handling
    static std::array<int, 2> sigtermFd;
    static void unixSignalHandler(int unused);

    QSocketNotifier* snTerm;

public:
    explicit SignalHandler(QObject* parent = nullptr);
    ~SignalHandler();

Q_SIGNALS:
    void shutdownRequested();

private Q_SLOTS:
    void handleSignal();
};
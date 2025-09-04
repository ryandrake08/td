#pragma once

#include <QObject>

class QSocketNotifier;

class SignalHandler : public QObject
{
    Q_OBJECT

public:
    explicit SignalHandler(QObject* parent = nullptr);
    ~SignalHandler();

Q_SIGNALS:
    void shutdownRequested();

private Q_SLOTS:
    void handleSignal();

private:
    static void unixSignalHandler(int unused);

    QSocketNotifier* snTerm;

    static int sigtermFd[2];
};
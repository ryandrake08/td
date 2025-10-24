#pragma once

#include <QObject>

class QSocketNotifier;

class SignalHandler : public QObject
{
    Q_OBJECT

    QSocketNotifier* snTerm;

public:
    explicit SignalHandler(QObject* parent = nullptr);
    ~SignalHandler();

Q_SIGNALS:
    void shutdownRequested();

private Q_SLOTS:
    void handleSignal();
};
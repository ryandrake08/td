#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

class TournamentService;

class TournamentConnection : public QObject
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

private Q_SLOTS:
    void on_connected();
    void on_disconnected();
    void on_readyRead();
    void on_error();

public:
    // construct
    TournamentConnection(QObject* parent=nullptr);
    virtual ~TournamentConnection();

    // connect to TournamentService
    void connect(const TournamentService& tournament);
    void disconnect();

    // send a command
    void send_command(const QString& cmd, const QVariantMap& arg);

Q_SIGNALS:
    void connected();
    void disconnected();
    void receivedData(const QVariantMap& data);
    void errorOccurred(const QString& error);
};

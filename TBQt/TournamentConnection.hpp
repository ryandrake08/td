#pragma once

#include <QHash>
#include <QIODevice>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QVariant>

class TournamentService;

class TournamentConnection : public QObject
{
    Q_OBJECT

    // The abstract IO device (either a tcp or unix socket)
    QScopedPointer<QIODevice> device;

private Q_SLOTS:
    void connected();
    void disconnected();
    void readyRead();

public:
    // construct
    TournamentConnection(QObject* parent=nullptr);

    // connect to TournamentService
    void connect(const TournamentService& tournament);
    void disconnect();

    // send a command
    void send_command(const QString& cmd, const QVariantHash& arg);

Q_SIGNALS:
    void tournament_connected();
    void tournament_disconnected();
    void received_data(const QVariantHash& data);
};

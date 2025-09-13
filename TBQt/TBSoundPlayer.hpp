#pragma once

#include <QObject>
#include <memory>

class TournamentSession;

class TBSoundPlayer : public QObject
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

private Q_SLOTS:
    void on_stateChanged(const QString& key, const QVariant& value);
    void on_playerMovementsUpdated(const QVariantList& movements);

public:
    explicit TBSoundPlayer(const TournamentSession& session, QObject* parent = nullptr);
    virtual ~TBSoundPlayer();
};
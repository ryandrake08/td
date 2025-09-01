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

public:
    explicit TBSoundPlayer(QObject* parent = nullptr);
    virtual ~TBSoundPlayer();

    void setSession(const TournamentSession& session);
};
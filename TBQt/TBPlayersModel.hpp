#pragma once

#include "TBVariantListTableModel.hpp"
#include <QObject>

class TournamentSession;

class TBPlayersModel : public TBVariantListTableModel
{
    Q_OBJECT

private Q_SLOTS:
    void on_stateChanged(const QVariantMap& state);

public:
    explicit TBPlayersModel(const TournamentSession& session, QObject* parent = nullptr);
};

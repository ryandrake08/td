#pragma once

#include "TBVariantListTableModel.hpp"
#include <QObject>

class TournamentSession;

class TBPlayersModel : public TBVariantListTableModel
{
    Q_OBJECT

private Q_SLOTS:
    void on_stateChanged(const QString& key, const QVariant& value);

public:
    explicit TBPlayersModel(const TournamentSession& session, QObject* parent = nullptr);

    // Override for checkbox functionality
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

Q_SIGNALS:
    void seatPlayerRequested(const QString& playerId);
    void unseatPlayerRequested(const QString& playerId);
};

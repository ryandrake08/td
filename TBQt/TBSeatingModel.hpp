#pragma once

#include "TBVariantListTableModel.hpp"
#include <QObject>

class TournamentSession;

class TBSeatingModel : public TBVariantListTableModel
{
    Q_OBJECT

Q_SIGNALS:
    void fundPlayerRequested(const QString& playerId, int sourceId);
    void bustPlayerRequested(const QString& playerId);
    void unseatPlayerRequested(const QString& playerId);

private Q_SLOTS:
    void on_stateChanged(const QString& key, const QVariant& value);

public:
    explicit TBSeatingModel(const TournamentSession& session, QObject* parent = nullptr);

    // Override for checkbox functionality in "Paid" column and button in "Manage" column
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};
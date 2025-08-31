#pragma once

#include "TBVariantListTableModel.hpp"
#include <QObject>

class TournamentSession;

class TBResultsModel : public TBVariantListTableModel
{
    Q_OBJECT

private:
    const TournamentSession& m_session;

private Q_SLOTS:
    void on_stateChanged(const QString& key, const QVariant& value);

public:
    explicit TBResultsModel(const TournamentSession& session, QObject* parent = nullptr);

    // Override for text alignment and payout formatting
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};
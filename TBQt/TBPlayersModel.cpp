#include "TBPlayersModel.hpp"
#include "TournamentSession.hpp"
#include <QDebug>

TBPlayersModel::TBPlayersModel(const TournamentSession& session, QObject* parent) : TBVariantListTableModel(parent)
{
    // set headers for this kind of model
    this->addHeader("name", QObject::tr("Player Name"));
    this->addHeader("seat_name", QObject::tr("Seated"));

    // observe session state
    QObject::connect(&session, SIGNAL(stateChanged(const QString&, const QVariant&)), this, SLOT(on_stateChanged(const QString&, const QVariant&)));
}

void TBPlayersModel::on_stateChanged(const QString& key, const QVariant& value)
{
    if(key == "seated_players")
    {
        this->setListData(value.toList());
    }
}

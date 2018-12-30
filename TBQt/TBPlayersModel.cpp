#include "TBPlayersModel.hpp"
#include "TournamentSession.hpp"
#include <QDebug>

TBPlayersModel::TBPlayersModel(const TournamentSession& session, QObject* parent) : TBVariantListTableModel(parent)
{
    // set headers for this kind of model
    this->addHeader("name", QObject::tr("Player Name"));
    this->addHeader("seat_name", QObject::tr("Seated"));

    // observe session
    QObject::connect(&session, SIGNAL(stateChanged(const QVariantMap&)), this, SLOT(on_stateChanged(const QVariantMap&)));
}

void TBPlayersModel::on_stateChanged(const QVariantMap& state)
{
    qDebug() << "TBPlayersModel::on_stateChanged";
    this->setListData(state["seated_players"].toList());
}

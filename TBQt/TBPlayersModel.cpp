#include "TBPlayersModel.hpp"

#include "TournamentSession.hpp"

TBPlayersModel::TBPlayersModel(const TournamentSession& session, QObject* parent) : TBVariantListTableModel(parent)
{
    // set headers for this kind of model
    this->addHeader("player_name", QObject::tr("Player Name"));
    this->addHeader("seat_name", QObject::tr("Seated"));

    // observe session state
    QObject::connect(&session, &TournamentSession::stateChanged, this, &TBPlayersModel::on_stateChanged);
}

void TBPlayersModel::on_stateChanged(const QString& key, const QVariant& value)
{
    if(key == "seated_players")
    {
        this->setListData(value.toList());
    }
}

QVariant TBPlayersModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return {};

    // Handle checkbox for the "Seated" column (column 1)
    if(index.column() == 1) // "seat_name" column
    {
        if(role == Qt::CheckStateRole)
        {
            // Get the player data for this row
            QVariantMap rowData = this->getRowData(index.row());
            QString seatName = rowData["seat_name"].toString();

            // Checkbox is checked if seat_name is not empty
            return seatName.isEmpty() ? Qt::Unchecked : Qt::Checked;
        }
        else if(role == Qt::DisplayRole)
        {
            // Show "Seat" label next to the checkbox
            return tr("Seat");
        }
        else if(role == Qt::TextAlignmentRole)
        {
            // Center "Seated" column content horizontally and vertically
            return static_cast<int>(Qt::AlignCenter);
        }
    }
    // Handle text alignment for "Player Name" column (column 0)
    else if(index.column() == 0 && role == Qt::TextAlignmentRole)
    {
        // Left-justify and center vertically for Player Name
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }

    // For all other cases, use the base implementation
    return TBVariantListTableModel::data(index, role);
}

bool TBPlayersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if(!index.isValid())
        return false;

    // Handle checkbox toggle for the "Seated" column (column 1)
    if(index.column() == 1 && role == Qt::CheckStateRole)
    {
        auto checkState = static_cast<Qt::CheckState>(value.toInt());

        // Get the raw row data which contains all player information
        QVariantMap rowData = this->getRowData(index.row());
        QString playerName = rowData["player_name"].toString();
        QString playerId = rowData["player_id"].toString();

        if(checkState == Qt::Checked)
        {
            // Request to seat the player via signal
            Q_EMIT seatPlayerRequested(playerId);
        }
        else
        {
            // Request to unseat the player via signal
            Q_EMIT unseatPlayerRequested(playerId);
        }

        // Don't emit dataChanged here - the session state change will trigger a model update
        return true;
    }

    // For all other cases, use the base implementation
    return TBVariantListTableModel::setData(index, value, role);
}

Qt::ItemFlags TBPlayersModel::flags(const QModelIndex& index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // Make the "Seated" column (column 1) checkable
    if(index.column() == 1)
    {
        flags |= Qt::ItemIsUserCheckable;
    }

    return flags;
}

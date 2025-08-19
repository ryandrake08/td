#include "TBSeatingModel.hpp"
#include "TournamentSession.hpp"
#include <QDebug>

TBSeatingModel::TBSeatingModel(const TournamentSession& session, QObject* parent) : TBVariantListTableModel(parent)
{
    // set headers for this kind of model
    this->addHeader("table_name", QObject::tr("Table"));
    this->addHeader("seat_name", QObject::tr("Seat"));
    this->addHeader("player_name", QObject::tr("Player Name"));
    this->addHeader("buyin", QObject::tr("Paid"));
    this->addHeader("manage", QObject::tr("Manage"));

    // observe session state
    QObject::connect(&session, SIGNAL(stateChanged(const QString&, const QVariant&)), this, SLOT(on_stateChanged(const QString&, const QVariant&)));
}

void TBSeatingModel::on_stateChanged(const QString& key, const QVariant& value)
{
    if(key == "seated_players")
    {
        // Filter to only show players that are actually seated (have a seat_name)
        QVariantList allPlayers = value.toList();
        QVariantList seatedPlayers;

        for (const QVariant& playerVariant : allPlayers)
        {
            QVariantMap player = playerVariant.toMap();
            QString seatName = player["seat_name"].toString();

            // Only include players that have been assigned a seat
            if (!seatName.isEmpty())
            {
                seatedPlayers.append(playerVariant);
            }
        }

        this->setListData(seatedPlayers);
    }
}

QVariant TBSeatingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // Handle checkbox for the "Paid" column (column 3)
    if (index.column() == 3) // "buyin" column
    {
        if (role == Qt::CheckStateRole)
        {
            // Get the player data for this row
            QVariantMap rowData = this->getRowData(index.row());
            bool hasBuyin = rowData["buyin"].toBool();

            // Checkbox is checked if player has bought in
            return hasBuyin ? Qt::Checked : Qt::Unchecked;
        }
        else if (role == Qt::DisplayRole)
        {
            // Don't show text in the checkbox column
            return QVariant();
        }
    }
    // Handle button for the "Manage" column (column 4)
    else if (index.column() == 4) // "manage" column
    {
        if (role == Qt::DisplayRole)
        {
            return tr("Manage");
        }
    }

    // Handle text alignment for all columns
    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == 2) // "Player Name" column
        {
            // Left-justify and center vertically for Player Name
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else // All other columns: Table, Seat, Paid, Manage
        {
            // Center horizontally and vertically for Table, Seat, Paid, Manage
            return static_cast<int>(Qt::AlignCenter);
        }
    }

    // Handle Qt::UserRole to provide player_id for any column
    if (role == Qt::UserRole)
    {
        QVariantMap rowData = this->getRowData(index.row());
        return rowData["player_id"].toString();
    }

    // For all other cases, use the base implementation
    return TBVariantListTableModel::data(index, role);
}

Qt::ItemFlags TBSeatingModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    // Make the "Paid" column (column 3) show checkboxes but not editable
    if (index.column() == 3)
    {
        flags |= Qt::ItemIsUserCheckable;
        // Note: We don't add Qt::ItemIsEditable to make it read-only
    }

    return flags;
}

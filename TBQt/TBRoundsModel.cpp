#include "TBRoundsModel.hpp"
#include "TournamentSession.hpp"

TBRoundsModel::TBRoundsModel(QObject* parent) : TBVariantListTableModel(parent)
{
}

void TBRoundsModel::setListData(const QVariantList& model)
{
    m_originalData = model;

    // Filter out round 0 for display
    QVariantList filteredData;
    for (int i = 0; i < model.size(); i++)
    {
        if (i == 0) // Skip round 0 (before tournament starts)
            continue;
        filteredData.append(model[i]);
    }

    TBVariantListTableModel::setListData(filteredData);
}

QVariantList TBRoundsModel::listData() const
{
    return m_originalData; // Return original data including round 0
}

QVariant TBRoundsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return QVariant();

    // Handle start time calculation (column 1)
    if (index.column() == 1 && role == Qt::DisplayRole)
    {
        return calculateStartTime(index.row());
    }

    // Handle time formatting for duration and break columns
    QVariant baseData = TBVariantListTableModel::data(index, role);
    if (role == Qt::DisplayRole)
    {
        if (index.column() == 2) // Duration column
        {
            return formatTime(baseData.toInt()); // baseData is already in milliseconds
        }
        else if (index.column() == 6) // Ante Type column
        {
            // Get ante value for this row
            QModelIndex anteIndex = index.model()->index(index.row(), 5);
            int ante = anteIndex.data(Qt::EditRole).toInt();
            if (ante <= 0)
                return QString(); // Show nothing when no ante

            // Show ante type when there's an ante
            int anteType = baseData.toInt();
            return TournamentSession::toString(TournamentSession::toAnteType(anteType));
        }
        else if (index.column() == 7) // Break column
        {
            int breakDuration = baseData.toInt(); // baseData is already in milliseconds
            return breakDuration > 0 ? formatTime(breakDuration) : QString();
        }
    }

    return baseData;
}

bool TBRoundsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    // Round number and start time are not editable
    if (index.column() == 0 || index.column() == 1)
        return false;

    return TBVariantListTableModel::setData(index, value, role);
}

Qt::ItemFlags TBRoundsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    // Round number and start time columns are not editable
    if (index.column() == 0 || index.column() == 1)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return TBVariantListTableModel::flags(index);
}

QString TBRoundsModel::formatTime(int milliseconds) const
{
    int totalMinutes = milliseconds / (1000 * 60); // Convert milliseconds to minutes
    if (totalMinutes < 0)
        return QString(); // Only return empty for negative values

    int hours = totalMinutes / 60;
    int mins = totalMinutes % 60;
    return QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(mins, 2, 10, QChar('0'));
}

QString TBRoundsModel::calculateStartTime(int roundIndex) const
{
    int totalMilliseconds = 0;

    // Calculate cumulative time for all rounds up to this point
    QVariantList displayData = TBVariantListTableModel::listData();
    for (int i = 0; i < roundIndex && i < displayData.size(); i++)
    {
        QVariantMap round = displayData[i].toMap();
        totalMilliseconds += round.value("duration").toInt();
        totalMilliseconds += round.value("break_duration").toInt();
    }

    return formatTime(totalMilliseconds);
}
#include "TBSeatingCompoundSortProxyModel.hpp"

#include "TBVariantListTableModel.hpp"

#include <QDebug>

TBSeatingCompoundSortProxyModel::TBSeatingCompoundSortProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
}

bool TBSeatingCompoundSortProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    QAbstractItemModel* source = this->sourceModel();
    if (!source)
    {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    // Always sort by table first, then seat - regardless of which column was clicked

    // Get table values (column 0: "Table")
    QString leftTable = source->data(source->index(left.row(), 0), Qt::DisplayRole).toString();
    QString rightTable = source->data(source->index(right.row(), 0), Qt::DisplayRole).toString();

    if (leftTable != rightTable)
    {
        // Try to extract numeric table numbers from table names
        bool leftOk, rightOk;
        int leftTableNum = leftTable.toInt(&leftOk);
        int rightTableNum = rightTable.toInt(&rightOk);

        if (leftOk && rightOk)
        {
            return leftTableNum < rightTableNum;
        }
        else
        {
            // Fallback to string comparison if not numeric
            return leftTable < rightTable;
        }
    }

    // If tables are the same, sort by seat (column 1: "Seat")
    QString leftSeat = source->data(source->index(left.row(), 1), Qt::DisplayRole).toString();
    QString rightSeat = source->data(source->index(right.row(), 1), Qt::DisplayRole).toString();

    // Convert to int for proper numeric sorting
    bool leftOk, rightOk;
    int leftSeatNum = leftSeat.toInt(&leftOk);
    int rightSeatNum = rightSeat.toInt(&rightOk);

    if (leftOk && rightOk)
    {
        return leftSeatNum < rightSeatNum;
    }
    else
    {
        // Fallback to string comparison if not numeric
        return leftSeat < rightSeat;
    }
}
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
    // Use UserRole to get numeric table_number and seat_number values
    
    // Get numeric table numbers (column 0 with UserRole)
    QVariant leftTableVar = source->data(source->index(left.row(), 0), Qt::UserRole);
    QVariant rightTableVar = source->data(source->index(right.row(), 0), Qt::UserRole);
    
    if (leftTableVar.isValid() && rightTableVar.isValid())
    {
        qulonglong leftTableNum = leftTableVar.toULongLong();
        qulonglong rightTableNum = rightTableVar.toULongLong();
        
        if (leftTableNum != rightTableNum)
        {
            return leftTableNum < rightTableNum;
        }
        
        // If table numbers are the same, sort by seat number (column 1 with UserRole)
        QVariant leftSeatVar = source->data(source->index(left.row(), 1), Qt::UserRole);
        QVariant rightSeatVar = source->data(source->index(right.row(), 1), Qt::UserRole);
        
        if (leftSeatVar.isValid() && rightSeatVar.isValid())
        {
            qulonglong leftSeatNum = leftSeatVar.toULongLong();
            qulonglong rightSeatNum = rightSeatVar.toULongLong();
            
            return leftSeatNum < rightSeatNum;
        }
    }
    
    // Fallback to default sorting if UserRole data is not available
    return QSortFilterProxyModel::lessThan(left, right);
}
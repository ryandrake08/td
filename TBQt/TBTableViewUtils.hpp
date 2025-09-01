#pragma once

#include <Qt>

class QSortFilterProxyModel;
class QTableView;
class QWidget;
class TBVariantListTableModel;

class TBTableViewUtils
{
public:
    // Helper function to set up table view with sorting proxy model
    static QSortFilterProxyModel* setupTableViewWithSorting(QWidget* parent, QTableView* tableView, TBVariantListTableModel* sourceModel, int defaultSortColumn = 0, Qt::SortOrder defaultSortOrder = Qt::AscendingOrder);

    // Helper function to get selected source row from table view with proxy model
    static int getSelectedSourceRow(QTableView* tableView);
};
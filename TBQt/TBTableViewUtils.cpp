#include "TBTableViewUtils.hpp"

#include "TBVariantListTableModel.hpp"

#include <QSortFilterProxyModel>
#include <QTableView>

QSortFilterProxyModel* TBTableViewUtils::setupTableViewWithSorting(QWidget* parent, QTableView* tableView, TBVariantListTableModel* sourceModel, int defaultSortColumn, Qt::SortOrder defaultSortOrder)
{
    // Create proxy model for sorting
    auto* proxyModel = new QSortFilterProxyModel(parent);
    proxyModel->setSourceModel(sourceModel);

    // Set up the table view
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);

    // Set default sort if requested
    if(defaultSortColumn >= 0)
    {
        tableView->sortByColumn(defaultSortColumn, defaultSortOrder);
    }
    else
    {
        // Reset to unsorted state by sorting by an invalid column
        proxyModel->sort(-1);
    }

    return proxyModel;
}

int TBTableViewUtils::getSelectedSourceRow(QTableView* tableView)
{
    QModelIndexList selectedRows = tableView->selectionModel()->selectedRows();
    if(selectedRows.isEmpty())
        return -1;

    // Get the proxy model from the table view
    auto* proxyModel = qobject_cast<QSortFilterProxyModel*>(tableView->model());
    if(!proxyModel)
        return -1; // Fallback if not using proxy model

    // Map from proxy to source model
    QModelIndex proxyIndex = selectedRows.first();
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    return sourceIndex.row();
}
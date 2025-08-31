#include "TBSetupTabWidget.hpp"

#include "TBVariantListTableModel.hpp"

#include <QSortFilterProxyModel>
#include <QTableView>

TBSetupTabWidget::TBSetupTabWidget(QWidget* parent) : QWidget(parent)
{
}

TBSetupTabWidget::~TBSetupTabWidget()
{
}

void TBSetupTabWidget::setupTableViewWithSorting(QTableView* tableView, TBVariantListTableModel* sourceModel, int defaultSortColumn, Qt::SortOrder defaultSortOrder)
{
    // Create proxy model for sorting
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(sourceModel);

    // Set up the table view
    tableView->setModel(proxyModel);
    tableView->setSortingEnabled(true);

    // Set default sort if requested
    if (defaultSortColumn >= 0) {
        tableView->sortByColumn(defaultSortColumn, defaultSortOrder);
    }
}

int TBSetupTabWidget::getSelectedSourceRow(QTableView* tableView)
{
    QModelIndexList selectedRows = tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return -1;

    // Get the proxy model from the table view
    QSortFilterProxyModel* proxyModel = qobject_cast<QSortFilterProxyModel*>(tableView->model());
    if (!proxyModel)
        return -1; // Fallback if not using proxy model

    // Map from proxy to source model
    QModelIndex proxyIndex = selectedRows.first();
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    return sourceIndex.row();
}
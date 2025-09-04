#include "TBSetupTablesWidget.hpp"

#include "TBTableViewUtils.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupTablesWidget.h"

#include <QHeaderView>
#include <QSortFilterProxyModel>

struct TBSetupTablesWidget::impl
{
    Ui::TBSetupTablesWidget ui;
    TBVariantListTableModel* model;

    int nextTableNumber;

    impl() : nextTableNumber(1) {}
};

TBSetupTablesWidget::TBSetupTablesWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addIndexHeader("table_number", tr("Table #"), 1);
    pimpl->model->addHeader("table_name", tr("Table Name"));

    // Set up table view with sorting
    TBTableViewUtils::setupTableViewWithSorting(this, pimpl->ui.tableView, pimpl->model, 0, Qt::AscendingOrder);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Table #: fit content
    header->setSectionResizeMode(1, QHeaderView::Stretch);          // Table Name: stretch to fill

    // Populate players per table dropdown (2-12)
    for(int i = 2; i <= 12; i++)
    {
        pimpl->ui.playersPerTableComboBox->addItem(QString::number(i), i);
    }
    pimpl->ui.playersPerTableComboBox->setCurrentIndex(6); // Default to 8 players per table

    // Connect signals
    QObject::connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupTablesWidget::on_addTableButtonClicked);
    QObject::connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupTablesWidget::on_removeTableButtonClicked);
    QObject::connect(pimpl->ui.playersPerTableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TBSetupTablesWidget::on_playersPerTableChanged);
    QObject::connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupTablesWidget::on_modelDataChanged);

    // Connect selection model after setting the model
    QObject::connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
                     [this]()
    {
        bool hasSelection = pimpl->ui.tableView->selectionModel()->hasSelection();
        pimpl->ui.removeButton->setEnabled(hasSelection);
    });
}

TBSetupTablesWidget::~TBSetupTablesWidget()
{
}

void TBSetupTablesWidget::setConfiguration(const QVariantMap& configuration)
{
    QVariantList tables = configuration.value("available_tables").toList();
    pimpl->model->setListData(tables);

    // Update next table number based on current count
    pimpl->nextTableNumber = tables.size() + 1;

    // Set players per table if available
    int playersPerTable = configuration.value("players_per_table", 8).toInt();
    int index = pimpl->ui.playersPerTableComboBox->findData(playersPerTable);
    if(index >= 0)
    {
        pimpl->ui.playersPerTableComboBox->setCurrentIndex(index);
    }
}

QVariantMap TBSetupTablesWidget::configuration() const
{
    QVariantMap config;
    config["available_tables"] = pimpl->model->listData();
    config["players_per_table"] = pimpl->ui.playersPerTableComboBox->currentData().toInt();
    return config;
}

bool TBSetupTablesWidget::validateConfiguration() const
{
    // At least one table is required
    return pimpl->model->rowCount() > 0;
}

void TBSetupTablesWidget::on_addTableButtonClicked()
{
    QVariantMap newTable;
    newTable["table_name"] = tr("Table %1").arg(pimpl->nextTableNumber);

    pimpl->nextTableNumber++;

    // Add to model
    QVariantList tables = pimpl->model->listData();
    tables.append(newTable);
    pimpl->model->setListData(tables);

    Q_EMIT configurationChanged();
}

void TBSetupTablesWidget::on_removeTableButtonClicked()
{
    int row = TBTableViewUtils::getSelectedSourceRow(pimpl->ui.tableView);
    if(row < 0)
        return;

    // Remove from model
    QVariantList tables = pimpl->model->listData();
    if(row >= 0 && row < tables.size())
    {
        tables.removeAt(row);
        pimpl->model->setListData(tables);
        Q_EMIT configurationChanged();
    }
}

void TBSetupTablesWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}

void TBSetupTablesWidget::on_playersPerTableChanged()
{
    Q_EMIT configurationChanged();
}
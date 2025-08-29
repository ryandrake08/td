#include "TBSetupTablesWidget.hpp"

#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupTablesWidget.h"

#include <QHeaderView>

struct TBSetupTablesWidget::impl
{
    Ui::TBSetupTablesWidget ui;
    TBVariantListTableModel* model;
    
    int nextTableNumber;
    
    impl() : nextTableNumber(1) {}
};

TBSetupTablesWidget::TBSetupTablesWidget(QWidget* parent)
    : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);
    
    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("table_name", tr("Table Name"));
    
    pimpl->ui.tableView->setModel(pimpl->model);
    
    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch); // Table Name: stretch to fill
    
    // Connect signals
    connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupTablesWidget::on_addTableButtonClicked);
    connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupTablesWidget::on_removeTableButtonClicked);
    connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupTablesWidget::on_modelDataChanged);
    
    // Connect selection model after setting the model
    connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
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
    
    // Update next table number based on existing tables
    pimpl->nextTableNumber = 1;
    for (const QVariant& tableVariant : tables)
    {
        QVariantMap table = tableVariant.toMap();
        QString name = table.value("table_name").toString();
        
        // Check if name matches pattern "Table X"
        if (name.startsWith("Table "))
        {
            QString numberPart = name.mid(6); // Remove "Table " prefix
            bool ok;
            int number = numberPart.toInt(&ok);
            if (ok && number >= pimpl->nextTableNumber)
            {
                pimpl->nextTableNumber = number + 1;
            }
        }
    }
}

QVariantMap TBSetupTablesWidget::configuration() const
{
    QVariantMap config;
    config["available_tables"] = pimpl->model->listData();
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
    QModelIndexList selectedRows = pimpl->ui.tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return;
    
    // Get the row to remove (take the first selected row)
    int row = selectedRows.first().row();
    
    // Remove from model
    QVariantList tables = pimpl->model->listData();
    if (row >= 0 && row < tables.size())
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
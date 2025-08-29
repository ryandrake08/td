#include "TBSetupFundingWidget.hpp"

#include "TBFundingTypeDelegate.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupFundingWidget.h"

#include <QHeaderView>

struct TBSetupFundingWidget::impl
{
    Ui::TBSetupFundingWidget ui;
    TBVariantListTableModel* model;
};

TBSetupFundingWidget::TBSetupFundingWidget(QWidget* parent)
    : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("name", tr("Name"));
    pimpl->model->addHeader("type", tr("Type"));
    pimpl->model->addHeader("chips", tr("Chips"));
    pimpl->model->addHeader("cost_amount", tr("Cost"));
    pimpl->model->addHeader("cost_currency", tr("Currency"));
    pimpl->model->addHeader("forbid_after_blind_level", tr("Forbid After Level"));

    pimpl->ui.tableView->setModel(pimpl->model);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);          // Name
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Type
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Chips
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Cost
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Currency
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Forbid After Level

    // Set funding type delegate for Type column (column 1)
    pimpl->ui.tableView->setItemDelegateForColumn(1, new TBFundingTypeDelegate(this));

    // Connect signals
    connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupFundingWidget::on_addFundingButtonClicked);
    connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupFundingWidget::on_removeFundingButtonClicked);
    connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupFundingWidget::on_modelDataChanged);

    // Connect selection model after setting the model
    connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                bool hasSelection = pimpl->ui.tableView->selectionModel()->hasSelection();
                pimpl->ui.removeButton->setEnabled(hasSelection);
            });
}

TBSetupFundingWidget::~TBSetupFundingWidget()
{
}

void TBSetupFundingWidget::setConfiguration(const QVariantMap& configuration)
{
    QVariantList funding = configuration.value("funding_sources").toList();

    // Flatten nested cost/commission/equity structures for display
    QVariantList displayFunding;
    for (const QVariant& fundingVariant : funding)
    {
        QVariantMap fundingItem = fundingVariant.toMap();
        QVariantMap displayItem;

        displayItem["name"] = fundingItem["name"];
        displayItem["type"] = fundingItem["type"];
        displayItem["chips"] = fundingItem["chips"];
        displayItem["forbid_after_blind_level"] = fundingItem["forbid_after_blind_level"];

        // Flatten cost structure
        QVariantMap cost = fundingItem["cost"].toMap();
        displayItem["cost_amount"] = cost["amount"];
        displayItem["cost_currency"] = cost["currency"];

        displayFunding.append(displayItem);
    }

    pimpl->model->setListData(displayFunding);
}

QVariantMap TBSetupFundingWidget::configuration() const
{
    QVariantMap config;

    // Reconstruct nested structures from flattened display data
    QVariantList displayFunding = pimpl->model->listData();
    QVariantList funding;

    for (const QVariant& displayVariant : displayFunding)
    {
        QVariantMap displayItem = displayVariant.toMap();
        QVariantMap fundingItem;

        fundingItem["name"] = displayItem["name"];
        fundingItem["type"] = displayItem["type"];
        fundingItem["chips"] = displayItem["chips"];
        fundingItem["forbid_after_blind_level"] = displayItem["forbid_after_blind_level"];

        // Create nested cost structure
        QVariantMap cost;
        cost["amount"] = displayItem["cost_amount"];
        cost["currency"] = displayItem["cost_currency"];
        fundingItem["cost"] = cost;

        // Default empty commission and equity
        QVariantMap commission;
        commission["amount"] = 0.0;
        commission["currency"] = displayItem["cost_currency"];
        fundingItem["commission"] = commission;

        QVariantMap equity;
        equity["amount"] = 0.0;
        fundingItem["equity"] = equity;

        funding.append(fundingItem);
    }

    config["funding_sources"] = funding;
    return config;
}

bool TBSetupFundingWidget::validateConfiguration() const
{
    // At least one funding source (buy-in) is required
    QVariantList funding = pimpl->model->listData();

    for (const QVariant& fundingVariant : funding)
    {
        QVariantMap fundingItem = fundingVariant.toMap();
        if (fundingItem["type"].toInt() == 0) // Buy-in type
        {
            return true;
        }
    }

    return false; // No buy-in found
}

void TBSetupFundingWidget::on_addFundingButtonClicked()
{
    QVariantMap newFunding = createDefaultFunding(0); // Default to Buy-in

    // Add to model
    QVariantList funding = pimpl->model->listData();
    funding.append(newFunding);
    pimpl->model->setListData(funding);

    Q_EMIT configurationChanged();
}

void TBSetupFundingWidget::on_removeFundingButtonClicked()
{
    QModelIndexList selectedRows = pimpl->ui.tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return;

    // Get the row to remove (take the first selected row)
    int row = selectedRows.first().row();

    // Remove from model
    QVariantList funding = pimpl->model->listData();
    if (row >= 0 && row < funding.size())
    {
        funding.removeAt(row);
        pimpl->model->setListData(funding);
        Q_EMIT configurationChanged();
    }
}

void TBSetupFundingWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}

QVariantMap TBSetupFundingWidget::createDefaultFunding(int fundingType) const
{
    QVariantMap funding;
    funding["name"] = (fundingType == 0) ? tr("Buy-in") :
                     (fundingType == 1) ? tr("Rebuy") : tr("Add-on");
    funding["type"] = fundingType;
    funding["chips"] = 1500; // Default chip count
    funding["cost_amount"] = 20.0; // Default cost
    funding["cost_currency"] = getDefaultCurrency();
    funding["forbid_after_blind_level"] = 999; // No restriction by default
    return funding;
}

QString TBSetupFundingWidget::getDefaultCurrency() const
{
    return "USD"; // Default currency
}
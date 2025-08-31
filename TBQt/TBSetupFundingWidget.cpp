#include "TBSetupFundingWidget.hpp"
#include "TournamentSession.hpp"

#include "TBBlindLevelDelegate.hpp"
#include "TBCurrency.hpp"
#include "TBFundingDetailsDelegate.hpp"
#include "TBFundingTypeDelegate.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupFundingWidget.h"

#include <QHeaderView>

struct TBSetupFundingWidget::impl
{
    Ui::TBSetupFundingWidget ui;
    TBVariantListTableModel* model;
    TBBlindLevelDelegate* blindLevelDelegate;
};

TBSetupFundingWidget::TBSetupFundingWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("name", tr("Name"));
    pimpl->model->addHeader("type", tr("Type"));
    pimpl->model->addHeader("chips", tr("Chips"));
    pimpl->model->addHeader("cost_amount", tr("Details"));
    pimpl->model->addHeader("forbid_after_blind_level", tr("Forbid After Level"));

    pimpl->ui.tableView->setModel(pimpl->model);

    // Enable sorting and set default sort by name
    pimpl->ui.tableView->sortByColumn(0, Qt::AscendingOrder);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);          // Name
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Type
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Chips
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Details
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Forbid After Level

    // Set funding type delegate for Type column (column 1)
    pimpl->ui.tableView->setItemDelegateForColumn(1, new TBFundingTypeDelegate(this));

    // Set funding details delegate for Details column (column 3)
    pimpl->ui.tableView->setItemDelegateForColumn(3, new TBFundingDetailsDelegate(this));

    // Set blind level delegate for Forbid After Level column (column 4)
    pimpl->blindLevelDelegate = new TBBlindLevelDelegate(this);
    pimpl->ui.tableView->setItemDelegateForColumn(4, pimpl->blindLevelDelegate);

    // Setup payout currency dropdown
    pimpl->ui.payoutCurrencyComboBox->addItems(TBCurrency::supportedCodes());
    pimpl->ui.payoutCurrencyComboBox->setCurrentText(TBCurrency::defaultCurrencyCode());

    // Connect signals
    connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupFundingWidget::on_addFundingButtonClicked);
    connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupFundingWidget::on_removeFundingButtonClicked);
    connect(pimpl->ui.payoutCurrencyComboBox, QOverload<const QString&>::of(&QComboBox::currentTextChanged), this, &TBSetupFundingWidget::on_modelDataChanged);
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

        // Store cost amount for display, keep full data for delegate
        QVariantMap cost = fundingItem["cost"].toMap();
        displayItem["cost_amount"] = cost["amount"];

        // Store complete funding data for details delegate
        displayItem["cost_currency"] = cost["currency"];
        displayItem["commission_amount"] = fundingItem.value("commission").toMap().value("amount", 0.0);
        displayItem["commission_currency"] = fundingItem.value("commission").toMap().value("currency", TBCurrency::defaultCurrencyCode());
        displayItem["equity_amount"] = fundingItem.value("equity").toMap().value("amount", 0.0);

        displayFunding.append(displayItem);
    }

    pimpl->model->setListData(displayFunding);

    // Set payout currency dropdown
    QString payoutCurrency = configuration.value("payout_currency", TBCurrency::defaultCurrencyCode()).toString();
    pimpl->ui.payoutCurrencyComboBox->setCurrentText(payoutCurrency);
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
        // Only include forbid_after_blind_level if it's a valid value (not "Never")
        QVariant forbidLevel = displayItem["forbid_after_blind_level"];
        if (forbidLevel.isValid() && !forbidLevel.isNull() && forbidLevel.toInt() >= 0)
        {
            fundingItem["forbid_after_blind_level"] = forbidLevel;
        }

        // Create nested cost structure
        QVariantMap cost;
        cost["amount"] = displayItem["cost_amount"];
        cost["currency"] = displayItem["cost_currency"];
        fundingItem["cost"] = cost;

        // Create commission structure
        QVariantMap commission;
        commission["amount"] = displayItem["commission_amount"];
        commission["currency"] = displayItem["commission_currency"];
        fundingItem["commission"] = commission;

        // Create equity structure
        QVariantMap equity;
        equity["amount"] = displayItem["equity_amount"];
        fundingItem["equity"] = equity;

        funding.append(fundingItem);
    }

    config["funding_sources"] = funding;
    config["payout_currency"] = pimpl->ui.payoutCurrencyComboBox->currentText();
    return config;
}

bool TBSetupFundingWidget::validateConfiguration() const
{
    // At least one funding source (buy-in) is required
    QVariantList funding = pimpl->model->listData();

    for (const QVariant& fundingVariant : funding)
    {
        QVariantMap fundingItem = fundingVariant.toMap();
        if (fundingItem["type"].toInt() == TournamentSession::kFundingTypeBuyin)
        {
            return true;
        }
    }

    return false; // No buy-in found
}

void TBSetupFundingWidget::on_addFundingButtonClicked()
{
    QVariantMap newFunding = createDefaultFunding(TournamentSession::kFundingTypeBuyin);

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
    funding["cost_currency"] = TBCurrency::defaultCurrencyCode();
    funding["commission_amount"] = 0.0;
    funding["commission_currency"] = TBCurrency::defaultCurrencyCode();
    funding["equity_amount"] = 20.0;
    // Don't set forbid_after_blind_level - missing key means "Never"
    return funding;
}


void TBSetupFundingWidget::setRoundsData(const QVariantList& rounds)
{
    pimpl->blindLevelDelegate->setBlindLevels(rounds);
}
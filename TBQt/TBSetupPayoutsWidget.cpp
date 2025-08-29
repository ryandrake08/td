#include "TBSetupPayoutsWidget.hpp"

#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupPayoutsWidget.h"

#include <QHeaderView>

struct TBSetupPayoutsWidget::impl
{
    Ui::TBSetupPayoutsWidget ui;
    TBVariantListTableModel* manualModel;
    TBVariantListTableModel* turnoutModel;
    TBVariantListTableModel* turnoutPayoutsModel;
};

TBSetupPayoutsWidget::TBSetupPayoutsWidget(QWidget* parent)
    : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);
    
    // Create and configure manual payouts model
    pimpl->manualModel = new TBVariantListTableModel(this);
    pimpl->manualModel->addHeader("place", tr("Place"));
    pimpl->manualModel->addHeader("amount", tr("Amount"));
    
    pimpl->ui.manualTableView->setModel(pimpl->manualModel);
    
    // Configure manual table columns
    QHeaderView* manualHeader = pimpl->ui.manualTableView->horizontalHeader();
    manualHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Place
    manualHeader->setSectionResizeMode(1, QHeaderView::Stretch);          // Amount
    
    // Create and configure turnout levels model
    pimpl->turnoutModel = new TBVariantListTableModel(this);
    pimpl->turnoutModel->addHeader("buyins_count", tr("Buy-ins"));
    
    pimpl->ui.turnoutTableView->setModel(pimpl->turnoutModel);
    
    // Create and configure turnout payouts model
    pimpl->turnoutPayoutsModel = new TBVariantListTableModel(this);
    pimpl->turnoutPayoutsModel->addHeader("place", tr("Place"));
    pimpl->turnoutPayoutsModel->addHeader("amount", tr("Amount"));
    
    pimpl->ui.turnoutPayoutsTableView->setModel(pimpl->turnoutPayoutsModel);
    
    // Connect signals for manual tab
    connect(pimpl->ui.addPayoutButton, &QPushButton::clicked, this, &TBSetupPayoutsWidget::on_addPayoutButtonClicked);
    connect(pimpl->ui.removePayoutButton, &QPushButton::clicked, this, &TBSetupPayoutsWidget::on_removePayoutButtonClicked);
    
    // Connect signals for turnout tab
    connect(pimpl->ui.addTurnoutButton, &QPushButton::clicked, this, &TBSetupPayoutsWidget::on_addTurnoutButtonClicked);
    connect(pimpl->ui.removeTurnoutButton, &QPushButton::clicked, this, &TBSetupPayoutsWidget::on_removeTurnoutButtonClicked);
    
    // Connect selection models
    connect(pimpl->ui.manualTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                bool hasSelection = pimpl->ui.manualTableView->selectionModel()->hasSelection();
                pimpl->ui.removePayoutButton->setEnabled(hasSelection);
            });
            
    connect(pimpl->ui.turnoutTableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TBSetupPayoutsWidget::on_turnoutSelectionChanged);
    
    // Connect data change signals
    connect(pimpl->manualModel, &QAbstractItemModel::dataChanged, this, &TBSetupPayoutsWidget::on_modelDataChanged);
    connect(pimpl->turnoutModel, &QAbstractItemModel::dataChanged, this, &TBSetupPayoutsWidget::on_modelDataChanged);
    connect(pimpl->turnoutPayoutsModel, &QAbstractItemModel::dataChanged, this, &TBSetupPayoutsWidget::on_modelDataChanged);
}

TBSetupPayoutsWidget::~TBSetupPayoutsWidget()
{
}

void TBSetupPayoutsWidget::setConfiguration(const QVariantMap& configuration)
{
    // Set manual payouts
    QVariantList manualPayouts = configuration.value("manual_payouts").toList();
    pimpl->manualModel->setListData(manualPayouts);
    
    // Set forced payouts
    QVariantList forcedPayouts = configuration.value("forced_payouts").toList();
    pimpl->turnoutModel->setListData(forcedPayouts);
}

QVariantMap TBSetupPayoutsWidget::configuration() const
{
    QVariantMap config;
    config["manual_payouts"] = pimpl->manualModel->listData();
    config["forced_payouts"] = pimpl->turnoutModel->listData();
    return config;
}

bool TBSetupPayoutsWidget::validateConfiguration() const
{
    // At least one payout structure is required
    return pimpl->manualModel->rowCount() > 0 || pimpl->turnoutModel->rowCount() > 0;
}

void TBSetupPayoutsWidget::on_addPayoutButtonClicked()
{
    int place = pimpl->manualModel->rowCount() + 1;
    QVariantMap newPayout = createDefaultPayout(place, 100.0);
    
    QVariantList payouts = pimpl->manualModel->listData();
    payouts.append(newPayout);
    pimpl->manualModel->setListData(payouts);
    
    Q_EMIT configurationChanged();
}

void TBSetupPayoutsWidget::on_removePayoutButtonClicked()
{
    QModelIndexList selectedRows = pimpl->ui.manualTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return;
    
    int row = selectedRows.first().row();
    QVariantList payouts = pimpl->manualModel->listData();
    if (row >= 0 && row < payouts.size())
    {
        payouts.removeAt(row);
        pimpl->manualModel->setListData(payouts);
        Q_EMIT configurationChanged();
    }
}

void TBSetupPayoutsWidget::on_addTurnoutButtonClicked()
{
    int buyinsCount = pimpl->turnoutModel->rowCount() * 10 + 10; // 10, 20, 30, etc.
    QVariantMap newLevel = createDefaultTurnoutLevel(buyinsCount);
    
    QVariantList levels = pimpl->turnoutModel->listData();
    levels.append(newLevel);
    pimpl->turnoutModel->setListData(levels);
    
    Q_EMIT configurationChanged();
}

void TBSetupPayoutsWidget::on_removeTurnoutButtonClicked()
{
    QModelIndexList selectedRows = pimpl->ui.turnoutTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return;
    
    int row = selectedRows.first().row();
    QVariantList levels = pimpl->turnoutModel->listData();
    if (row >= 0 && row < levels.size())
    {
        levels.removeAt(row);
        pimpl->turnoutModel->setListData(levels);
        Q_EMIT configurationChanged();
    }
}

void TBSetupPayoutsWidget::on_turnoutSelectionChanged()
{
    bool hasSelection = pimpl->ui.turnoutTableView->selectionModel()->hasSelection();
    pimpl->ui.removeTurnoutButton->setEnabled(hasSelection);
    
    // Update turnout payouts display based on selection
    updateTurnoutPayoutsDisplay();
}

void TBSetupPayoutsWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}

void TBSetupPayoutsWidget::updateTurnoutPayoutsDisplay()
{
    // Simplified implementation - show empty for now
    pimpl->turnoutPayoutsModel->setListData(QVariantList());
}

QVariantMap TBSetupPayoutsWidget::createDefaultPayout(int place, double amount) const
{
    QVariantMap payout;
    payout["place"] = place;
    payout["amount"] = amount;
    return payout;
}

QVariantMap TBSetupPayoutsWidget::createDefaultTurnoutLevel(int buyinsCount) const
{
    QVariantMap level;
    level["buyins_count"] = buyinsCount;
    level["payouts"] = QVariantList(); // Empty payouts list
    return level;
}
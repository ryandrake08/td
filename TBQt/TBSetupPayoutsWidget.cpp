#include "TBSetupPayoutsWidget.hpp"
#include "TournamentSession.hpp"

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

TBSetupPayoutsWidget::TBSetupPayoutsWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure manual payouts model
    pimpl->manualModel = new TBVariantListTableModel(this);
    pimpl->manualModel->addIndexHeader("place", tr("Place"), 1);
    pimpl->manualModel->addHeader("payout", tr("Payout"));
    pimpl->manualModel->addHeader("currency", tr("Currency"));

    pimpl->ui.manualTableView->setModel(pimpl->manualModel);

    // Enable sorting for manual payouts
    pimpl->ui.manualTableView->sortByColumn(0, Qt::AscendingOrder);

    // Configure manual table columns
    QHeaderView* manualHeader = pimpl->ui.manualTableView->horizontalHeader();
    manualHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Place
    manualHeader->setSectionResizeMode(1, QHeaderView::Stretch);          // Payout
    manualHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Currency

    // Create and configure turnout levels model
    pimpl->turnoutModel = new TBVariantListTableModel(this);
    pimpl->turnoutModel->addHeader("buyins_count", tr("Buy-ins"));

    pimpl->ui.turnoutTableView->setModel(pimpl->turnoutModel);

    // Enable sorting for turnout levels
    pimpl->ui.turnoutTableView->sortByColumn(0, Qt::AscendingOrder);

    // Create and configure turnout payouts model
    pimpl->turnoutPayoutsModel = new TBVariantListTableModel(this);
    pimpl->turnoutPayoutsModel->addIndexHeader("place", tr("Place"), 1);
    pimpl->turnoutPayoutsModel->addHeader("payout", tr("Payout"));
    pimpl->turnoutPayoutsModel->addHeader("currency", tr("Currency"));

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

    // Connect automatic payout signals
    connect(pimpl->ui.percentSeatsPaidSlider, &QSlider::valueChanged, this, &TBSetupPayoutsWidget::on_percentSeatsSliderChanged);
    connect(pimpl->ui.percentSeatsPaidSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TBSetupPayoutsWidget::on_percentSeatsSpinBoxChanged);
    connect(pimpl->ui.payoutShapeSlider, &QSlider::valueChanged, this, &TBSetupPayoutsWidget::on_payoutShapeChanged);
    connect(pimpl->ui.payTheBubbleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TBSetupPayoutsWidget::on_modelDataChanged);
    connect(pimpl->ui.payKnockoutsSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &TBSetupPayoutsWidget::on_modelDataChanged);
    connect(pimpl->ui.roundPayoutsCheckBox, &QCheckBox::toggled, this, &TBSetupPayoutsWidget::on_modelDataChanged);

    // Connect tab selection to update payout policy
    connect(pimpl->ui.payoutTabWidget, &QTabWidget::currentChanged, this, &TBSetupPayoutsWidget::on_payoutTabChanged);

    // Initialize payout shape description
    pimpl->ui.payoutShapeDescriptionLabel->setText(payoutShapeDescription(0.2));

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
    // Set payout policy tab selection
    int payoutPolicy = configuration.value("payout_policy", 0).toInt();
    pimpl->ui.payoutTabWidget->setCurrentIndex(payoutPolicy);

    // Set automatic payout parameters
    QVariantMap automaticPayouts = configuration.value("automatic_payouts").toMap();
    int percentSeats = static_cast<int>(automaticPayouts.value("percent_seats_paid", 0.15).toDouble() * 100);
    pimpl->ui.percentSeatsPaidSlider->setValue(percentSeats);
    pimpl->ui.percentSeatsPaidSpinBox->setValue(percentSeats);

    int shapeValue = static_cast<int>(automaticPayouts.value("payout_shape", 0.2).toDouble() * 100);
    pimpl->ui.payoutShapeSlider->setValue(shapeValue);
    pimpl->ui.payoutShapeDescriptionLabel->setText(payoutShapeDescription(shapeValue / 100.0));

    pimpl->ui.payTheBubbleSpinBox->setValue(automaticPayouts.value("pay_the_bubble", 0.0).toDouble());
    pimpl->ui.payKnockoutsSpinBox->setValue(automaticPayouts.value("pay_knockouts", 0.0).toDouble());
    pimpl->ui.roundPayoutsCheckBox->setChecked(automaticPayouts.value("round_payouts", false).toBool());

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

    // Set payout policy based on current tab
    config["payout_policy"] = pimpl->ui.payoutTabWidget->currentIndex();

    // Build automatic payout parameters
    QVariantMap automaticPayouts;
    automaticPayouts["percent_seats_paid"] = pimpl->ui.percentSeatsPaidSlider->value() / 100.0;
    automaticPayouts["payout_shape"] = pimpl->ui.payoutShapeSlider->value() / 100.0;
    automaticPayouts["pay_the_bubble"] = pimpl->ui.payTheBubbleSpinBox->value();
    automaticPayouts["pay_knockouts"] = pimpl->ui.payKnockoutsSpinBox->value();
    automaticPayouts["round_payouts"] = pimpl->ui.roundPayoutsCheckBox->isChecked();
    config["automatic_payouts"] = automaticPayouts;

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
    QVariantMap newPayout = createDefaultPayout(100.0);

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
    QModelIndexList selectedRows = pimpl->ui.turnoutTableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
    {
        pimpl->turnoutPayoutsModel->setListData(QVariantList());
        return;
    }

    int row = selectedRows.first().row();
    QVariantList turnoutLevels = pimpl->turnoutModel->listData();
    if (row < 0 || row >= turnoutLevels.size())
    {
        pimpl->turnoutPayoutsModel->setListData(QVariantList());
        return;
    }

    QVariantMap selectedLevel = turnoutLevels[row].toMap();
    QVariantList payouts = selectedLevel.value("payouts").toList();
    pimpl->turnoutPayoutsModel->setListData(payouts);
}

QVariantMap TBSetupPayoutsWidget::createDefaultPayout(double amount) const
{
    QVariantMap payout;
    payout["payout"] = amount;
    payout["currency"] = "USD";
    return payout;
}

QVariantMap TBSetupPayoutsWidget::createDefaultTurnoutLevel(int buyinsCount) const
{
    QVariantMap level;
    level["buyins_count"] = buyinsCount;
    level["payouts"] = QVariantList(); // Empty payouts list
    return level;
}

void TBSetupPayoutsWidget::on_percentSeatsSliderChanged(int value)
{
    pimpl->ui.percentSeatsPaidSpinBox->setValue(value);
    on_modelDataChanged();
}

void TBSetupPayoutsWidget::on_percentSeatsSpinBoxChanged(int value)
{
    pimpl->ui.percentSeatsPaidSlider->setValue(value);
    on_modelDataChanged();
}

void TBSetupPayoutsWidget::on_payoutShapeChanged(int value)
{
    double shape = value / 100.0;
    pimpl->ui.payoutShapeDescriptionLabel->setText(payoutShapeDescription(shape));
    on_modelDataChanged();
}

void TBSetupPayoutsWidget::on_payoutTabChanged(int index)
{
    // Update payout_policy based on selected tab
    // 0 = Automatic, 1 = Manual, 2 = Depends on Turnout
    // We only handle Automatic here - other tabs would need their own handling
    if (index == 0) // Automatic tab selected
    {
        // This will be handled in configuration() method
    }
    on_modelDataChanged();
}

QString TBSetupPayoutsWidget::payoutShapeDescription(double shape) const
{
    if (shape <= 0.0) {
        return tr("Same To Everyone");
    } else if (shape < 0.25) {
        return tr("Relatively Flat");
    } else if (shape < 0.5) {
        return tr("Balanced");
    } else if (shape < 0.75) {
        return tr("Top Heavy");
    } else if (shape < 1.0) {
        return tr("Reward Deep");
    } else {
        return tr("Winner Takes All");
    }
}
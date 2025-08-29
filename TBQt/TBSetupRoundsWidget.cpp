#include "TBSetupRoundsWidget.hpp"

#include "TBAnteTypeDelegate.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupRoundsWidget.h"

#include <QHeaderView>
#include <QInputDialog>

struct TBSetupRoundsWidget::impl
{
    Ui::TBSetupRoundsWidget ui;
    TBVariantListTableModel* model;
};

TBSetupRoundsWidget::TBSetupRoundsWidget(QWidget* parent)
    : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);
    
    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("little_blind", tr("Small Blind"));
    pimpl->model->addHeader("big_blind", tr("Big Blind"));
    pimpl->model->addHeader("ante", tr("Ante"));
    pimpl->model->addHeader("ante_type", tr("Ante Type"));
    pimpl->model->addHeader("duration", tr("Duration (min)"));
    pimpl->model->addHeader("break_duration", tr("Break (min)"));
    pimpl->model->addHeader("reason", tr("Break Reason"));
    
    pimpl->ui.tableView->setModel(pimpl->model);
    
    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Small Blind
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Big Blind
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Ante
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Ante Type
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Duration
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Break
    header->setSectionResizeMode(6, QHeaderView::Stretch);          // Break Reason
    
    // Set ante type delegate for Ante Type column (column 3)
    pimpl->ui.tableView->setItemDelegateForColumn(3, new TBAnteTypeDelegate(this));
    
    // Connect signals
    connect(pimpl->ui.addRoundButton, &QPushButton::clicked, this, &TBSetupRoundsWidget::on_addRoundButtonClicked);
    connect(pimpl->ui.addBreakButton, &QPushButton::clicked, this, &TBSetupRoundsWidget::on_addBreakButtonClicked);
    connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupRoundsWidget::on_removeButtonClicked);
    connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupRoundsWidget::on_modelDataChanged);
    
    // Connect selection model after setting the model
    connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                bool hasSelection = pimpl->ui.tableView->selectionModel()->hasSelection();
                pimpl->ui.removeButton->setEnabled(hasSelection);
            });
}

TBSetupRoundsWidget::~TBSetupRoundsWidget()
{
}

void TBSetupRoundsWidget::setConfiguration(const QVariantMap& configuration)
{
    QVariantList rounds = configuration.value("blind_levels").toList();
    
    // Filter out setup rounds (rounds with negative duration)
    QVariantList displayRounds;
    for (const QVariant& roundVariant : rounds)
    {
        QVariantMap round = roundVariant.toMap();
        int duration = round.value("duration").toInt();
        if (duration >= 0) // Only show non-setup rounds
        {
            displayRounds.append(round);
        }
    }
    
    pimpl->model->setListData(displayRounds);
}

QVariantMap TBSetupRoundsWidget::configuration() const
{
    QVariantMap config;
    config["blind_levels"] = pimpl->model->listData();
    return config;
}

bool TBSetupRoundsWidget::validateConfiguration() const
{
    // At least one round is required
    QVariantList rounds = pimpl->model->listData();
    return !rounds.isEmpty();
}

void TBSetupRoundsWidget::on_addRoundButtonClicked()
{
    int nextBlindLevel = calculateNextBlindLevel();
    QVariantMap newRound = createDefaultRound(nextBlindLevel / 2, nextBlindLevel);
    
    // Add to model
    QVariantList rounds = pimpl->model->listData();
    rounds.append(newRound);
    pimpl->model->setListData(rounds);
    
    Q_EMIT configurationChanged();
}

void TBSetupRoundsWidget::on_addBreakButtonClicked()
{
    bool ok;
    int duration = QInputDialog::getInt(this, tr("Add Break"), 
                                       tr("Break duration (minutes):"), 
                                       15, 1, 120, 1, &ok);
    if (!ok)
        return;
        
    QString reason = QInputDialog::getText(this, tr("Add Break"),
                                          tr("Break reason:"), 
                                          QLineEdit::Normal, 
                                          tr("Break"), &ok);
    if (!ok)
        return;
    
    QVariantMap newBreak = createBreakRound(duration, reason);
    
    // Add to model
    QVariantList rounds = pimpl->model->listData();
    rounds.append(newBreak);
    pimpl->model->setListData(rounds);
    
    Q_EMIT configurationChanged();
}

void TBSetupRoundsWidget::on_removeButtonClicked()
{
    QModelIndexList selectedRows = pimpl->ui.tableView->selectionModel()->selectedRows();
    if (selectedRows.isEmpty())
        return;
    
    // Get the row to remove (take the first selected row)
    int row = selectedRows.first().row();
    
    // Remove from model
    QVariantList rounds = pimpl->model->listData();
    if (row >= 0 && row < rounds.size())
    {
        rounds.removeAt(row);
        pimpl->model->setListData(rounds);
        Q_EMIT configurationChanged();
    }
}

void TBSetupRoundsWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}

QVariantMap TBSetupRoundsWidget::createDefaultRound(int littleBlind, int bigBlind) const
{
    QVariantMap round;
    round["little_blind"] = littleBlind;
    round["big_blind"] = bigBlind;
    round["ante"] = 0;
    round["ante_type"] = 0; // None
    round["duration"] = 20; // 20 minutes default
    round["break_duration"] = 0;
    round["reason"] = QString();
    return round;
}

QVariantMap TBSetupRoundsWidget::createBreakRound(int duration, const QString& reason) const
{
    QVariantMap breakRound;
    breakRound["little_blind"] = 0;
    breakRound["big_blind"] = 0;
    breakRound["ante"] = 0;
    breakRound["ante_type"] = 0;
    breakRound["duration"] = 0;
    breakRound["break_duration"] = duration;
    breakRound["reason"] = reason;
    return breakRound;
}

int TBSetupRoundsWidget::calculateNextBlindLevel() const
{
    QVariantList rounds = pimpl->model->listData();
    
    if (rounds.isEmpty())
        return 2; // Start with 1/2 blinds
    
    // Find the highest big blind
    int maxBigBlind = 0;
    for (const QVariant& roundVariant : rounds)
    {
        QVariantMap round = roundVariant.toMap();
        int bigBlind = round.value("big_blind").toInt();
        if (bigBlind > maxBigBlind)
        {
            maxBigBlind = bigBlind;
        }
    }
    
    // Double the highest big blind, or use standard progression
    if (maxBigBlind == 0) return 2;
    if (maxBigBlind < 10) return maxBigBlind * 2;
    
    // For higher levels, use more gradual increases
    return static_cast<int>(maxBigBlind * 1.5);
}

int TBSetupRoundsWidget::calculateCumulativeTime(int roundIndex) const
{
    QVariantList rounds = pimpl->model->listData();
    int totalTime = 0;
    
    for (int i = 0; i <= roundIndex && i < rounds.size(); i++)
    {
        QVariantMap round = rounds[i].toMap();
        totalTime += round.value("duration").toInt();
        totalTime += round.value("break_duration").toInt();
    }
    
    return totalTime;
}
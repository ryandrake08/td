#include "TBSetupRoundsWidget.hpp"

#include "TBAnteTypeDelegate.hpp"
#include "TBRoundsModel.hpp"

#include "TournamentSession.hpp"

#include "ui_TBSetupRoundsWidget.h"

#include <QHeaderView>
#include <QInputDialog>

struct TBSetupRoundsWidget::impl
{
    Ui::TBSetupRoundsWidget ui;
    TBRoundsModel* model;
};

TBSetupRoundsWidget::TBSetupRoundsWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBRoundsModel(this);
    pimpl->model->addIndexHeader("round_number", tr("Round"), 1); // Start from 1 since round 0 is filtered out
    pimpl->model->addHeader("start_time", tr("Start Time"));
    pimpl->model->addHeader("duration", tr("Duration"));
    pimpl->model->addHeader("little_blind", tr("Small Blind"));
    pimpl->model->addHeader("big_blind", tr("Big Blind"));
    pimpl->model->addHeader("ante", tr("Ante"));
    pimpl->model->addHeader("ante_type", tr("Ante Type"));
    pimpl->model->addHeader("break_duration", tr("Break"));
    pimpl->model->addHeader("reason", tr("Break Message"));

    pimpl->ui.tableView->setModel(pimpl->model);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Round #
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Start Time
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Duration
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Small Blind
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Big Blind
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Ante
    header->setSectionResizeMode(6, QHeaderView::ResizeToContents); // Ante Type
    header->setSectionResizeMode(7, QHeaderView::ResizeToContents); // Break
    header->setSectionResizeMode(8, QHeaderView::Stretch);          // Break Message

    // Set ante type delegate for Ante Type column (column 6)
    pimpl->ui.tableView->setItemDelegateForColumn(6, new TBAnteTypeDelegate(this));

    // Connect signals
    connect(pimpl->ui.addRoundButton, &QPushButton::clicked, this, &TBSetupRoundsWidget::on_addRoundButtonClicked);
    connect(pimpl->ui.generateButton, &QPushButton::clicked, this, &TBSetupRoundsWidget::on_generateButtonClicked);
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
    pimpl->model->setListData(rounds); // TBRoundsModel will handle filtering
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

void TBSetupRoundsWidget::on_generateButtonClicked()
{
    // TODO: Implement round auto-generation dialog
    // This should invoke the round generation dialog as seen in macOS client
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
    round["ante_type"] = TournamentSession::toInt(TournamentSession::AnteType::None);
    round["duration"] = 20; // 20 minutes default
    round["break_duration"] = 0; // No break by default
    round["reason"] = QString(); // Empty break reason
    return round;
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
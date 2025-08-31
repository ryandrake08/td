#include "TBSetupPlayersWidget.hpp"

#include "TBDateEditDelegate.hpp"
#include "TBTableViewUtils.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupPlayersWidget.h"

#include <QDateTime>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QUuid>

struct TBSetupPlayersWidget::impl
{
    Ui::TBSetupPlayersWidget ui;
    TBVariantListTableModel* model;

    int nextPlayerNumber;

    impl() : nextPlayerNumber(1) {}
};

TBSetupPlayersWidget::TBSetupPlayersWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("name", tr("Player Name"));
    pimpl->model->addHeader("added_at", tr("Member Since"));

    // Set up table view with sorting
    TBTableViewUtils::setupTableViewWithSorting(this, pimpl->ui.tableView, pimpl->model, 0, Qt::AscendingOrder);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);      // Player Name: stretch
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Member Since: fit content

    // Set date delegate for "Member Since" column (column 1)
    pimpl->ui.tableView->setItemDelegateForColumn(1, new TBDateEditDelegate(this));

    // Connect signals
    connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupPlayersWidget::on_addPlayerButtonClicked);
    connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupPlayersWidget::on_removePlayerButtonClicked);
    connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupPlayersWidget::on_modelDataChanged);

    // Connect selection model after setting the model
    connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                bool hasSelection = pimpl->ui.tableView->selectionModel()->hasSelection();
                pimpl->ui.removeButton->setEnabled(hasSelection);
            });
}

TBSetupPlayersWidget::~TBSetupPlayersWidget()
{
}

void TBSetupPlayersWidget::setConfiguration(const QVariantMap& configuration)
{
    QVariantList players = configuration.value("players").toList();
    pimpl->model->setListData(players);

    // Update next player number based on existing players
    pimpl->nextPlayerNumber = 1;
    for (const QVariant& playerVariant : players)
    {
        QVariantMap player = playerVariant.toMap();
        QString name = player.value("name").toString();

        // Check if name matches pattern "Player X"
        if (name.startsWith("Player "))
        {
            QString numberPart = name.mid(7); // Remove "Player " prefix
            bool ok;
            int number = numberPart.toInt(&ok);
            if (ok && number >= pimpl->nextPlayerNumber)
            {
                pimpl->nextPlayerNumber = number + 1;
            }
        }
    }
}

QVariantMap TBSetupPlayersWidget::configuration() const
{
    QVariantMap config;
    config["players"] = pimpl->model->listData();
    return config;
}

bool TBSetupPlayersWidget::validateConfiguration() const
{
    // At least one player is required
    return pimpl->model->rowCount() > 0;
}

void TBSetupPlayersWidget::on_addPlayerButtonClicked()
{
    QVariantMap newPlayer;
    newPlayer["player_id"] = QUuid::createUuid().toString();
    newPlayer["name"] = tr("Player %1").arg(pimpl->nextPlayerNumber);
    newPlayer["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    pimpl->nextPlayerNumber++;

    // Add to model
    QVariantList players = pimpl->model->listData();
    players.append(newPlayer);
    pimpl->model->setListData(players);

    Q_EMIT configurationChanged();
}

void TBSetupPlayersWidget::on_removePlayerButtonClicked()
{
    int row = TBTableViewUtils::getSelectedSourceRow(pimpl->ui.tableView);
    if (row < 0)
        return;

    // Remove from model
    QVariantList players = pimpl->model->listData();
    if (row >= 0 && row < players.size())
    {
        players.removeAt(row);
        pimpl->model->setListData(players);
        Q_EMIT configurationChanged();
    }
}

void TBSetupPlayersWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}
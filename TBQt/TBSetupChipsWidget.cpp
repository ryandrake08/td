#include "TBSetupChipsWidget.hpp"

#include "TBColorDisplayDelegate.hpp"
#include "TBTableViewUtils.hpp"
#include "TBVariantListTableModel.hpp"

#include "ui_TBSetupChipsWidget.h"

#include <QHeaderView>
#include <QRandomGenerator>
#include <QSortFilterProxyModel>

struct TBSetupChipsWidget::impl
{
    Ui::TBSetupChipsWidget ui;
    TBVariantListTableModel* model;

    QList<QString> usedColors;
};

TBSetupChipsWidget::TBSetupChipsWidget(QWidget* parent) : TBSetupTabWidget(parent), pimpl(new impl())
{
    // Setup UI from .ui file
    pimpl->ui.setupUi(this);

    // Create and configure model
    pimpl->model = new TBVariantListTableModel(this);
    pimpl->model->addHeader("color", tr("Color"));
    pimpl->model->addHeader("denomination", tr("Denomination"));
    pimpl->model->addHeader("count_available", tr("Count"));

    // Set up table view with sorting
    TBTableViewUtils::setupTableViewWithSorting(this, pimpl->ui.tableView, pimpl->model, 1, Qt::AscendingOrder);

    // Configure column behavior
    QHeaderView* header = pimpl->ui.tableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Color: fit content
    header->setSectionResizeMode(1, QHeaderView::Stretch);          // Denomination: stretch
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Count: fit content

    // Set color delegate for Color column (column 0)
    pimpl->ui.tableView->setItemDelegateForColumn(0, new TBColorDisplayDelegate(this));

    // Connect signals
    QObject::connect(pimpl->ui.addButton, &QPushButton::clicked, this, &TBSetupChipsWidget::on_addChipButtonClicked);
    QObject::connect(pimpl->ui.removeButton, &QPushButton::clicked, this, &TBSetupChipsWidget::on_removeChipButtonClicked);
    QObject::connect(pimpl->model, &QAbstractItemModel::dataChanged, this, &TBSetupChipsWidget::on_modelDataChanged);

    // Connect selection model after setting the model
    QObject::connect(pimpl->ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            [this]() {
                bool hasSelection = pimpl->ui.tableView->selectionModel()->hasSelection();
                pimpl->ui.removeButton->setEnabled(hasSelection);
            });
}

TBSetupChipsWidget::~TBSetupChipsWidget()
{
}

void TBSetupChipsWidget::setConfiguration(const QVariantMap& configuration)
{
    QVariantList chips = configuration.value("available_chips").toList();
    pimpl->model->setListData(chips);

    // Track used colors
    pimpl->usedColors.clear();
    for (const QVariant& chipVariant : chips)
    {
        QVariantMap chip = chipVariant.toMap();
        QString color = chip.value("color").toString();
        if (!color.isEmpty())
        {
            pimpl->usedColors.append(color);
        }
    }
}

QVariantMap TBSetupChipsWidget::configuration() const
{
    QVariantMap config;
    config["available_chips"] = pimpl->model->listData();
    return config;
}

bool TBSetupChipsWidget::validateConfiguration() const
{
    // At least one chip denomination is required
    return pimpl->model->rowCount() > 0;
}

void TBSetupChipsWidget::on_addChipButtonClicked()
{
    QList<int> defaultDenominations = getDefaultDenominations();
    QVariantList existingChips = pimpl->model->listData();

    // Find the next unused denomination
    int nextDenomination = 1;
    for (int denomination : defaultDenominations)
    {
        bool used = false;
        for (const QVariant& chipVariant : existingChips)
        {
            QVariantMap chip = chipVariant.toMap();
            if (chip.value("denomination").toInt() == denomination)
            {
                used = true;
                break;
            }
        }
        if (!used)
        {
            nextDenomination = denomination;
            break;
        }
    }

    // If all defaults are used, double the highest denomination
    if (nextDenomination == 1 && !existingChips.isEmpty())
    {
        int maxDenomination = 0;
        for (const QVariant& chipVariant : existingChips)
        {
            QVariantMap chip = chipVariant.toMap();
            int denomination = chip.value("denomination").toInt();
            if (denomination > maxDenomination)
            {
                maxDenomination = denomination;
            }
        }
        nextDenomination = maxDenomination * 2;
    }

    QVariantMap newChip;
    newChip["color"] = generateRandomColor();
    newChip["denomination"] = nextDenomination;
    newChip["count_available"] = 100; // Default count

    // Add to model
    QVariantList chips = pimpl->model->listData();
    chips.append(newChip);
    pimpl->model->setListData(chips);

    Q_EMIT configurationChanged();
}

void TBSetupChipsWidget::on_removeChipButtonClicked()
{
    int row = TBTableViewUtils::getSelectedSourceRow(pimpl->ui.tableView);
    if (row < 0)
        return;

    // Remove from model
    QVariantList chips = pimpl->model->listData();
    if (row >= 0 && row < chips.size())
    {
        chips.removeAt(row);
        pimpl->model->setListData(chips);
        Q_EMIT configurationChanged();
    }
}

void TBSetupChipsWidget::on_modelDataChanged()
{
    Q_EMIT configurationChanged();
}

QString TBSetupChipsWidget::generateRandomColor() const
{
    QString color;
    int attempts = 0;

    do {
        // Generate random RGB values avoiding very dark or very light colors
        int r = QRandomGenerator::global()->bounded(50, 206); // 50-205 range
        int g = QRandomGenerator::global()->bounded(50, 206);
        int b = QRandomGenerator::global()->bounded(50, 206);

        color = QColor(r, g, b).name();
        attempts++;
    } while (pimpl->usedColors.contains(color) && attempts < 20);

    pimpl->usedColors.append(color);
    return color;
}

QList<int> TBSetupChipsWidget::getDefaultDenominations() const
{
    // Standard poker chip denominations
    return {1, 5, 10, 25, 50, 100, 500, 1000, 5000, 10000};
}
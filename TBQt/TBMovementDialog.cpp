#include "TBMovementDialog.hpp"
#include "ui_TBMovementDialog.h"

#include <QStandardItemModel>
#include <QHeaderView>

TBMovementDialog::TBMovementDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::TBMovementDialog), model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Set up model headers
    model->setHorizontalHeaderLabels({
        tr("Player"),
        tr("From Table"),
        tr("From Seat"),
        tr("To Table"),
        tr("To Seat")
    });

    ui->tableView->setModel(model);

    // Configure column sizing
    QHeaderView* header = ui->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(0, QHeaderView::Stretch);          // Player Name: stretch to fill
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // From Table: fit content
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // From Seat: fit content
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // To Table: fit content
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // To Seat: fit content
}

TBMovementDialog::~TBMovementDialog() = default;

void TBMovementDialog::setMovements(const QVariantList& movements)
{
    // Clear existing data
    model->setRowCount(0);

    if (movements.isEmpty())
    {
        ui->titleLabel->setText(tr("No player movements are required."));
        return;
    }

    // Set title based on number of movements
    if (movements.size() == 1)
    {
        ui->titleLabel->setText(tr("The following player movement is required:"));
    }
    else
    {
        ui->titleLabel->setText(tr("The following %1 player movements are required:").arg(movements.size()));
    }

    // Populate model with movement data
    model->setRowCount(movements.size());

    for (int row = 0; row < movements.size(); ++row)
    {
        QVariantMap moveData = movements[row].toMap();

        QString playerName = moveData["name"].toString();
        QString fromTable = moveData["from_table_name"].toString();
        QString fromSeat = moveData["from_seat_name"].toString();
        QString toTable = moveData["to_table_name"].toString();
        QString toSeat = moveData["to_seat_name"].toString();

        model->setItem(row, 0, new QStandardItem(playerName));
        model->setItem(row, 1, new QStandardItem(fromTable));
        model->setItem(row, 2, new QStandardItem(fromSeat));
        model->setItem(row, 3, new QStandardItem(toTable));
        model->setItem(row, 4, new QStandardItem(toSeat));

        // Make all items non-editable
        for (int col = 0; col < 5; ++col)
        {
            QStandardItem* item = model->item(row, col);
            if (item)
            {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
        }
    }

    // Auto-resize columns to content after populating data
    ui->tableView->resizeColumnsToContents();
}
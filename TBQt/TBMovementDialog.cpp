#include "TBMovementDialog.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

struct TBMovementDialog::impl
{
    QTableView* tableView;
    QStandardItemModel* model;
    QLabel* titleLabel;
    QDialogButtonBox* buttonBox;
};

TBMovementDialog::TBMovementDialog(QWidget* parent) : QDialog(parent), pimpl(new impl)
{
    setWindowTitle(tr("Player Movements"));
    setModal(true);
    resize(600, 400);

    // Create main layout
    auto mainLayout = new QVBoxLayout(this);

    // Title label
    pimpl->titleLabel = new QLabel(tr("The following player movements are required:"), this);
    pimpl->titleLabel->setWordWrap(true);
    mainLayout->addWidget(pimpl->titleLabel);

    // Create table view and model
    pimpl->tableView = new QTableView(this);
    pimpl->model = new QStandardItemModel(this);

    // Set up model headers
    pimpl->model->setHorizontalHeaderLabels({
        tr("Player"),
        tr("From Table"),
        tr("From Seat"),
        tr("To Table"),
        tr("To Seat")
    });

    pimpl->tableView->setModel(pimpl->model);
    pimpl->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pimpl->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    pimpl->tableView->setAlternatingRowColors(true);
    pimpl->tableView->setSortingEnabled(false);

    // Configure column sizing
    QHeaderView* header = pimpl->tableView->horizontalHeader();
    header->setStretchLastSection(true);
    header->setSectionResizeMode(0, QHeaderView::Stretch);          // Player Name: stretch to fill
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // From Table: fit content
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // From Seat: fit content
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents); // To Table: fit content
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents); // To Seat: fit content

    mainLayout->addWidget(pimpl->tableView);

    // Create button box with OK button
    pimpl->buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(pimpl->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    mainLayout->addWidget(pimpl->buttonBox);
}

TBMovementDialog::~TBMovementDialog() = default;

void TBMovementDialog::setMovements(const QVariantList& movements)
{
    // Clear existing data
    pimpl->model->setRowCount(0);

    if (movements.isEmpty())
    {
        pimpl->titleLabel->setText(tr("No player movements are required."));
        return;
    }

    // Set title based on number of movements
    if (movements.size() == 1)
    {
        pimpl->titleLabel->setText(tr("The following player movement is required:"));
    }
    else
    {
        pimpl->titleLabel->setText(tr("The following %1 player movements are required:").arg(movements.size()));
    }

    // Populate model with movement data
    pimpl->model->setRowCount(movements.size());

    for (int row = 0; row < movements.size(); ++row)
    {
        QVariantMap moveData = movements[row].toMap();

        QString playerName = moveData["name"].toString();
        QString fromTable = moveData["from_table_name"].toString();
        QString fromSeat = moveData["from_seat_name"].toString();
        QString toTable = moveData["to_table_name"].toString();
        QString toSeat = moveData["to_seat_name"].toString();

        pimpl->model->setItem(row, 0, new QStandardItem(playerName));
        pimpl->model->setItem(row, 1, new QStandardItem(fromTable));
        pimpl->model->setItem(row, 2, new QStandardItem(fromSeat));
        pimpl->model->setItem(row, 3, new QStandardItem(toTable));
        pimpl->model->setItem(row, 4, new QStandardItem(toSeat));

        // Make all items non-editable
        for (int col = 0; col < 5; ++col)
        {
            QStandardItem* item = pimpl->model->item(row, col);
            if (item)
            {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
        }
    }

    // Auto-resize columns to content after populating data
    pimpl->tableView->resizeColumnsToContents();
}
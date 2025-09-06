#include "TBTableWidget.hpp"
#include "ui_TBTableWidget.h"

#include <QHeaderView>
#include <QStandardItemModel>

struct TBTableWidget::impl
{
    // UI
    Ui::TBTableWidget ui {};

    // Model
    QStandardItemModel* seatsModel { nullptr };

    // Data
    QString tableName;
    QVariantList seats;

    impl() = default;
};

TBTableWidget::TBTableWidget(QWidget* parent) : QWidget(parent), pimpl(new impl)
{
    pimpl->ui.setupUi(this);

    // Set QGroupBox title font to match TBTournamentDisplayWindow (21px, bold)
    QFont titleFont = pimpl->ui.tableGroupBox->font();
    titleFont.setPointSize(21);
    titleFont.setBold(true);
    pimpl->ui.tableGroupBox->setFont(titleFont);

    // Set up model
    pimpl->seatsModel = new QStandardItemModel(this);
    pimpl->seatsModel->setHorizontalHeaderLabels({ "Seat", "Player" });
    pimpl->ui.seatsTableView->setModel(pimpl->seatsModel);

    // Configure columns - seat number narrow, player name wide
    QHeaderView* header = pimpl->ui.seatsTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    pimpl->ui.seatsTableView->setColumnWidth(0, 60); // Fixed width for seat numbers

    // Configure vertical header (row numbers)
    pimpl->ui.seatsTableView->verticalHeader()->setVisible(false);
}

TBTableWidget::~TBTableWidget() = default;

void TBTableWidget::setTableName(const QString& name)
{
    pimpl->tableName = name;
    pimpl->ui.tableGroupBox->setTitle(name);
    updateSeatsTable();
}

void TBTableWidget::setSeats(const QVariantList& seatList)
{
    pimpl->seats = seatList;
    updateSeatsTable();
}

void TBTableWidget::updateSeatsTable()
{
    if(!pimpl->seatsModel)
    {
        return;
    }

    // Clear existing data
    pimpl->seatsModel->setRowCount(0);

    if(pimpl->seats.isEmpty())
    {
        // Add a single row showing "No players seated"
        pimpl->seatsModel->setRowCount(1);
        pimpl->seatsModel->setItem(0, 0, new QStandardItem("—"));
        pimpl->seatsModel->setItem(0, 1, new QStandardItem("(no players seated)"));

        // Make items non-editable and style them
        for(int col = 0; col < 2; ++col)
        {
            QStandardItem* item = pimpl->seatsModel->item(0, col);
            if(item)
            {
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                if(col == 1)
                {
                    QFont font = item->font();
                    font.setItalic(true);
                    item->setFont(font);
                    item->setForeground(QColor(120, 120, 120));
                }
            }
        }
    }
    else
    {
        // Sort seats by seat_name (numerical sorting)
        QVariantList sortedSeats = pimpl->seats;
        std::sort(sortedSeats.begin(), sortedSeats.end(), [](const QVariant& a, const QVariant& b)
        {
            QString seatA = a.toMap().value("seat_name").toString();
            QString seatB = b.toMap().value("seat_name").toString();
            return seatA.toInt() < seatB.toInt(); // Numerical comparison
        });

        // Populate with seat data
        pimpl->seatsModel->setRowCount(sortedSeats.size());
        for(int row = 0; row < sortedSeats.size(); ++row)
        {
            QVariantMap seat = sortedSeats[row].toMap();
            QString playerName = seat.value("player_name").toString();
            QString seatName = seat.value("seat_name").toString();

            // Seat number in first column
            auto* seatItem = new QStandardItem(seatName);
            seatItem->setFlags(seatItem->flags() & ~Qt::ItemIsEditable);
            seatItem->setTextAlignment(Qt::AlignCenter);
            pimpl->seatsModel->setItem(row, 0, seatItem);

            // Player name in second column
            QString displayName = playerName.isEmpty() ? "(empty)" : playerName;
            auto* playerItem = new QStandardItem(displayName);
            playerItem->setFlags(playerItem->flags() & ~Qt::ItemIsEditable);

            if(playerName.isEmpty())
            {
                // Style empty seats
                QFont font = playerItem->font();
                font.setItalic(true);
                playerItem->setFont(font);
                playerItem->setForeground(QColor(120, 120, 120));
            }

            pimpl->seatsModel->setItem(row, 1, playerItem);
        }
    }

    // Update widget height based on content
    int numRows = pimpl->seatsModel->rowCount();
    int tableHeaderHeight = pimpl->ui.seatsTableView->horizontalHeader()->height();
    int rowHeight = pimpl->ui.seatsTableView->rowHeight(0);
    int mainLayoutMargins = pimpl->ui.mainLayout->contentsMargins().top() + pimpl->ui.mainLayout->contentsMargins().bottom();
    int groupBoxMargins = pimpl->ui.groupBoxLayout->contentsMargins().top() + pimpl->ui.groupBoxLayout->contentsMargins().bottom();
    int groupBoxTitleHeight = 30; // Estimated height for QGroupBox title with 21px font

    int totalHeight = mainLayoutMargins + groupBoxTitleHeight + groupBoxMargins + tableHeaderHeight + (numRows * rowHeight) + 10; // +10 for padding

    setFixedHeight(totalHeight);
    pimpl->ui.seatsTableView->setFixedHeight(tableHeaderHeight + (numRows * rowHeight) + 5);
}

QSize TBTableWidget::sizeHint() const
{
    // Fixed width, height depends on number of seats
    int width = 320;
    int numRows = pimpl->seatsModel ? pimpl->seatsModel->rowCount() : 0;

    // Calculate height with QGroupBox: main margins + group box title + group margins + table header + rows + padding
    int height = 10 + 30 + 16 + 25 + (35 * qMax(numRows, 1)) + 20;

    return { width, height };
}
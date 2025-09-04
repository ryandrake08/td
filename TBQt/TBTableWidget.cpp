#include "TBTableWidget.hpp"

#include <QFont>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

struct TBTableWidget::impl
{
    // UI components
    QLabel* tableNameLabel;
    QTableView* seatsTableView;
    QStandardItemModel* seatsModel;
    QVBoxLayout* mainLayout;

    // Data
    QString tableName;
    QVariantList seats;

    impl() : tableNameLabel(nullptr), seatsTableView(nullptr), seatsModel(nullptr), mainLayout(nullptr) {}
};

TBTableWidget::TBTableWidget(QWidget* parent) : QWidget(parent), pimpl(new impl)
{
    // Set fixed width matching macOS (320px)
    setFixedWidth(320);

    // Main vertical layout
    pimpl->mainLayout = new QVBoxLayout(this);
    pimpl->mainLayout->setContentsMargins(5, 5, 5, 5);
    pimpl->mainLayout->setSpacing(5);

    // Table name header with frame
    QFrame* headerFrame = new QFrame();
    headerFrame->setFrameShape(QFrame::StyledPanel);
    headerFrame->setFixedHeight(35);
    QVBoxLayout* headerLayout = new QVBoxLayout(headerFrame);
    headerLayout->setContentsMargins(8, 8, 8, 8);

    pimpl->tableNameLabel = new QLabel("Table Name");
    QFont headerFont = pimpl->tableNameLabel->font();
    headerFont.setPointSize(14);
    headerFont.setBold(true);
    pimpl->tableNameLabel->setFont(headerFont);
    pimpl->tableNameLabel->setAlignment(Qt::AlignCenter);
    headerLayout->addWidget(pimpl->tableNameLabel);

    pimpl->mainLayout->addWidget(headerFrame);

    // Seats table view
    pimpl->seatsTableView = new QTableView();
    pimpl->seatsTableView->setAlternatingRowColors(true);
    pimpl->seatsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    pimpl->seatsTableView->setSelectionMode(QAbstractItemView::NoSelection);
    pimpl->seatsTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pimpl->seatsTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pimpl->seatsTableView->setShowGrid(false);

    // Set up model
    pimpl->seatsModel = new QStandardItemModel(this);
    pimpl->seatsModel->setHorizontalHeaderLabels({ "Seat", "Player" });
    pimpl->seatsTableView->setModel(pimpl->seatsModel);

    // Configure columns - seat number narrow, player name wide
    QHeaderView* header = pimpl->seatsTableView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    pimpl->seatsTableView->setColumnWidth(0, 60); // Fixed width for seat numbers

    // Configure vertical header (row numbers)
    pimpl->seatsTableView->verticalHeader()->setVisible(false);

    pimpl->mainLayout->addWidget(pimpl->seatsTableView);

    // Set background color
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(250, 250, 250));
    setPalette(palette);
}

TBTableWidget::~TBTableWidget() = default;

void TBTableWidget::setTableName(const QString& name)
{
    pimpl->tableName = name;
    if(pimpl->tableNameLabel)
    {
        pimpl->tableNameLabel->setText(name);
    }
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
            QStandardItem* seatItem = new QStandardItem(seatName);
            seatItem->setFlags(seatItem->flags() & ~Qt::ItemIsEditable);
            seatItem->setTextAlignment(Qt::AlignCenter);
            pimpl->seatsModel->setItem(row, 0, seatItem);

            // Player name in second column
            QString displayName = playerName.isEmpty() ? "(empty)" : playerName;
            QStandardItem* playerItem = new QStandardItem(displayName);
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
    int headerHeight = 35; // Header frame height
    int tableHeaderHeight = pimpl->seatsTableView->horizontalHeader()->height();
    int rowHeight = pimpl->seatsTableView->rowHeight(0);
    int margins = pimpl->mainLayout->contentsMargins().top() + pimpl->mainLayout->contentsMargins().bottom();
    int spacing = pimpl->mainLayout->spacing();

    int totalHeight = margins + headerHeight + spacing + tableHeaderHeight + (numRows * rowHeight) + 10; // +10 for padding

    setFixedHeight(totalHeight);
    pimpl->seatsTableView->setFixedHeight(tableHeaderHeight + (numRows * rowHeight) + 5);
}

QSize TBTableWidget::sizeHint() const
{
    // Fixed width, height depends on number of seats
    int width = 320;
    int numRows = pimpl->seatsModel ? pimpl->seatsModel->rowCount() : 0;

    // Calculate height matching macOS formula: 20 + 35 + 8 + 25 + (35 * numRows) + 20
    int height = 20 + 35 + 8 + 25 + (35 * qMax(numRows, 1)) + 20;

    return QSize(width, height);
}
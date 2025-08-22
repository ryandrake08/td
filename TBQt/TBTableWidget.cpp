#include "TBTableWidget.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QFontMetrics>
#include <QDebug>

struct TBTableWidget::impl
{
    QString tableName;
    QVariantList seats;
    QFont titleFont;
    QFont playerFont;

    impl() {
        titleFont.setPointSize(14);
        titleFont.setBold(true);
        playerFont.setPointSize(10);
    }
};

TBTableWidget::TBTableWidget(QWidget* parent)
    : QWidget(parent), pimpl(new impl)
{
    // Set minimum size
    setMinimumSize(320, 108);

    // Set background color
    setAutoFillBackground(true);

    // Set palette for background
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(250, 250, 250));
    setPalette(palette);
}

TBTableWidget::~TBTableWidget() = default;

void TBTableWidget::setTableName(const QString& tableName)
{
    pimpl->tableName = tableName;
    update();
}

void TBTableWidget::setSeats(const QVariantList& seats)
{
    pimpl->seats = seats;
    updateLayout();
    update();
}

QSize TBTableWidget::sizeHint() const
{
    // Base size similar to macOS version: 320x108
    int width = 320;
    int height = 20 + 35 + 8 + 25 + (35 * pimpl->seats.size()) + 20; // Matching macOS calculation

    return QSize(width, qMax(height, 108)); // Minimum 108 height
}

void TBTableWidget::updateLayout()
{
    // Recalculate size based on number of seats
    QSize newSize = sizeHint();
    setMinimumSize(newSize);
    resize(newSize);
}

void TBTableWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect();

    // Draw border
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.setBrush(QBrush(QColor(255, 255, 255)));
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), 8, 8);

    // Draw table name
    painter.setFont(pimpl->titleFont);
    painter.setPen(QColor(50, 50, 50));

    QRect titleRect = rect.adjusted(10, 10, -10, -rect.height() + 45);
    painter.drawText(titleRect, Qt::AlignCenter | Qt::AlignTop, pimpl->tableName);

    // Draw table illustration (simple oval)
    QRect tableRect = rect.adjusted(50, 45, -50, -rect.height() + 80);
    painter.setPen(QPen(QColor(100, 100, 100), 2));
    painter.setBrush(QBrush(QColor(240, 240, 240)));
    painter.drawEllipse(tableRect);

    // Draw seats and players
    painter.setFont(pimpl->playerFont);

    int yOffset = 90;
    for (const QVariant& seatVariant : pimpl->seats)
    {
        QVariantMap seat = seatVariant.toMap();
        QString playerName = seat.value("player_name").toString();
        QString seatName = seat.value("seat_name").toString();

        QRect seatRect = rect.adjusted(10, yOffset, -10, yOffset + 30 - rect.height());

        // Draw seat background
        if (!playerName.isEmpty()) {
            painter.setPen(QPen(QColor(150, 150, 150), 1));
            painter.setBrush(QBrush(QColor(220, 255, 220))); // Light green for occupied
        } else {
            painter.setPen(QPen(QColor(180, 180, 180), 1));
            painter.setBrush(QBrush(QColor(245, 245, 245))); // Light gray for empty
        }
        painter.drawRoundedRect(seatRect, 4, 4);

        // Draw seat text
        painter.setPen(QColor(50, 50, 50));
        QString seatText = seatName;
        if (!playerName.isEmpty()) {
            seatText += ": " + playerName;
        } else {
            seatText += ": (empty)";
        }

        painter.drawText(seatRect.adjusted(8, 0, -8, 0), Qt::AlignLeft | Qt::AlignVCenter, seatText);

        yOffset += 35;
    }

    // If no seats, show message
    if (pimpl->seats.isEmpty()) {
        painter.setFont(pimpl->playerFont);
        painter.setPen(QColor(120, 120, 120));
        QRect messageRect = rect.adjusted(10, 90, -10, -10);
        painter.drawText(messageRect, Qt::AlignCenter, "No players seated at this table");
    }
}
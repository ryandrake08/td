#include "TBFlowLayout.hpp"

#include <QRect>
#include <QStyle>
#include <QWidget>

struct TBFlowLayout::impl
{
    QList<QLayoutItem *> itemList;
    int hSpace;
    int vSpace;

    impl(int hSpacing, int vSpacing) : hSpace(hSpacing), vSpace(vSpacing) {}
};

TBFlowLayout::TBFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), pimpl(new impl(hSpacing, vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

TBFlowLayout::TBFlowLayout(int margin, int hSpacing, int vSpacing)
    : pimpl(new impl(hSpacing, vSpacing))
{
    setContentsMargins(margin, margin, margin, margin);
}

TBFlowLayout::~TBFlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void TBFlowLayout::addItem(QLayoutItem *item)
{
    pimpl->itemList.append(item);
}

int TBFlowLayout::horizontalSpacing() const
{
    if (pimpl->hSpace >= 0) {
        return pimpl->hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int TBFlowLayout::verticalSpacing() const
{
    if (pimpl->vSpace >= 0) {
        return pimpl->vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int TBFlowLayout::count() const
{
    return pimpl->itemList.size();
}

QLayoutItem *TBFlowLayout::itemAt(int index) const
{
    return pimpl->itemList.value(index);
}

QLayoutItem *TBFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < pimpl->itemList.size())
        return pimpl->itemList.takeAt(index);
    return nullptr;
}

Qt::Orientations TBFlowLayout::expandingDirections() const
{
    return {};
}

bool TBFlowLayout::hasHeightForWidth() const
{
    return true;
}

int TBFlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void TBFlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize TBFlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize TBFlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem *item : pimpl->itemList)
        size = size.expandedTo(item->minimumSize());

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

int TBFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);

    if (pimpl->itemList.isEmpty()) {
        return rect.y() + bottom;
    }

    // First pass: calculate how many items fit per row and total content width
    QList<QList<QLayoutItem*>> rows;
    QList<int> rowWidths;
    QList<QLayoutItem*> currentRow;
    int currentRowWidth = 0;
    int maxRowWidth = 0;

    for (QLayoutItem *item : pimpl->itemList) {
        const QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);

        int itemWidth = item->sizeHint().width();
        int nextRowWidth = currentRowWidth + itemWidth + (currentRow.isEmpty() ? 0 : spaceX);

        if (nextRowWidth > effectiveRect.width() && !currentRow.isEmpty()) {
            // Finish current row and start new one
            rows.append(currentRow);
            rowWidths.append(currentRowWidth);
            maxRowWidth = qMax(maxRowWidth, currentRowWidth);

            currentRow.clear();
            currentRow.append(item);
            currentRowWidth = itemWidth;
        } else {
            // Add to current row
            currentRow.append(item);
            currentRowWidth = nextRowWidth;
        }
    }

    // Add the last row
    if (!currentRow.isEmpty()) {
        rows.append(currentRow);
        rowWidths.append(currentRowWidth);
        maxRowWidth = qMax(maxRowWidth, currentRowWidth);
    }

    // Second pass: position items with centered rows
    int y = effectiveRect.y();
    int totalHeight = 0;

    for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        const QList<QLayoutItem*>& row = rows[rowIndex];
        int rowWidth = rowWidths[rowIndex];

        // Center the row horizontally
        int startX = effectiveRect.x() + (effectiveRect.width() - rowWidth) / 2;
        int x = startX;
        int lineHeight = 0;

        for (QLayoutItem *item : row) {
            const QWidget *wid = item->widget();
            int spaceX = horizontalSpacing();
            if (spaceX == -1)
                spaceX = wid->style()->layoutSpacing(
                    QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);

            if (!testOnly)
                item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

            x += item->sizeHint().width();
            if (item != row.last()) // Don't add spacing after last item in row
                x += spaceX;

            lineHeight = qMax(lineHeight, item->sizeHint().height());
        }

        // Move to next row
        if (rowIndex < rows.size() - 1) {
            int spaceY = verticalSpacing();
            if (spaceY == -1 && !row.isEmpty()) {
                const QWidget *wid = row.first()->widget();
                spaceY = wid->style()->layoutSpacing(
                    QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
            }
            y += lineHeight + spaceY;
        }

        totalHeight = y + lineHeight - effectiveRect.y();
    }

    return totalHeight + bottom;
}

int TBFlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, nullptr, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}
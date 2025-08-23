#include "TBManageButtonDelegate.hpp"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>

TBManageButtonDelegate::TBManageButtonDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void TBManageButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Create button style option
    QStyleOptionButton buttonOption;
    buttonOption.rect = option.rect.adjusted(4, 4, -4, -4); // Add some padding
    buttonOption.text = index.data(Qt::DisplayRole).toString();
    buttonOption.state = QStyle::State_Enabled;

    // Try to add a standard list icon (simple approach)
    buttonOption.icon = QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView);
    if (!buttonOption.icon.isNull())
    {
        buttonOption.iconSize = QSize(16, 16);
    }

    // Check if mouse is over this button
    if (option.state & QStyle::State_MouseOver)
        buttonOption.state |= QStyle::State_MouseOver;

    // Draw the button
    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);
}

bool TBManageButtonDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    Q_UNUSED(model)

    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            // Check if click is within the button area (with padding)
            QRect buttonRect = option.rect.adjusted(4, 4, -4, -4);
            if (buttonRect.contains(mouseEvent->pos()))
            {
                Q_EMIT buttonClicked(index);
                return true;
            }
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QSize TBManageButtonDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)

    // Return a reasonable size for the button
    QFontMetrics fm(option.font);
    return QSize(fm.horizontalAdvance("Manage") + 40, fm.height() + 8); // Extra width for icon
}
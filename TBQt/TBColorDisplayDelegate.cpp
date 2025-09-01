#include "TBColorDisplayDelegate.hpp"

#include <QColorDialog>
#include <QColor>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>

TBColorDisplayDelegate::TBColorDisplayDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget* TBColorDisplayDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QPushButton* button = new QPushButton(parent);
    button->setText(tr("Choose Color..."));

    // Install event filter to prevent premature editor destruction
    button->installEventFilter(const_cast<TBColorDisplayDelegate*>(this));

    QObject::connect(button, &QPushButton::clicked, this, &TBColorDisplayDelegate::onColorButtonClicked);

    return button;
}

void TBColorDisplayDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QPushButton* button = qobject_cast<QPushButton*>(editor);
    if (!button)
        return;

    QString colorString = index.model()->data(index, Qt::EditRole).toString();
    QColor color = parseColor(colorString);

    button->setProperty("currentColorString", colorString);

    if (color.isValid())
    {
        button->setText(color.name());
        QString style = QString("background-color: %1; color: %2;")
            .arg(color.name())
            .arg(color.lightness() > 128 ? "black" : "white");
        button->setStyleSheet(style);
    }
    else
    {
        button->setText(tr("Choose Color..."));
        button->setStyleSheet("");
    }
}

void TBColorDisplayDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QPushButton* button = qobject_cast<QPushButton*>(editor);
    if (!button)
        return;

    QColor selectedColor = button->property("selectedColor").value<QColor>();
    if (selectedColor.isValid())
    {
        model->setData(index, selectedColor.name(), Qt::EditRole);
    }
}

void TBColorDisplayDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void TBColorDisplayDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString colorString = index.data().toString();
    QColor color = parseColor(colorString);

    if (color.isValid())
    {
        // Draw color swatch only (no text)
        QRect colorRect = option.rect.adjusted(4, 4, -4, -4);

        painter->fillRect(colorRect, color);
        painter->setPen(Qt::black);
        painter->drawRect(colorRect);
    }
    else
    {
        // Fallback to default painting
        QStyledItemDelegate::paint(painter, option, index);
    }
}


QColor TBColorDisplayDelegate::parseColor(const QString& colorString) const
{
    if (colorString.isEmpty())
        return QColor();

    // Handle hex colors
    if (colorString.startsWith("#"))
        return QColor(colorString);

    // Handle named colors
    QColor color;
    color.setNamedColor(colorString);
    return color;
}

void TBColorDisplayDelegate::onColorButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button)
        return;

    QColor currentColor = parseColor(button->property("currentColorString").toString());
    QColor color = QColorDialog::getColor(currentColor, button->window(), tr("Select Chip Color"));

    // Check if button still exists after modal dialog
    QPushButton* stillValidButton = qobject_cast<QPushButton*>(sender());
    if (!stillValidButton || !color.isValid())
        return;

    stillValidButton->setProperty("selectedColor", color);
    stillValidButton->setText(color.name());

    // Update button background to show the color
    QString style = QString("background-color: %1; color: %2;")
        .arg(color.name())
        .arg(color.lightness() > 128 ? "black" : "white");
    stillValidButton->setStyleSheet(style);
}

bool TBColorDisplayDelegate::eventFilter(QObject* watched, QEvent* event)
{
    // Check if this is a focus out event on our button editor
    if (event->type() == QEvent::FocusOut)
    {
        QPushButton* button = qobject_cast<QPushButton*>(watched);
        if (button)
        {
            // Block focus out events to prevent Qt from destroying the editor
            // while the color dialog is open
            return true;
        }
    }

    return QStyledItemDelegate::eventFilter(watched, event);
}

QString TBColorDisplayDelegate::generateRandomColor() const
{
    // Generate random RGB values avoiding very dark or very light colors
    int r = QRandomGenerator::global()->bounded(50, 206); // 50-205 range
    int g = QRandomGenerator::global()->bounded(50, 206);
    int b = QRandomGenerator::global()->bounded(50, 206);

    return QColor(r, g, b).name();
}
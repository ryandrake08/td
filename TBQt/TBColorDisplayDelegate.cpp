#include "TBColorDisplayDelegate.hpp"

#include <QColorDialog>
#include <QColor>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>

TBColorDisplayDelegate::TBColorDisplayDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* TBColorDisplayDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QPushButton* button = new QPushButton(parent);
    button->setText(tr("Choose Color..."));
    
    connect(button, &QPushButton::clicked, [button, this]() {
        QColor currentColor = parseColor(button->property("currentColorString").toString());
        QColor color = QColorDialog::getColor(currentColor, button, tr("Select Chip Color"));
        
        if (color.isValid())
        {
            button->setProperty("selectedColor", color);
            button->setText(color.name());
            
            // Update button background to show the color
            QString style = QString("background-color: %1; color: %2;")
                .arg(color.name())
                .arg(color.lightness() > 128 ? "black" : "white");
            button->setStyleSheet(style);
        }
    });
    
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
        // Draw color swatch
        QRect colorRect = option.rect.adjusted(4, 4, -4, -4);
        QRect swatchRect = QRect(colorRect.left(), colorRect.top(), 20, colorRect.height());
        
        painter->fillRect(swatchRect, color);
        painter->setPen(Qt::black);
        painter->drawRect(swatchRect);
        
        // Draw color name
        QRect textRect = colorRect.adjusted(25, 0, 0, 0);
        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, color.name().toUpper());
    }
    else
    {
        // Fallback to default painting
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QString TBColorDisplayDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    Q_UNUSED(locale)
    
    QString colorString = value.toString();
    QColor color = parseColor(colorString);
    
    return color.isValid() ? color.name().toUpper() : colorString;
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

QString TBColorDisplayDelegate::generateRandomColor() const
{
    // Generate random RGB values avoiding very dark or very light colors
    int r = QRandomGenerator::global()->bounded(50, 206); // 50-205 range
    int g = QRandomGenerator::global()->bounded(50, 206);
    int b = QRandomGenerator::global()->bounded(50, 206);
    
    return QColor(r, g, b).name();
}
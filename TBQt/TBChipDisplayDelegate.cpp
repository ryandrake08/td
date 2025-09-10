#include "TBChipDisplayDelegate.hpp"

#include <QApplication>
#include <QDir>
#include <QPainter>
#include <QPixmap>
#include <QStyleOptionViewItem>

// Initialize CSS color lookup table with common colors from TBColor+CSS.m
const QMap<QString, QString> TBChipDisplayDelegate::cssColors = TBChipDisplayDelegate::createCssColorMap();

TBChipDisplayDelegate::TBChipDisplayDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

TBChipDisplayDelegate::~TBChipDisplayDelegate() = default;

void TBChipDisplayDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Only handle color column (column 0) specially
    if(index.column() == 0)
    {
        // Get the color string from the model
        QString colorString = index.data(Qt::DisplayRole).toString();
        QColor chipColor = parseColor(colorString);

        // Draw selection background if selected
        if(option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        else
        {
            painter->fillRect(option.rect, option.palette.base());
        }

        // Calculate chip display size and position - make it larger to see the image better
        int chipSize = qMin(option.rect.height() - 2, 24);                   // Larger size, less margin
        int chipX = option.rect.x() + 4;                                     // Smaller left margin
        int chipY = option.rect.y() + (option.rect.height() - chipSize) / 2; // Center vertically

        QRect chipRect(chipX, chipY, chipSize, chipSize);

        painter->setRenderHint(QPainter::Antialiasing);

        // Draw colored ellipse first as background
        painter->setBrush(QBrush(chipColor));
        painter->setPen(QPen(QColor(0, 0, 0, 60), 1)); // Subtle border
        painter->drawEllipse(chipRect);

        // Draw chip image on top of the colored ellipse
        static QPixmap chipImage(":/icons/i_chip.svg");

        if(!chipImage.isNull())
        {
            // Try drawing with composition mode to ensure visibility
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            QPixmap scaledChip = chipImage.scaled(chipSize, chipSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            painter->drawPixmap(chipRect, scaledChip);
        }
        else
        {
            // Fallback: draw a white border if image fails to load
            painter->setPen(QPen(QColor(255, 255, 255, 200), 2)); // White border
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(chipRect);

            // Also draw an 'X' to indicate missing image
            painter->setPen(QPen(QColor(255, 255, 255), 1));
            painter->drawLine(chipRect.topLeft(), chipRect.bottomRight());
            painter->drawLine(chipRect.topRight(), chipRect.bottomLeft());
        }
    }
    else
    {
        // Use default rendering for other columns
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize TBChipDisplayDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    // For color column, ensure minimum height for ellipse
    if(index.column() == 0)
    {
        size.setHeight(qMax(size.height(), 28)); // Minimum 28px height for 20px ellipse + margins
    }

    return size;
}

QColor TBChipDisplayDelegate::parseColor(const QString& colorString) const
{
    if(colorString.isEmpty())
    {
        return { 128, 128, 128 }; // Default gray
    }

    // Try hex color first (starts with #)
    if(colorString.startsWith("#"))
    {
        return { colorString };
    }

    // Try CSS color name lookup
    QString lowerColorName = colorString.toLower();
    if(cssColors.contains(lowerColorName))
    {
        return { cssColors[lowerColorName] };
    }

    // Try as direct hex without #
    QColor directHex(QString("#") + colorString);
    if(directHex.isValid())
    {
        return directHex;
    }

    // Fall back to Qt color parsing
    QColor qtColor(colorString);
    if(qtColor.isValid())
    {
        return qtColor;
    }

    // Final fallback to gray
    return { 128, 128, 128 };
}

QMap<QString, QString> TBChipDisplayDelegate::createCssColorMap()
{
    // Subset of most commonly used colors from TBColor+CSS.m
    // Full list would be too large, this covers typical chip colors
    QMap<QString, QString> colors;

    colors["black"] = "#000000";
    colors["white"] = "#ffffff";
    colors["red"] = "#ff0000";
    colors["green"] = "#008000";
    colors["blue"] = "#0000ff";
    colors["yellow"] = "#ffff00";
    colors["orange"] = "#ffa500";
    colors["purple"] = "#800080";
    colors["pink"] = "#ffc0cb";
    colors["brown"] = "#a52a2a";
    colors["gray"] = "#808080";
    colors["grey"] = "#808080";
    colors["darkblue"] = "#00008b";
    colors["darkgreen"] = "#006400";
    colors["darkred"] = "#8b0000";
    colors["lightblue"] = "#add8e6";
    colors["lightgreen"] = "#90ee90";
    colors["lightyellow"] = "#ffffe0";
    colors["darkgray"] = "#a9a9a9";
    colors["darkgrey"] = "#a9a9a9";
    colors["lightgray"] = "#d3d3d3";
    colors["lightgrey"] = "#d3d3d3";
    colors["navy"] = "#000080";
    colors["maroon"] = "#800000";
    colors["olive"] = "#808000";
    colors["lime"] = "#00ff00";
    colors["aqua"] = "#00ffff";
    colors["teal"] = "#008080";
    colors["silver"] = "#c0c0c0";
    colors["fuchsia"] = "#ff00ff";
    colors["gold"] = "#ffd700";
    colors["crimson"] = "#dc143c";
    colors["coral"] = "#ff7f50";
    colors["salmon"] = "#fa8072";
    colors["chocolate"] = "#d2691e";
    colors["tan"] = "#d2b48c";
    colors["beige"] = "#f5f5dc";
    colors["ivory"] = "#fffff0";
    colors["khaki"] = "#f0e68c";
    colors["violet"] = "#ee82ee";
    colors["indigo"] = "#4b0082";
    colors["turquoise"] = "#40e0d0";

    return colors;
}
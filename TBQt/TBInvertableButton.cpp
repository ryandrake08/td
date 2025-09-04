#include "TBInvertableButton.hpp"

#include "TBImageInversionHelper.hpp"

#include <QIcon>
#include <QPainter>
#include <QStyleOption>
#include <QStyleOptionButton>

TBInvertableButton::TBInvertableButton(QWidget* parent) : QPushButton(parent), m_imageInverted(false), m_textBelowIcon(false)
{
}

TBInvertableButton::TBInvertableButton(const QString& text, QWidget* parent) : QPushButton(text, parent), m_imageInverted(false), m_textBelowIcon(false)
{
}

void TBInvertableButton::setOriginalIcon(const QIcon& icon)
{
    m_originalIcon = icon;
    m_invertedIcon = TBImageInversionHelper::createInvertedIcon(icon);
    updateButtonIcon();
}

bool TBInvertableButton::imageInverted() const
{
    return m_imageInverted;
}

bool TBInvertableButton::textBelowIcon() const
{
    return m_textBelowIcon;
}

void TBInvertableButton::setImageInverted(bool inverted)
{
    if(m_imageInverted == inverted)
    {
        return;
    }

    m_imageInverted = inverted;
    updateButtonIcon();
}

void TBInvertableButton::setTextBelowIcon(bool below)
{
    if(m_textBelowIcon == below)
    {
        return;
    }

    m_textBelowIcon = below;
    update(); // Trigger repaint
}

void TBInvertableButton::updateButtonIcon()
{
    if(m_imageInverted && !m_invertedIcon.isNull())
    {
        setIcon(m_invertedIcon);
    }
    else if(!m_originalIcon.isNull())
    {
        setIcon(m_originalIcon);
    }
}

void TBInvertableButton::paintEvent(QPaintEvent* event)
{
    if(!m_textBelowIcon)
    {
        // Use default painting when not using text below icon layout
        QPushButton::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw the button background using the style
    QStyleOptionButton option;
    initStyleOption(&option);
    option.text = QString(); // Don't draw text through style
    option.icon = QIcon();   // Don't draw icon through style
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);

    // Get the current icon
    QIcon currentIcon = icon();
    if(!currentIcon.isNull())
    {
        // Calculate icon position (centered horizontally, in upper portion)
        QSize iconSize = this->iconSize();
        int iconX = (width() - iconSize.width()) / 2;
        int iconY = 8; // Small margin from top

        QRect iconRect(iconX, iconY, iconSize.width(), iconSize.height());

        // Draw the icon
        QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        if(isDown())
            mode = QIcon::Selected;
        currentIcon.paint(&painter, iconRect, Qt::AlignCenter, mode);
    }

    // Draw the text below the icon
    QString buttonText = text();
    if(!buttonText.isEmpty())
    {
        // Calculate text area (below icon)
        int textY = 8 + iconSize().height() + 4; // Icon position + icon height + small gap
        int textHeight = height() - textY - 8;   // Remaining height minus bottom margin

        QRect textRect(4, textY, width() - 8, textHeight); // Small horizontal margins

        // Set up text drawing
        painter.setPen(isEnabled() ? palette().color(QPalette::ButtonText)
                                   : palette().color(QPalette::Disabled, QPalette::ButtonText));
        painter.setFont(font());

        // Draw text centered and with word wrapping
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, buttonText);
    }
}
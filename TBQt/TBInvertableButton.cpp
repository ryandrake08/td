#include "TBInvertableButton.hpp"

#include "TBImageInversionHelper.hpp"

#include <QIcon>

TBInvertableButton::TBInvertableButton(QWidget* parent) : QPushButton(parent), m_imageInverted(false)
{
}

TBInvertableButton::TBInvertableButton(const QString& text, QWidget* parent) : QPushButton(text, parent), m_imageInverted(false)
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

void TBInvertableButton::setImageInverted(bool inverted)
{
    if (m_imageInverted == inverted)
    {
        return;
    }

    m_imageInverted = inverted;
    updateButtonIcon();
}

void TBInvertableButton::updateButtonIcon()
{
    if (m_imageInverted && !m_invertedIcon.isNull())
    {
        setIcon(m_invertedIcon);
    }
    else if (!m_originalIcon.isNull())
    {
        setIcon(m_originalIcon);
    }
}
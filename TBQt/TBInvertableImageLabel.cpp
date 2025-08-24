#include "TBInvertableImageLabel.hpp"

#include "TBImageInversionHelper.hpp"

#include <QPixmap>

TBInvertableImageLabel::TBInvertableImageLabel(QWidget* parent) : QLabel(parent), m_imageInverted(false)
{
}

void TBInvertableImageLabel::setOriginalPixmap(const QPixmap& pixmap)
{
    m_originalPixmap = pixmap;
    m_invertedPixmap = TBImageInversionHelper::createInvertedPixmap(pixmap);

    // Update display based on current inversion state
    this->updateImagePixmap();
}

bool TBInvertableImageLabel::imageInverted() const
{
    return m_imageInverted;
}

void TBInvertableImageLabel::setImageInverted(bool inverted)
{
    if (m_imageInverted == inverted)
    {
        return;
    }

    m_imageInverted = inverted;
    this->updateImagePixmap();
}

void TBInvertableImageLabel::updateImagePixmap()
{
    if (m_imageInverted && !m_invertedPixmap.isNull())
    {
        setPixmap(m_invertedPixmap);
    }
    else if (!m_originalPixmap.isNull())
    {
        setPixmap(m_originalPixmap);
    }
}
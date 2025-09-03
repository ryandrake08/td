#include "TBScalableLabel.hpp"

#include <QPainter>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QRect>
#include <QFontDatabase>
#include <QApplication>

TBScalableLabel::TBScalableLabel(QWidget* parent) : QLabel(parent), m_minimumFontSize(8), m_maximumFontSize(72)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAlignment(Qt::AlignCenter);
    setWordWrap(true);
}

TBScalableLabel::TBScalableLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent), m_minimumFontSize(8), m_maximumFontSize(72)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAlignment(Qt::AlignCenter);
    setWordWrap(true);
}

void TBScalableLabel::setMinimumFontSize(int size)
{
    m_minimumFontSize = qMax(1, size);
    updateFontSize();
}

void TBScalableLabel::setMaximumFontSize(int size)
{
    m_maximumFontSize = qMax(m_minimumFontSize, size);
    updateFontSize();
}

int TBScalableLabel::minimumFontSize() const
{
    return m_minimumFontSize;
}

int TBScalableLabel::maximumFontSize() const
{
    return m_maximumFontSize;
}

void TBScalableLabel::setFontFamily(const QString& family)
{
    m_fontFamily = family;
    updateFontSize();
}

QString TBScalableLabel::fontFamily() const
{
    return m_fontFamily;
}

void TBScalableLabel::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    updateFontSize();
}

void TBScalableLabel::paintEvent(QPaintEvent* event)
{
    // Use the base class paint event with our dynamically sized font
    QLabel::paintEvent(event);
}

void TBScalableLabel::updateFontSize()
{
    if (text().isEmpty() || size().isEmpty()) {
        return;
    }

    // Calculate the optimal font size for the current widget size
    QRect availableRect = contentsRect();
    int optimalSize = calculateOptimalFontSize(availableRect);

    // Clamp to our min/max range
    optimalSize = qBound(m_minimumFontSize, optimalSize, m_maximumFontSize);

    // Update font if size changed
    QFont currentFont = font();
    if (currentFont.pointSize() != optimalSize || (!m_fontFamily.isEmpty() && currentFont.family() != m_fontFamily))
    {
        currentFont.setPointSize(optimalSize);
        if (!m_fontFamily.isEmpty())
        {
            QString styleName;
            if (currentFont.bold())
            {
                styleName = "Bold";
            }
            else
            {
                styleName = "Regular";
            }
            QFontDatabase fontDb;
            if (fontDb.hasFamily(m_fontFamily))
            {
                currentFont.setFamily(m_fontFamily);
                if (!fontDb.styles(m_fontFamily).contains(styleName))
                {
                    // Fallback to maintaining bold weight if specific style not available
                    currentFont.setBold(currentFont.bold());
                }
            }
        }
        setFont(currentFont);
    }
}

int TBScalableLabel::calculateOptimalFontSize(const QRect& rect) const
{
    if (rect.width() <= 0 || rect.height() <= 0 || text().isEmpty()) {
        return m_minimumFontSize;
    }

    // Binary search for optimal font size
    int minSize = m_minimumFontSize;
    int maxSize = m_maximumFontSize;
    int optimalSize = minSize;

    while (minSize <= maxSize)
    {
        int testSize = (minSize + maxSize) / 2;

        QFont testFont = font();
        testFont.setPointSize(testSize);
        if (!m_fontFamily.isEmpty())
        {
            QString styleName;
            if (testFont.bold())
            {
                styleName = "Bold";
            }
            else
            {
                styleName = "Regular";
            }
            QFontDatabase fontDb;
            if (fontDb.hasFamily(m_fontFamily))
            {
                testFont.setFamily(m_fontFamily);
                if (!fontDb.styles(m_fontFamily).contains(styleName))
                {
                    // Fallback to maintaining bold weight if specific style not available
                    testFont.setBold(testFont.bold());
                }
            }
        }
        QFontMetrics fm(testFont);

        QRect boundingRect;
        if (wordWrap())
        {
            // For word-wrapped text, calculate bounding rectangle
            boundingRect = fm.boundingRect(rect, alignment() | Qt::TextWordWrap, text());
        }
        else
        {
            // For single-line text
            boundingRect = fm.boundingRect(text());
        }

        // Check if text fits within available space (with some padding)
        const int padding = 4;
        if (boundingRect.width() <= rect.width() - padding && boundingRect.height() <= rect.height() - padding)
        {
            optimalSize = testSize;
            minSize = testSize + 1;  // Try larger
        }
        else
        {
            maxSize = testSize - 1;  // Try smaller
        }
    }

    return optimalSize;
}
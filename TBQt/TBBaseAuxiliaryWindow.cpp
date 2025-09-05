#include "TBBaseAuxiliaryWindow.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QPalette>
#include <QRegExp>
#include <QString>
#include <QWidget>
#include <QtMath>

TBBaseAuxiliaryWindow::TBBaseAuxiliaryWindow(QWidget* parent) : QWidget(parent)
{
    // Set window attributes for proper top-level window behavior
    setAttribute(Qt::WA_DeleteOnClose, true); // Allow user to close and delete
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
}

TBBaseAuxiliaryWindow::~TBBaseAuxiliaryWindow() = default;

void TBBaseAuxiliaryWindow::closeEvent(QCloseEvent* event)
{
    // Emit signal to notify parent that we're closing
    Q_EMIT windowClosed();

    // Accept the close event - this will delete the widget due to WA_DeleteOnClose
    event->accept();
}

bool colorIsDark(const QColor& color)
{
    // Calculate relative luminance using ITU-R BT.601 coefficients
    return 0.299f * color.redF() + 0.587f * color.greenF() + 0.114f * color.blueF() < 0.5;
}

void TBBaseAuxiliaryWindow::setBackgroundColorString(const QString& backgroundColorString)
{
    // Default: empty stylesheet restores system defaults
    QString styleSheet = QString();

    // Check if we have a valid custom color
    if(!backgroundColorString.isEmpty())
    {
        QColor backgroundColor(backgroundColorString);
        if(backgroundColor.isValid())
        {
            // Choose appropriate text color based on background darkness
            QColor textColor = colorIsDark(backgroundColor) ? QColor("white") : QColor("black");

            // Apply custom window and text colors
            QString className = this->metaObject()->className();
            styleSheet = QString("%1 { background-color: %2; color: %3; }").arg(className, backgroundColor.name(), textColor.name());

            // Make all child widgets transparent so parent background shows through
            styleSheet += QString(" %1 * { background-color: transparent; color: %2; }").arg(className, textColor.name());
        }
    }

    // Check before and after if darkness is actually changing
    bool currentIsDark = this->isBackgroundDark();
    this->setStyleSheet(styleSheet);
    bool isDark = this->isBackgroundDark();

    // Only emit signal if the dark status actually changed
    if(isDark != currentIsDark)
    {
        Q_EMIT backgroundIsDarkChanged(isDark);
    }
}

bool TBBaseAuxiliaryWindow::isBackgroundDark() const
{
    // Get the effective background color for this widget
    QPalette palette = this->palette();
    QColor backgroundColor = palette.color(QPalette::Window);

    // If we have a custom stylesheet, try to extract background color from it
    QString currentStyleSheet = this->styleSheet();
    if(!currentStyleSheet.isEmpty())
    {
        QRegExp bgColorRegex(QString("%1.*background-color:\\s*([^;]+)").arg(this->metaObject()->className()));
        if(bgColorRegex.indexIn(currentStyleSheet) != -1)
        {
            QColor stylesheetColor(bgColorRegex.cap(1).trimmed());
            if(stylesheetColor.isValid())
            {
                backgroundColor = stylesheetColor;
            }
        }
    }

    return colorIsDark(backgroundColor);
}

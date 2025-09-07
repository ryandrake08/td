#include "TBBackgroundMonitor.hpp"
#include "TBColorUtils.hpp"
#include "TBInvertableButton.hpp"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QPalette>
#include <QPointer>
#include <QRegExp>
#include <QStyle>
#include <QTimer>
#include <QWidget>

struct TBBackgroundMonitor::impl
{
    QWidget* watchedWidget;
    bool lastKnownDark;
    QList<QPointer<TBInvertableButton>> buttons;

    explicit impl(QWidget* parent) : watchedWidget(parent), lastKnownDark(this->calculateIsDark())
    {
    }

    bool calculateIsDark() const
    {
        if(!watchedWidget)
        {
            return false;
        }

        // Get the effective background color for the watched widget
        QPalette palette = watchedWidget->palette();
        QColor backgroundColor = palette.color(QPalette::Window);

        // If widget has a custom stylesheet, try to extract background color from it
        QString currentStyleSheet = watchedWidget->styleSheet();
        if(!currentStyleSheet.isEmpty())
        {
            // Try to extract background-color from stylesheet
            QRegExp bgColorRegex("background-color:\\s*([^;]+)");
            if(bgColorRegex.indexIn(currentStyleSheet) != -1)
            {
                QColor stylesheetColor(bgColorRegex.cap(1).trimmed());
                if(stylesheetColor.isValid())
                {
                    backgroundColor = stylesheetColor;
                }
            }
        }

        return TBColorUtils::colorIsDark(backgroundColor);
    }

    void updateAllButtons(bool isDark)
    {
        // Clean up destroyed buttons and update remaining ones
        for(auto it = buttons.begin(); it != buttons.end();)
        {
            TBInvertableButton* button = *it;
            if(!button)
            {
                // Button was destroyed, remove from list
                it = buttons.erase(it);
            }
            else
            {
                // Update button
                button->setImageInverted(isDark);
                ++it;
            }
        }
    }
};

TBBackgroundMonitor::TBBackgroundMonitor(QWidget* parent) : QObject(parent), pimpl(new impl(parent))
{
    if(!pimpl->watchedWidget)
    {
        qWarning() << "TBBackgroundMonitor: null watched widget provided";
        return;
    }

    // Install event filter on watched widget to catch palette/theme changes
    pimpl->watchedWidget->installEventFilter(this);

    // Also install on QApplication to catch system-wide palette changes
    QApplication::instance()->installEventFilter(this);

    // Watch for widget destruction
    QObject::connect(pimpl->watchedWidget, &QWidget::destroyed, this, &TBBackgroundMonitor::onWatchedWidgetDestroyed);
}

TBBackgroundMonitor::~TBBackgroundMonitor() = default;

void TBBackgroundMonitor::registerButton(TBInvertableButton* button)
{
    if(!button)
    {
        return;
    }

    // Check if already registered
    for(const auto& buttonPointer : pimpl->buttons)
    {
        if(buttonPointer == button)
        {
            return; // Already registered
        }
    }

    // Add to list
    pimpl->buttons.append(QPointer<TBInvertableButton>(button));

    // Set initial state
    button->setImageInverted(pimpl->lastKnownDark);
}

void TBBackgroundMonitor::unregisterButton(TBInvertableButton* button)
{
    if(!button)
    {
        return;
    }

    // Remove from list
    for(auto it = pimpl->buttons.begin(); it != pimpl->buttons.end(); ++it)
    {
        if(*it == button)
        {
            pimpl->buttons.erase(it);
            break;
        }
    }
}

bool TBBackgroundMonitor::eventFilter(QObject* watched, QEvent* event)
{
    if(pimpl->watchedWidget)
    {
        // Check for events that might change the color scheme
        bool shouldCheck = false;

        if(event->type() == QEvent::ApplicationPaletteChange)
        {
            shouldCheck = true;
        }
        else if(event->type() == QEvent::ThemeChange)
        {
            shouldCheck = true;
        }
        else if(event->type() == QEvent::PaletteChange && watched == pimpl->watchedWidget)
        {
            shouldCheck = true;
        }
        else if(event->type() == QEvent::StyleChange && watched == pimpl->watchedWidget)
        {
            shouldCheck = true;
        }

        if(shouldCheck)
        {
            // Use QTimer::singleShot to defer the check until after event processing
            // This ensures palette changes are fully propagated before we check
            QTimer::singleShot(0, this, &TBBackgroundMonitor::checkAndUpdateButtons);
        }
    }

    return QObject::eventFilter(watched, event);
}

void TBBackgroundMonitor::checkAndUpdateButtons()
{
    if(pimpl->watchedWidget)
    {
        bool currentlyDark = pimpl->calculateIsDark();

        if(currentlyDark != pimpl->lastKnownDark)
        {
            pimpl->lastKnownDark = currentlyDark;
            pimpl->updateAllButtons(currentlyDark);
            Q_EMIT backgroundDarknessChanged(currentlyDark);
        }
    }
}

void TBBackgroundMonitor::onWatchedWidgetDestroyed()
{
    // Remove event filters
    if(QApplication::instance())
    {
        QApplication::instance()->removeEventFilter(this);
    }

    pimpl->watchedWidget = nullptr;
}

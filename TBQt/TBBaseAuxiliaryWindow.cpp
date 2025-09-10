#include "TBBaseAuxiliaryWindow.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QPalette>
#include <QRegExp>
#include <QScreen>
#include <QSettings>
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

void TBBaseAuxiliaryWindow::setBackgroundColorString(const QString& backgroundColorString)
{
    // Check if we have a valid custom color
    if(!backgroundColorString.isEmpty())
    {
        QColor backgroundColor(backgroundColorString);
        if(backgroundColor.isValid())
        {
            bool isDark = backgroundColor.lightnessF() < 0.5;

            // Choose appropriate text color based on background darkness
            QColor textColor = isDark ? Qt::white : Qt::black;

            // Create modified palette
            QPalette palette = this->palette();
            palette.setColor(QPalette::Window, backgroundColor);
            palette.setColor(QPalette::WindowText, textColor);
            palette.setColor(QPalette::Button, backgroundColor);
            palette.setColor(QPalette::ButtonText, textColor);
            palette.setColor(QPalette::Base, backgroundColor);
            palette.setColor(QPalette::AlternateBase, backgroundColor);
            palette.setColor(QPalette::Text, textColor);

            // Apply palette to widget and child widgets
            this->setPalette(palette);
            for(QWidget* child : this->findChildren<QWidget*>())
            {
                child->setPalette(palette);
            }

            // Override icons for this specific background color
            this->overrideIconsForBackground(isDark);
        }
    }
    else
    {
        // Restore default palette to widget and child widgets
        QPalette palette = QApplication::palette();
        this->setPalette(palette);
        for(QWidget* child : this->findChildren<QWidget*>())
        {
            child->setPalette(palette);
        }

        // Restore theme-based icons
        this->restoreThemeBasedIcons();
    }
}

void TBBaseAuxiliaryWindow::showUsingDisplaySettings(const QString& windowType)
{
    QSettings settings;

    // Get available screens
    QList<QScreen*> screens = QApplication::screens();

    // Get the screen index for this window type
    int screenIndex = settings.value(QString("Display/%1Screen").arg(windowType), 0).toInt();
    if(screenIndex >= 0 && screenIndex < screens.size())
    {

        // Move window to the specified screen
        QScreen* targetScreen = screens[screenIndex];
        QRect screenGeometry = targetScreen->geometry();

        // Move to the screen and then go fullscreen
        this->move(screenGeometry.topLeft());
    }

    // Check if this window type should start fullscreen
    bool startFullscreen = settings.value(QString("Display/%1Fullscreen").arg(windowType), false).toBool();
    if(startFullscreen)
    {
        // Show fullscreen
        this->showFullScreen();
    }
    else
    {
        // Show normally
        this->show();
    }

    // Always raise to the top and activate
    this->raise();
    this->activateWindow();
}

void TBBaseAuxiliaryWindow::overrideIconsForBackground(bool /* isDark */)
{
}

void TBBaseAuxiliaryWindow::restoreThemeBasedIcons()
{
}

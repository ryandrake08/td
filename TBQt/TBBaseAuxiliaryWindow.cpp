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
            // Create modified palette instead of using CSS stylesheets
            QPalette palette = this->palette();
            palette.setColor(QPalette::Window, backgroundColor);

            // Choose appropriate text color based on background darkness
            QColor textColor = backgroundColor.lightnessF() < 0.5 ? Qt::white : Qt::black;

            // Set palette colors that Qt template icons can access
            palette.setColor(QPalette::WindowText, textColor);
            palette.setColor(QPalette::ButtonText, textColor);
            palette.setColor(QPalette::Text, textColor);

            // Apply palette and enable auto-fill background
            this->setPalette(palette);
            this->setAutoFillBackground(true);

            // Clear any existing stylesheets that would override palette
            this->setStyleSheet(QString());
        }
    }
    else
    {
        // Restore default palette and disable custom background
        this->setPalette(QApplication::palette());
        this->setAutoFillBackground(false);
        this->setStyleSheet(QString());
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

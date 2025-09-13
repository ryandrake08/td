#include "TBBaseMainWindow.hpp"

#include "TBActionClockWindow.hpp"
#include "TBSeatingChartWindow.hpp"
#include "TBSettingsDialog.hpp"
#include "TBSoundPlayer.hpp"
#include "TBTournamentDisplayWindow.hpp"

#include "TournamentSession.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QIcon>
#include <QMessageBox>
#include <QPalette>

struct TBBaseMainWindow::impl
{
    // tournament session
    TournamentSession session;

    // common child windows
    TBSeatingChartWindow* seatingChartWindow { nullptr };
    TBTournamentDisplayWindow* displayWindow { nullptr };
    TBActionClockWindow* actionClockWindow { nullptr };

    // sound notifications
    TBSoundPlayer* soundPlayer;

    explicit impl(TBBaseMainWindow* parent) : soundPlayer(new TBSoundPlayer(session, parent)) {}
};

TBBaseMainWindow::TBBaseMainWindow(QWidget* parent) : QMainWindow(parent), pimpl(new impl(this))
{
    // set up rest of window
    this->setUnifiedTitleAndToolBarOnMac(true);

    // Set initial theme based on current system palette
    QColor windowColor = QApplication::palette().color(QPalette::Window);
    QIcon::setThemeName(windowColor.lightnessF() < 0.5 ? "dark_theme" : "light_theme");
}

TBBaseMainWindow::~TBBaseMainWindow() = default;

// Events

void TBBaseMainWindow::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange)
    {
        QColor windowColor = QApplication::palette().color(QPalette::Window);
        QIcon::setThemeName(windowColor.lightnessF() < 0.5 ? "dark_theme" : "light_theme");
    }

    QMainWindow::changeEvent(event);
}

void TBBaseMainWindow::closeEvent(QCloseEvent* /* event */)
{
    // disconnect from session
    pimpl->session.disconnect();
}

TournamentSession& TBBaseMainWindow::getSession()
{
    return pimpl->session;
}

bool TBBaseMainWindow::isSeatingChartWindowVisible() const
{
    return pimpl->seatingChartWindow && pimpl->seatingChartWindow->isVisible();
}

bool TBBaseMainWindow::isDisplayWindowVisible() const
{
    return pimpl->displayWindow && pimpl->displayWindow->isVisible();
}

void TBBaseMainWindow::on_actionPauseResume_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.toggle_pause_game();
    }
    else
    {
        pimpl->session.start_game();
    }
}

void TBBaseMainWindow::on_actionPreviousRound_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_previous_level();
    }
}

void TBBaseMainWindow::on_actionNextRound_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_next_level();
    }
}

void TBBaseMainWindow::on_actionCallClock_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        const auto& actionClockTimeRemaining(pimpl->session.state()["action_clock_time_remaining"].toInt());
        if(actionClockTimeRemaining == 0)
        {
            pimpl->session.set_action_clock(TournamentSession::kActionClockRequestTime);
        }
        else
        {
            pimpl->session.clear_action_clock();
        }
    }
}

void TBBaseMainWindow::on_actionCancelClock_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.clear_action_clock();
    }
}

void TBBaseMainWindow::on_actionEndGame_triggered()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.stop_game();
    }
}

void TBBaseMainWindow::on_actionShowHideSeatingChart_triggered()
{
    if(pimpl->seatingChartWindow && pimpl->seatingChartWindow->isVisible())
    {
        // Close and destroy the window
        pimpl->seatingChartWindow->close();
    }
    else
    {
        // Create new window if it doesn't exist
        if(!pimpl->seatingChartWindow)
        {
            pimpl->seatingChartWindow = new TBSeatingChartWindow(pimpl->session, this);

            // Connect to destroyed signal to handle automatic destruction by closing
            QObject::connect(pimpl->seatingChartWindow, &QObject::destroyed, this, [this]()
            {
                pimpl->seatingChartWindow = nullptr;
                this->updateSeatingChartMenuText();
            });
        }

        // Apply settings and show
        pimpl->seatingChartWindow->showUsingDisplaySettings("SeatingChart");
    }
    this->updateSeatingChartMenuText();
}

void TBBaseMainWindow::on_actionShowHideMainDisplay_triggered()
{
    if(pimpl->displayWindow && pimpl->displayWindow->isVisible())
    {
        // Close and destroy the window
        pimpl->displayWindow->close();
    }
    else
    {
        // Create new window if it doesn't exist
        if(!pimpl->displayWindow)
        {
            pimpl->displayWindow = new TBTournamentDisplayWindow(pimpl->session, this);

            // Connect to destroyed signal to handle automatic destruction by closing
            QObject::connect(pimpl->displayWindow, &QObject::destroyed, this, [this]()
            {
                pimpl->displayWindow = nullptr;
                this->updateDisplayMenuText();
            });

            // Connect TBTournamentDisplayWindow action signals to base main window slots
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::previousRoundRequested, this, &TBBaseMainWindow::on_actionPreviousRound_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::pauseResumeRequested, this, &TBBaseMainWindow::on_actionPauseResume_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::nextRoundRequested, this, &TBBaseMainWindow::on_actionNextRound_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::callClockRequested, this, &TBBaseMainWindow::on_actionCallClock_triggered);
        }

        // Apply settings and show
        pimpl->displayWindow->showUsingDisplaySettings("TournamentDisplay");
    }
    this->updateDisplayMenuText();
}

void TBBaseMainWindow::on_actionMinimize_triggered()
{
    this->showMinimized();
}

void TBBaseMainWindow::on_actionZoom_triggered()
{
    if(this->isMaximized())
    {
        this->showNormal();
    }
    else
    {
        this->showMaximized();
    }
}

void TBBaseMainWindow::on_actionBringAllToFront_triggered()
{
    this->raise();
    this->activateWindow();
}

void TBBaseMainWindow::on_actionHelp_triggered()
{
    QMessageBox message(this);
    message.setIcon(QMessageBox::Information);
    message.setIconPixmap(QIcon::fromTheme("i_application").pixmap(64, 64));
    message.setWindowTitle(QObject::tr("Help"));
    message.setText(QObject::tr("Help isn't available for %1").arg(QCoreApplication::applicationName()));
    message.exec();
}

void TBBaseMainWindow::on_actionAbout_triggered()
{
    // show about box
    QMessageBox message(this);
    message.setIconPixmap(QIcon::fromTheme("i_application").pixmap(64, 64));
    message.setWindowTitle(QObject::tr("About %1...").arg(QCoreApplication::applicationName()));
    message.setText(QCoreApplication::applicationName());
    message.setInformativeText(QObject::tr("Version %1").arg(QCoreApplication::applicationVersion()));
    message.exec();
}

void TBBaseMainWindow::on_actionSettings_triggered()
{
    TBSettingsDialog dialog(this);
    dialog.exec();
}

void TBBaseMainWindow::on_actionExit_triggered()
{
    this->close();
}

void TBBaseMainWindow::updateDisplayMenuText()
{
    // Update menu text based on display window visibility
    bool isVisible = this->isDisplayWindowVisible();
    QString menuText = isVisible ? QObject::tr("Hide Main Display") : QObject::tr("Show Main Display");

    // Find the action by name (both derived classes use the same name)
    auto* action = this->findChild<QAction*>("actionShowHideMainDisplay");
    if(action)
    {
        action->setText(menuText);
    }
}

void TBBaseMainWindow::updateSeatingChartMenuText()
{
    // Update menu text based on seating chart window visibility
    bool isVisible = this->isSeatingChartWindowVisible();
    QString menuText = isVisible ? QObject::tr("Hide Seating Chart") : QObject::tr("Show Seating Chart");

    // Find the action by name (both derived classes use the same name)
    auto* action = this->findChild<QAction*>("actionShowHideSeatingChart");
    if(action)
    {
        action->setText(menuText);
    }
}

void TBBaseMainWindow::updateActionClock(const QVariantMap& state)
{

    int timeRemaining = state["action_clock_time_remaining"].toInt();
    if(timeRemaining == 0)
    {
        // Clock is not active - hide window
        if(pimpl->actionClockWindow)
        {
            pimpl->actionClockWindow->close();
        }
    }
    else
    {
        // If the window is not yet created
        if(!pimpl->actionClockWindow)
        {
            QWidget* parent(this);
            if(pimpl->displayWindow)
            {
                parent = pimpl->displayWindow;
            }

            // Create the window, parent is either the display (if open) or this window
            pimpl->actionClockWindow = new TBActionClockWindow(pimpl->session, parent);

            // Connect to destroyed signal to handle automatic destruction by closing
            QObject::connect(pimpl->actionClockWindow, &QObject::destroyed, this, [this]()
            {
                pimpl->actionClockWindow = nullptr;
                this->on_actionCancelClock_triggered();
            });
        }

        // Show window if not already visible
        if(!pimpl->actionClockWindow->isVisible())
        {
            pimpl->actionClockWindow->showCenteredOverParent();
        }

        // Update the clock itself
        pimpl->actionClockWindow->updateActionClock(state);
    }
}

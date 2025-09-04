#include "TBBaseMainWindow.hpp"

#include "TBSeatingChartWindow.hpp"
#include "TBSoundPlayer.hpp"
#include "TBTournamentDisplayWindow.hpp"

#include "TournamentSession.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QScreen>
#include <QSettings>

struct TBBaseMainWindow::impl
{
    // tournament session
    TournamentSession session;

    // common child windows
    TBSeatingChartWindow* seatingChartWindow;
    TBTournamentDisplayWindow* displayWindow;

    // sound notifications
    TBSoundPlayer* soundPlayer;

    explicit impl(TBBaseMainWindow* parent) : seatingChartWindow(new TBSeatingChartWindow(session, parent)), displayWindow(new TBTournamentDisplayWindow(session, parent)), soundPlayer(new TBSoundPlayer(parent)) {}
};

TBBaseMainWindow::TBBaseMainWindow(QWidget* parent) : QMainWindow(parent), pimpl(new impl(this))
{
    // set up rest of window
    this->setUnifiedTitleAndToolBarOnMac(true);

    // initialize sound player with session
    pimpl->soundPlayer->setSession(pimpl->session);
}

TBBaseMainWindow::~TBBaseMainWindow() = default;

// initializeTournamentSession is now pure virtual - implemented by derived classes

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
    return pimpl->seatingChartWindow->isVisible();
}

bool TBBaseMainWindow::isDisplayWindowVisible() const
{
    return pimpl->displayWindow->isVisible();
}

void TBBaseMainWindow::on_actionExit_triggered()
{
    this->close();
}

void TBBaseMainWindow::on_actionPauseResume_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().toggle_pause_game();
    }
    else
    {
        this->getSession().start_game();
    }
}

void TBBaseMainWindow::on_actionPreviousRound_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().set_previous_level();
    }
}

void TBBaseMainWindow::on_actionNextRound_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().set_next_level();
    }
}

void TBBaseMainWindow::on_actionCallClock_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    const auto& actionClockTimeRemaining(this->getSession().state()["action_clock_time_remaining"].toInt());
    if(current_blind_level != 0 && actionClockTimeRemaining == 0)
    {
        this->getSession().set_action_clock(TournamentSession::kActionClockRequestTime);
    }
}

void TBBaseMainWindow::on_actionClearClock_triggered()
{
    this->getSession().clear_action_clock();
}

void TBBaseMainWindow::on_actionShowHideSeatingChart_triggered()
{
    if(pimpl->seatingChartWindow->isVisible())
    {
        pimpl->seatingChartWindow->hide();
    }
    else
    {
        this->applyDisplaySettings(pimpl->seatingChartWindow, "SeatingChart");
        pimpl->seatingChartWindow->raise();
        pimpl->seatingChartWindow->activateWindow();
    }
    this->updateSeatingChartMenuText();
}

void TBBaseMainWindow::on_actionShowHideMainDisplay_triggered()
{
    if(pimpl->displayWindow->isVisible())
    {
        pimpl->displayWindow->hide();
    }
    else
    {
        this->applyDisplaySettings(pimpl->displayWindow, "TournamentDisplay");
        pimpl->displayWindow->raise();
        pimpl->displayWindow->activateWindow();
    }
    this->updateDisplayMenuText();
}

void TBBaseMainWindow::updateDisplayMenuText()
{
    // Update menu text based on display window visibility
    bool isVisible = this->isDisplayWindowVisible();
    QString menuText = isVisible ? tr("Hide Main Display") : tr("Show Main Display");

    // Find the action by name (both derived classes use the same name)
    QAction* action = this->findChild<QAction*>("actionShowHideMainDisplay");
    if(action)
    {
        action->setText(menuText);
    }
}

void TBBaseMainWindow::updateSeatingChartMenuText()
{
    // Update menu text based on seating chart window visibility
    bool isVisible = this->isSeatingChartWindowVisible();
    QString menuText = isVisible ? tr("Hide Seating Chart") : tr("Show Seating Chart");

    // Find the action by name (both derived classes use the same name)
    QAction* action = this->findChild<QAction*>("actionShowHideSeatingChart");
    if(action)
    {
        action->setText(menuText);
    }
}

void TBBaseMainWindow::applyDisplaySettings(QMainWindow* window, const QString& windowType)
{
    if (!window) {
        return;
    }
    
    QSettings settings;
    
    // Check if this window type should start fullscreen
    bool startFullscreen = settings.value(QString("Display/%1Fullscreen").arg(windowType), false).toBool();
    
    if (startFullscreen) {
        // Get the screen index for this window type
        int screenIndex = settings.value(QString("Display/%1Screen").arg(windowType), 0).toInt();
        
        // Get available screens
        QList<QScreen*> screens = QApplication::screens();
        
        if (screenIndex >= 0 && screenIndex < screens.size()) {
            // Move window to the specified screen
            QScreen* targetScreen = screens[screenIndex];
            QRect screenGeometry = targetScreen->geometry();
            
            // Move to the screen and then go fullscreen
            window->move(screenGeometry.topLeft());
        }
        
        // Show fullscreen
        window->showFullScreen();
    } else {
        // Show normally
        window->show();
    }
}

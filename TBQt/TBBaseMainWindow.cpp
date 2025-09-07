#include "TBBaseMainWindow.hpp"

#include "TBSeatingChartWindow.hpp"
#include "TBSoundPlayer.hpp"
#include "TBTournamentDisplayWindow.hpp"

#include "TournamentSession.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>

struct TBBaseMainWindow::impl
{
    // tournament session
    TournamentSession session;

    // common child windows
    TBSeatingChartWindow* seatingChartWindow { nullptr };
    TBTournamentDisplayWindow* displayWindow { nullptr };

    // sound notifications
    TBSoundPlayer* soundPlayer;

    explicit impl(TBBaseMainWindow* parent) : soundPlayer(new TBSoundPlayer(parent)) {}
};

TBBaseMainWindow::TBBaseMainWindow(QWidget* parent) : QMainWindow(parent), pimpl(new impl(this))
{
    // set up rest of window
    this->setUnifiedTitleAndToolBarOnMac(true);

    // initialize sound player with session
    pimpl->soundPlayer->setSession(pimpl->session);

    // Connect action request signals to state-dependent handlers
    QObject::connect(this, &TBBaseMainWindow::pauseResumeRequested, this, [this]() {
        this->handlePauseResumeAction(pimpl->session.state());
    });
    QObject::connect(this, &TBBaseMainWindow::previousRoundRequested, this, [this]() {
        this->handlePreviousRoundAction(pimpl->session.state());
    });
    QObject::connect(this, &TBBaseMainWindow::nextRoundRequested, this, [this]() {
        this->handleNextRoundAction(pimpl->session.state());
    });
    QObject::connect(this, &TBBaseMainWindow::callClockRequested, this, [this]() {
        this->handleCallClockAction(pimpl->session.state());
    });
    QObject::connect(this, &TBBaseMainWindow::clearClockRequested, this, &TBBaseMainWindow::handleClearClockAction);
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
    return pimpl->seatingChartWindow && pimpl->seatingChartWindow->isVisible();
}

bool TBBaseMainWindow::isDisplayWindowVisible() const
{
    return pimpl->displayWindow && pimpl->displayWindow->isVisible();
}

void TBBaseMainWindow::on_actionExit_triggered()
{
    this->close();
}

void TBBaseMainWindow::on_actionPauseResume_triggered()
{
    Q_EMIT pauseResumeRequested();
}

void TBBaseMainWindow::on_actionPreviousRound_triggered()
{
    Q_EMIT previousRoundRequested();
}

void TBBaseMainWindow::on_actionNextRound_triggered()
{
    Q_EMIT nextRoundRequested();
}

void TBBaseMainWindow::on_actionCallClock_triggered()
{
    Q_EMIT callClockRequested();
}

void TBBaseMainWindow::on_actionClearClock_triggered()
{
    Q_EMIT clearClockRequested();
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
            // Connect to window closed signal to clear our pointer when user closes it
            QObject::connect(pimpl->seatingChartWindow, &TBSeatingChartWindow::windowClosed, this, [this]()
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
            // Connect to window closed signal to clear our pointer when user closes it
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::windowClosed, this, [this]()
            {
                pimpl->displayWindow = nullptr;
                this->updateDisplayMenuText();
            });
            
            // Connect TBTournamentDisplayWindow action signals to base main window signals
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::previousLevelRequested, this, &TBBaseMainWindow::previousRoundRequested);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::pauseToggleRequested, this, &TBBaseMainWindow::pauseResumeRequested);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::nextLevelRequested, this, &TBBaseMainWindow::nextRoundRequested);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::actionClockStartRequested, this, &TBBaseMainWindow::callClockRequested);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::actionClockClearRequested, this, &TBBaseMainWindow::clearClockRequested);
        }

        // Apply settings and show
        pimpl->displayWindow->showUsingDisplaySettings("TournamentDisplay");
    }
    this->updateDisplayMenuText();
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

void TBBaseMainWindow::handlePauseResumeAction(const QVariantMap& state)
{
    const auto& current_blind_level(state["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.toggle_pause_game();
    }
    else
    {
        pimpl->session.start_game();
    }
}

void TBBaseMainWindow::handlePreviousRoundAction(const QVariantMap& state)
{
    const auto& current_blind_level(state["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_previous_level();
    }
}

void TBBaseMainWindow::handleNextRoundAction(const QVariantMap& state)
{
    const auto& current_blind_level(state["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_next_level();
    }
}

void TBBaseMainWindow::handleCallClockAction(const QVariantMap& state)
{
    const auto& current_blind_level(state["current_blind_level"].toInt());
    const auto& actionClockTimeRemaining(state["action_clock_time_remaining"].toInt());
    if(current_blind_level != 0 && actionClockTimeRemaining == 0)
    {
        pimpl->session.set_action_clock(TournamentSession::kActionClockRequestTime);
    }
}

void TBBaseMainWindow::handleClearClockAction()
{
    pimpl->session.clear_action_clock();
}

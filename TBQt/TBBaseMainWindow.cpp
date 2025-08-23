#include "TBBaseMainWindow.hpp"

#include "TBSeatingChartWindow.hpp"
#include "TBTournamentDisplayWindow.hpp"

#include "TournamentSession.hpp"

#include <QCloseEvent>

struct TBBaseMainWindow::impl
{
    // tournament session
    TournamentSession session;

    // common child windows
    TBSeatingChartWindow* seatingChartWindow;
    TBTournamentDisplayWindow* displayWindow;

    explicit impl(TBBaseMainWindow* parent) : seatingChartWindow(new TBSeatingChartWindow(session, parent)), displayWindow(new TBTournamentDisplayWindow(session, parent)) {}
};

TBBaseMainWindow::TBBaseMainWindow(QWidget* parent) : QMainWindow(parent), pimpl(new impl(this))
{
    // set up rest of window
    this->setUnifiedTitleAndToolBarOnMac(true);
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
    if(current_blind_level != 0)
    {
        const auto& action_clock_time_remaining(this->getSession().state()["action_clock_time_remaining"].toInt());
        if(action_clock_time_remaining == 0)
        {
            this->getSession().set_action_clock(TournamentSession::kActionClockRequestTime);
        }
        else
        {
            this->getSession().clear_action_clock();
        }
    }
}

void TBBaseMainWindow::on_actionShowHideSeatingChart_triggered()
{
    if (pimpl->seatingChartWindow->isVisible())
    {
        pimpl->seatingChartWindow->hide();
    }
    else
    {
        pimpl->seatingChartWindow->show();
        pimpl->seatingChartWindow->raise();
        pimpl->seatingChartWindow->activateWindow();
    }
    this->updateSeatingChartMenuText();
}

void TBBaseMainWindow::on_actionShowHideMainDisplay_triggered()
{
    if (pimpl->displayWindow->isVisible())
    {
        pimpl->displayWindow->hide();
    }
    else
    {
        pimpl->displayWindow->show();
        pimpl->displayWindow->raise();
        pimpl->displayWindow->activateWindow();
    }
    this->updateDisplayMenuText();
}

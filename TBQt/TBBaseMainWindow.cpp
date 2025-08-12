#include "TBBaseMainWindow.hpp"
#include "TournamentSession.hpp"

#include <QDebug>
#include <QCloseEvent>

struct TBBaseMainWindow::impl
{
    // tournament session
    TournamentSession session;
};

TBBaseMainWindow::TBBaseMainWindow(QWidget* parent) : QMainWindow(parent), pimpl(new impl)
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

void TBBaseMainWindow::pauseResumeAction()
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

void TBBaseMainWindow::previousRoundAction()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_previous_level();
    }
}

void TBBaseMainWindow::nextRoundAction()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_next_level();
    }
}

void TBBaseMainWindow::callClockAction()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        const auto& action_clock_time_remaining(pimpl->session.state()["action_clock_time_remaining"].toInt());
        if(action_clock_time_remaining == 0)
        {
            pimpl->session.set_action_clock();
        }
        else
        {
            pimpl->session.clear_action_clock();
        }
    }
}

TournamentSession& TBBaseMainWindow::getSession()
{
    return pimpl->session;
}
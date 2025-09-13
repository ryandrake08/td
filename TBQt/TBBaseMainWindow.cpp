#include "TBBaseMainWindow.hpp"

#include "TBSeatingChartWindow.hpp"
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

void TBBaseMainWindow::on_actionExit_triggered()
{
    this->close();
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
    const auto& actionClockTimeRemaining(pimpl->session.state()["action_clock_time_remaining"].toInt());
    if(current_blind_level != 0 && actionClockTimeRemaining == 0)
    {
        pimpl->session.set_action_clock(TournamentSession::kActionClockRequestTime);
    }
}

void TBBaseMainWindow::on_actionClearClock_triggered()
{
    pimpl->session.clear_action_clock();
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
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::previousLevelRequested, this, &TBBaseMainWindow::on_actionPreviousRound_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::pauseToggleRequested, this, &TBBaseMainWindow::on_actionPauseResume_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::nextLevelRequested, this, &TBBaseMainWindow::on_actionNextRound_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::actionClockStartRequested, this, &TBBaseMainWindow::on_actionCallClock_triggered);
            QObject::connect(pimpl->displayWindow, &TBTournamentDisplayWindow::actionClockClearRequested, this, &TBBaseMainWindow::on_actionClearClock_triggered);
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
    QMessageBox helpBox(this);
    helpBox.setIcon(QMessageBox::Information);
    helpBox.setIconPixmap(QIcon::fromTheme("i_application").pixmap(64, 64));
    helpBox.setWindowTitle(QObject::tr("Help"));
    helpBox.setText(QObject::tr("Help isn't available for %1").arg(QCoreApplication::applicationName()));
    helpBox.exec();
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

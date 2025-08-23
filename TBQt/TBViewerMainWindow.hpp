#pragma once

#include "TBBaseMainWindow.hpp"
#include "TournamentService.hpp"
#include <memory>

class TBSeatingChartWindow;
class TBTournamentDisplayWindow;

class TBViewerMainWindow : public TBBaseMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateDisplayMenuText();
    void updateSeatingChartMenuText();

private Q_SLOTS:
    // ui slots - limited viewer functionality
    void on_actionAbout_Poker_Remote_triggered();
    void on_actionExit_triggered();
    void on_actionConnectToTournament_triggered();
    void on_actionDisconnect_triggered();
    void on_actionPauseResume_triggered();
    void on_actionPreviousRound_triggered();
    void on_actionNextRound_triggered();
    void on_actionCallClock_triggered();
    void on_actionShowHideSeatingChart_triggered();
    void on_actionShowHideMainDisplay_triggered();

    // other slots
    void on_authorizedChanged(bool auth) override;
    void on_connectedChanged(bool connected);

public:
    // create a viewer main window
    TBViewerMainWindow();
    virtual ~TBViewerMainWindow();

    // connect to a tournament service
    void connectToTournament(const TournamentService& service);
};
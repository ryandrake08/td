#pragma once

#include "TBBaseMainWindow.hpp"
#include "TournamentService.hpp"
#include <memory>

class TBViewerMainWindow : public TBBaseMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

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

    // other slots
    void on_authorizedChanged(bool auth) override;
    void on_connectedChanged(bool connected);
    void on_tournamentStateChanged(const QString& key, const QVariant& value);

private:
    // tournament display update methods
    void updateTournamentDisplay();
    void updateTournamentInfo(const QVariantMap& state);
    void updateTournamentStats(const QVariantMap& state);
    void updateTournamentClock(const QVariantMap& state);
    void updateModels(const QVariantMap& state);

public:
    // create a viewer main window
    TBViewerMainWindow();
    virtual ~TBViewerMainWindow();

    // connect to a tournament service
    void connectToTournament(const TournamentService& service);
};
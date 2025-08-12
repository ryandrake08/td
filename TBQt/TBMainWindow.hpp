#pragma once

#include "TBBaseMainWindow.hpp"
#include <memory>

class TBMainWindow : public TBBaseMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

private Q_SLOTS:
    // ui slots
    void on_actionAbout_Poker_Buddy_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();
    void on_actionQuickStart_triggered();
    void on_actionReset_triggered();
    void on_actionConfigure_triggered();
    void on_actionAuthorize_triggered();
    void on_actionPlan_triggered();
    void on_actionShowDisplay_triggered();
    void on_actionShowMoves_triggered();
    void on_actionRebalance_triggered();
    void on_actionPauseResume_triggered();
    void on_actionPreviousRound_triggered();
    void on_actionNextRound_triggered();
    void on_actionCallClock_triggered();
    void on_actionEndGame_triggered();
    void on_actionExport_triggered();

    // other slots
    void on_authorizedChanged(bool auth) override;
    void on_filenameChanged(const QString& filename);

public:
    // create a main window
    TBMainWindow();
    virtual ~TBMainWindow();

    // load a document to be managed by this window
    bool load_document(const QString& filename);
};

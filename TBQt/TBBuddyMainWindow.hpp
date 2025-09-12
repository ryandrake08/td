#pragma once

#include "TBBaseMainWindow.hpp"
#include <memory>

class TBBuddyMainWindow : public TBBaseMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateTournamentClock(const QVariantMap& state);
    void updateActionButtons(const QVariantMap& state, bool authorized);
    void updateWindowTitle(const QVariantMap& state, const QString& filename = QString());
    void updateMovementBadge();
    void updateToolbarMenuText();

    // helper functions to show dialogs
    void showPlayerMovements(const QVariantList& movements);

    // recent files management
    void updateRecentFilesMenu();
    void addRecentFile(const QString& filePath);

private Q_SLOTS:
    // ui slots
    void on_actionAbout_Poker_Buddy_triggered();
    void on_actionSettings_triggered();
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionClose_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionQuickStart_triggered();
    void on_actionReset_triggered();
    void on_actionConfigure_triggered();
    void on_actionAuthorize_triggered();
    void on_actionPlan_triggered();
    void on_actionShowMoves_triggered();
    void on_actionRebalance_triggered();
    void on_actionEndGame_triggered();
    void on_actionExport_triggered();
    void on_actionShowHideToolbar_triggered();

    // recent files slots
    void on_openRecentFileTriggered();
    void on_clearRecentFilesTriggered();

    void on_manageButtonClicked(const QModelIndex& index);

    // other slots
    void on_authorizedChanged(bool auth) override;
    void on_filenameChanged(const QString& filename);
    void on_configurationChanged(const QVariantMap& config);
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void on_playerMovementsUpdated(const QVariantList& movements);
    void on_bustPlayer(const QString& playerId);

public:
    // create a main window
    TBBuddyMainWindow();
    virtual ~TBBuddyMainWindow();

    // load a document to be managed by this window
    bool load_document(const QString& filename);
};

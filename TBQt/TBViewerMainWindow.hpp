#pragma once

#include "TBBaseMainWindow.hpp"
#include "TournamentService.hpp"
#include <memory>

class QListWidgetItem;
class TBSeatingChartWindow;
class TBTournamentDisplayWindow;
class TournamentBrowser;

class TBViewerMainWindow : public TBBaseMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateServiceMenu();
    void updateActionButtons(const QVariantMap& state, bool authorized);

private Q_SLOTS:
    // ui slots - limited viewer functionality
    void on_actionConnectToTournament_triggered();
    void on_actionDisconnect_triggered();

    // slots for TournamentBrowser changes
    void on_servicesUpdated(const QVariantList& services);

    // slots for TournamentSession changes
    void on_authorizedChanged(bool auth) override;
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void on_connectedChanged(bool connected);

    // slot for tournament list double-click
    void on_tournamentListItemActivated(QListWidgetItem* item);

public:
    // create a viewer main window
    TBViewerMainWindow();
    virtual ~TBViewerMainWindow();

    // connect to a tournament service
    void connectToTournament(const TournamentService& service);
};
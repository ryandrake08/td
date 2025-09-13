#pragma once

#include "TBBaseMainWindow.hpp"
#include "TournamentService.hpp"
#include <memory>

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

private Q_SLOTS:
    // ui slots - limited viewer functionality
    void on_actionSettings_triggered();
    void on_actionConnectToTournament_triggered();
    void on_actionDisconnect_triggered();

    // other slots
    void on_authorizedChanged(bool auth) override;
    void on_connectedChanged(bool connected);
    void on_servicesUpdated(const QVariantList& services);

public:
    // create a viewer main window
    TBViewerMainWindow();
    virtual ~TBViewerMainWindow();

    // connect to a tournament service
    void connectToTournament(const TournamentService& service);
};
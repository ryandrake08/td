#pragma once

#include "TBBaseAuxiliaryWindow.hpp"
#include <memory>

class TournamentSession;

// Tournament seating chart display window
class TBSeatingChartWindow : public TBBaseAuxiliaryWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateTournamentName();
    void updateTournamentBuyin();
    void updateBackgroundColor();
    void updateSeatingChart();

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);

public:
    explicit TBSeatingChartWindow(TournamentSession& tournamentSession, QWidget* parent = nullptr);
    virtual ~TBSeatingChartWindow() override;
};
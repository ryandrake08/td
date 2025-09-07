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
    void updateFromState(const QVariantMap& state);
    void updateTournamentName(const QVariantMap& state);
    void updateTournamentBuyin(const QVariantMap& state);
    void updateBackgroundColor(const QVariantMap& state);
    void updateSeatingChart(const QVariantMap& state);

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);

public:
    explicit TBSeatingChartWindow(TournamentSession& sess, QWidget* parent = nullptr);
    virtual ~TBSeatingChartWindow() override;
};
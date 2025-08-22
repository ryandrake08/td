#pragma once

#include <QMainWindow>
#include <memory>

class TournamentSession;

// Tournament seating chart display window
class TBSeatingChartWindow : public QMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBSeatingChartWindow(TournamentSession& tournamentSession, QWidget* parent = nullptr);
    virtual ~TBSeatingChartWindow() override;

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void updateSeatingChart();
    void updateWindowTitle();
    void updateTournamentInfo();
    void updateBackgroundColor();

private:
    void rebuildTableWidgets();
};
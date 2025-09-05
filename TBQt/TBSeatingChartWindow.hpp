#pragma once

#include <QWidget>
#include <memory>

class TournamentSession;

// Tournament seating chart display window
class TBSeatingChartWindow : public QWidget
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

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit TBSeatingChartWindow(TournamentSession& tournamentSession, QWidget* parent = nullptr);
    virtual ~TBSeatingChartWindow() override;

Q_SIGNALS:
    void windowClosed();
};
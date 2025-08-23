#pragma once

#include <QMainWindow>
#include <QVariantMap>
#include <memory>

class TournamentSession;

// Tournament display window containing shared tournament viewing functionality
// This window provides a standalone interface for tournament display
class TBTournamentDisplayWindow : public QMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // updaters for various changeable UI controls
    void updateTournamentName();
    void updateTournamentBuyin();
    void updateCurrentRoundNumber();
    void updatePlayersLeft();
    void updateTotalEntries();
    void updateAverageStack();
    void updateElapsedTime();
    void updateTournamentClock();
    void updateCurrentRoundInfo();
    void updateNextRoundInfo();
    void updateAvailableChips();

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void on_previousRoundButtonClicked();
    void on_pauseResumeButtonClicked();
    void on_nextRoundButtonClicked();
    void on_callClockButtonClicked();

public:
    explicit TBTournamentDisplayWindow(TournamentSession& session, QWidget* parent = nullptr);
    virtual ~TBTournamentDisplayWindow() override;
};
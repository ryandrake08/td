#pragma once

#include "TBBaseAuxiliaryWindow.hpp"
#include <QVariantMap>
#include <memory>

class TournamentSession;

// Tournament display window containing shared tournament viewing functionality
// This window provides a standalone interface for tournament display
class TBTournamentDisplayWindow : public TBBaseAuxiliaryWindow
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
    void updateCurrentRoundNumber(const QVariantMap& state);
    void updatePlayersLeft(const QVariantMap& state);
    void updateTotalEntries(const QVariantMap& state);
    void updateAverageStack(const QVariantMap& state);
    void updateElapsedTime(const QVariantMap& state);
    void updateTournamentClock(const QVariantMap& state);
    void updateCurrentRoundInfo(const QVariantMap& state);
    void updateNextRoundInfo(const QVariantMap& state);
    void updateAvailableChips(const QVariantMap& state);

protected:
    // Override window-specific icon switching to access UI objects directly
    void overrideIconsForBackground(bool isDark) override;
    void restoreThemeBasedIcons() override;

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void on_previousRoundButtonClicked();
    void on_pauseResumeButtonClicked();
    void on_nextRoundButtonClicked();
    void on_callClockButtonClicked();
    void onActionClockCanceled();

public:
    explicit TBTournamentDisplayWindow(const TournamentSession& session, QWidget* parent = nullptr);
    virtual ~TBTournamentDisplayWindow() override;

Q_SIGNALS:
    void actionClockClearRequested();
    void previousLevelRequested();
    void pauseToggleRequested();
    void gameStartRequested();
    void nextLevelRequested();
    void actionClockStartRequested();
};
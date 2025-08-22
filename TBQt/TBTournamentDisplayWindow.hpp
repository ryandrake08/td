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

public:
    explicit TBTournamentDisplayWindow(TournamentSession& session, QWidget* parent = nullptr);
    virtual ~TBTournamentDisplayWindow() override;

    // Access to session for external connections
    TournamentSession& getSession() const;

private Q_SLOTS:
    void on_tournamentStateChanged(const QString& key, const QVariant& value);
    void on_previousRoundButtonClicked();
    void on_pauseResumeButtonClicked(); 
    void on_nextRoundButtonClicked();
    void on_callClockButtonClicked();

private:
    void setupUI();
    void connectSignals();
    void updateTournamentDisplay();
    void updateTournamentInfo(const QVariantMap& state);
    void updateTournamentStats(const QVariantMap& state);
    void updateTournamentClock(const QVariantMap& state);
    void updateModels(const QVariantMap& state);
    void updateWindowTitle();
};
#pragma once

#include <QWidget>
#include <QVariantMap>
#include <memory>

class TournamentSession;

// Tournament display widget containing shared tournament viewing functionality
// This widget can be embedded in different contexts without conflicts
class TBTournamentDisplayWidget : public QWidget
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

public:
    explicit TBTournamentDisplayWidget(TournamentSession& session, QWidget* parent = nullptr);
    virtual ~TBTournamentDisplayWidget() override;

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
    void updateTournamentDisplay();
    void updateTournamentInfo(const QVariantMap& state);
    void updateTournamentStats(const QVariantMap& state);
    void updateTournamentClock(const QVariantMap& state);
    void updateModels(const QVariantMap& state);
};
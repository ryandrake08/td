#pragma once

#include <QMainWindow>
#include <memory>

class QEvent;

class TBBaseMainWindow : public QMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

protected Q_SLOTS:
    // common UI slots shared by both derived classes
    void on_actionExit_triggered();
    void on_actionPauseResume_triggered();
    void on_actionPreviousRound_triggered();
    void on_actionNextRound_triggered();
    void on_actionCallClock_triggered();
    void on_actionClearClock_triggered();
    void on_actionShowHideSeatingChart_triggered();
    void on_actionShowHideMainDisplay_triggered();

private Q_SLOTS:
    // virtual slots that derived classes must implement
    virtual void on_authorizedChanged(bool auth) = 0;

    // session state-dependent action handlers
    void handlePauseResumeAction(const QVariantMap& state);
    void handlePreviousRoundAction(const QVariantMap& state);
    void handleNextRoundAction(const QVariantMap& state);
    void handleCallClockAction(const QVariantMap& state);
    void handleClearClockAction();

Q_SIGNALS:
    // signals emitted by UI actions that need session state checking
    void pauseResumeRequested();
    void previousRoundRequested();
    void nextRoundRequested();
    void callClockRequested();
    void clearClockRequested();

public:
    // create a base main window
    TBBaseMainWindow(QWidget* parent = nullptr);
    virtual ~TBBaseMainWindow();

    // override closeEvent to do clean shutdown
    void closeEvent(QCloseEvent* event) override;

    // Handle theme changes for app-wide icon adaptation
    void changeEvent(QEvent* event) override;

protected:
    // accessor methods for derived classes
    class TournamentSession& getSession();

    // common UI update methods
    void updateDisplayMenuText();
    void updateSeatingChartMenuText();

    // visibility accessor methods for child windows
    bool isSeatingChartWindowVisible() const;
    bool isDisplayWindowVisible() const;
};
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
    void on_actionPauseResume_triggered();
    void on_actionPreviousRound_triggered();
    void on_actionNextRound_triggered();
    void on_actionCallClock_triggered();
    void on_actionCancelClock_triggered();
    void on_actionEndGame_triggered();
    void on_actionShowHideSeatingChart_triggered();
    void on_actionShowHideMainDisplay_triggered();
    void on_actionPrintSeatingChart_triggered();
    void on_actionMinimize_triggered();
    void on_actionZoom_triggered();
    void on_actionBringAllToFront_triggered();
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_actionSettings_triggered();
    void on_actionExit_triggered();

private Q_SLOTS:
    // virtual slots that derived classes must implement
    virtual void on_authorizedChanged(bool auth) = 0;

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
    void updateActionClock(const QVariantMap& state);

    // visibility accessor methods for child windows
    bool isSeatingChartWindowVisible() const;
    bool isDisplayWindowVisible() const;
};
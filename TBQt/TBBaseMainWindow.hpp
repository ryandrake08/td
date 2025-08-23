#pragma once

#include <QMainWindow>
#include <memory>

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
    void on_actionShowHideSeatingChart_triggered();
    void on_actionShowHideMainDisplay_triggered();

private Q_SLOTS:
    // virtual slots that derived classes must implement
    virtual void on_authorizedChanged(bool auth) = 0;

public:
    // create a base main window
    TBBaseMainWindow(QWidget* parent = nullptr);
    virtual ~TBBaseMainWindow();

    // override closeEvent to do clean shutdown
    void closeEvent(QCloseEvent* event) override;

protected:
    // accessor methods for derived classes
    class TournamentSession& getSession();

    // pure virtual UI update methods (require UI access)
    virtual void updateDisplayMenuText() = 0;
    virtual void updateSeatingChartMenuText() = 0;

    // visibility accessor methods for child windows
    bool isSeatingChartWindowVisible() const;
    bool isDisplayWindowVisible() const;
};
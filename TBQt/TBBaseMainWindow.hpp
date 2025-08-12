#pragma once

#include <QMainWindow>
#include <memory>

class TBBaseMainWindow : public QMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

private Q_SLOTS:
    // common slots that both applications need
    virtual void on_authorizedChanged(bool auth) = 0;

public:
    // create a base main window
    TBBaseMainWindow(QWidget* parent = nullptr);
    virtual ~TBBaseMainWindow();

    // override closeEvent to do clean shutdown
    void closeEvent(QCloseEvent* event) override;

protected:
    // common tournament control methods
    void pauseResumeAction();
    void previousRoundAction();
    void nextRoundAction();
    void callClockAction();

    // accessor methods for derived classes
    class TournamentSession& getSession();
};
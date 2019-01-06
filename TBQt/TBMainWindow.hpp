#pragma once

#include <QMainWindow>
#include <memory>

class TBMainWindow : public QMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

private Q_SLOTS:
    void on_actionExit_triggered();
    void on_actionExport_triggered();
    void on_actionPauseResume_triggered();
    void on_actionPreviousRound_triggered();
    void on_actionNextRound_triggered();
    void on_actionCallClock_triggered();
    void on_actionEndGame_triggered();
    void on_actionQuickStart_triggered();
    void on_actionReset_triggered();
    void on_authorizedChanged(bool auth);

public:
    // create a main window
    TBMainWindow();
    virtual ~TBMainWindow();

    // load a document to be managed by this window
    bool load_document(const QString& filename);
};

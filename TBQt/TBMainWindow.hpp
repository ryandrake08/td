#pragma once

#include <QMainWindow>
#include <memory>

// forward declare
namespace Ui { class TBMainWindow; }

class TBMainWindow : public QMainWindow
{
    Q_OBJECT

    // pimpl
    struct impl;
    std::unique_ptr<impl> pimpl;

    // moc ui
    std::unique_ptr<Ui::TBMainWindow> ui;

public:
    // create a main window associated with a file (default = no file)
    explicit TBMainWindow(const QString& file = QString());

    // destroy main window
    ~TBMainWindow();

private slots:
    void on_actionExit_triggered();
};

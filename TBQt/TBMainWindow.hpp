#pragma once

#include <QMainWindow>
#include <memory>

// forward declare
namespace Ui { class TBMainWindow; }

class TBMainWindow : public QMainWindow
{
    Q_OBJECT

    // moc ui
    std::unique_ptr<Ui::TBMainWindow> ui;

public:
    explicit TBMainWindow(QWidget* parent = nullptr);
    ~TBMainWindow();

private slots:
    void on_actionExit_triggered();

private:
};

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
    // create a main window
    explicit TBMainWindow();

    // destroy main window
    ~TBMainWindow();

    // load a document to be managed by this window
    bool load_document(const QString& filename);

private Q_SLOTS:
    void on_actionExit_triggered();
};

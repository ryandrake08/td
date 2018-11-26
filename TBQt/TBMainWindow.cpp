#include "TBMainWindow.hpp"
#include "ui_TBMainWindow.h"

TBMainWindow::TBMainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::TBMainWindow)
{
    ui->setupUi(this);
}

TBMainWindow::~TBMainWindow() = default;

void TBMainWindow::on_actionExit_triggered()
{
    this->close();
}

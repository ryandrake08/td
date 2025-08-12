#include "TBViewerMainWindow.hpp"
#include "TBPlayersModel.hpp"
#include "TBRuntimeError.hpp"
#include "TournamentSession.hpp"

#include "ui_TBViewerMainWindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QWidget>

struct TBViewerMainWindow::impl
{
    // moc ui
    Ui::TBViewerMainWindow ui;
};

TBViewerMainWindow::TBViewerMainWindow() : TBBaseMainWindow(), pimpl(new impl)
{
    // set up moc
    this->pimpl->ui.setupUi(this);

    // hook up TournamentSession signals
    QObject::connect(&this->getSession(), SIGNAL(authorizedChanged(bool)), this, SLOT(on_authorizedChanged(bool)));

    // TODO: Viewer should connect to external daemon via service discovery
    // For now, create a placeholder implementation that doesn't start its own daemon
    // This will be completed in Stage 1.2 when we implement proper networking
    qDebug() << "TBViewerMainWindow: Session initialized (external connection not yet implemented)";
}

TBViewerMainWindow::~TBViewerMainWindow() = default;

void TBViewerMainWindow::on_actionAbout_Poker_Remote_triggered()
{
    // show about box
    QMessageBox message(this);
    message.setIconPixmap(QPixmap(":/Resources/icon_64x64.png"));
    message.setWindowTitle(QObject::tr("About %1...").arg("Poker Remote"));
    message.setText("Poker Remote");
    message.setInformativeText(QObject::tr("Version %1").arg(QCoreApplication::applicationVersion()));
    message.exec();
}

void TBViewerMainWindow::on_actionExit_triggered()
{
    // close window
    this->close();
}

void TBViewerMainWindow::on_actionPauseResume_triggered()
{
    this->pauseResumeAction();
}

void TBViewerMainWindow::on_actionPreviousRound_triggered()
{
    this->previousRoundAction();
}

void TBViewerMainWindow::on_actionNextRound_triggered()
{
    this->nextRoundAction();
}

void TBViewerMainWindow::on_actionCallClock_triggered()
{
    this->callClockAction();
}

void TBViewerMainWindow::on_authorizedChanged(bool auth)
{
    qDebug() << "TBViewerMainWindow::on_authorized:" << auth;
}
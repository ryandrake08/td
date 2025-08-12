#include "TBViewerMainWindow.hpp"
#include "TBConnectToDialog.hpp"
#include "TBPlayersModel.hpp"
#include "TBRuntimeError.hpp"
#include "TournamentService.hpp"
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
    QObject::connect(&this->getSession(), SIGNAL(connectedChanged(bool)), this, SLOT(on_connectedChanged(bool)));

    // initialize connection state
    on_connectedChanged(false); // start with disconnected state
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

void TBViewerMainWindow::on_actionConnectToTournament_triggered()
{
    TBConnectToDialog dialog(this);
    dialog.set_host("localhost"); // default host
    dialog.set_port(TournamentService::default_port);
    
    if(dialog.exec() == QDialog::Accepted)
    {
        QString host = dialog.host();
        int port = dialog.port();
        
        qDebug() << "Connecting to tournament at" << host << ":" << port;
        
        // create tournament service and connect
        TournamentService service(host.toStdString(), port);
        this->getSession().connect(service);
    }
}

void TBViewerMainWindow::on_actionDisconnect_triggered()
{
    qDebug() << "Disconnecting from tournament";
    this->getSession().disconnect();
}

void TBViewerMainWindow::on_authorizedChanged(bool auth)
{
    qDebug() << "TBViewerMainWindow::on_authorized:" << auth;
}

void TBViewerMainWindow::on_connectedChanged(bool connected)
{
    qDebug() << "TBViewerMainWindow::on_connected:" << connected;
    
    // update menu state
    this->pimpl->ui.actionConnectToTournament->setEnabled(!connected);
    this->pimpl->ui.actionDisconnect->setEnabled(connected);
}
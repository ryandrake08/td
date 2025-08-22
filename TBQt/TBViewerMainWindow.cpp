#include "TBViewerMainWindow.hpp"
#include "TBConnectToDialog.hpp"
#include "TBRuntimeError.hpp"
#include "TBSeatingChartWindow.hpp"
#include "TBTournamentDisplayWindow.hpp"
#include "TournamentService.hpp"
#include "TournamentSession.hpp"

#include "ui_TBViewerMainWindow.h"

#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QString>
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QApplication>

struct TBViewerMainWindow::impl
{
    // moc ui
    Ui::TBViewerMainWindow ui;

    // seating chart window - will be created when needed to pass session reference
    TBSeatingChartWindow* seatingChartWindow;

    // Tournament display window - will be created when needed
    TBTournamentDisplayWindow* displayWindow;

    impl(TournamentSession& session) : seatingChartWindow(nullptr), displayWindow(nullptr) {
        Q_UNUSED(session) // Will be used for window initialization
    }
};

TBViewerMainWindow::TBViewerMainWindow() : TBBaseMainWindow(), pimpl(new impl(this->getSession()))
{
    // set up moc ui
    pimpl->ui.setupUi(this);
    
    // Initialize seating chart window with session reference
    pimpl->seatingChartWindow = new TBSeatingChartWindow(this->getSession(), this);

    // Note: Tournament display window will be created on-demand when menu action is triggered
    // No central widget needed - this is now a dummy window with menu only

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
    message.setInformativeText(QObject::tr("Version %1").arg(QApplication::applicationVersion()));
    message.exec();
}

void TBViewerMainWindow::on_actionExit_triggered()
{
    // close window
    this->close();
}

void TBViewerMainWindow::on_actionPauseResume_triggered()
{
    // Delegate to session directly
    this->getSession().toggle_pause_game();
}

void TBViewerMainWindow::on_actionPreviousRound_triggered()
{
    // Delegate to session directly
    this->getSession().set_previous_level();
}

void TBViewerMainWindow::on_actionNextRound_triggered()
{
    // Delegate to session directly
    this->getSession().set_next_level();
}

void TBViewerMainWindow::on_actionCallClock_triggered()
{
    // Delegate to session directly
    this->getSession().set_action_clock(60000); // 60 seconds default
}

void TBViewerMainWindow::on_actionShowSeatingChart_triggered()
{
    // Show the seating chart window
    pimpl->seatingChartWindow->show();
    pimpl->seatingChartWindow->raise();
    pimpl->seatingChartWindow->activateWindow();
}

void TBViewerMainWindow::on_actionShowHideMainDisplay_triggered()
{
    // Create display window if it doesn't exist
    if (!pimpl->displayWindow) {
        pimpl->displayWindow = new TBTournamentDisplayWindow(this->getSession(), this);
        pimpl->displayWindow->setAttribute(Qt::WA_DeleteOnClose, false);
    }
    
    // Toggle visibility
    if (pimpl->displayWindow->isVisible()) {
        pimpl->displayWindow->hide();
    } else {
        pimpl->displayWindow->show();
        pimpl->displayWindow->raise();
        pimpl->displayWindow->activateWindow();
    }
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

    // update menu state using UI actions
    pimpl->ui.actionConnectToTournament->setEnabled(!connected);
    pimpl->ui.actionDisconnect->setEnabled(connected);
}

// Tournament display is now handled by TBTournamentDisplayWidget


void TBViewerMainWindow::connectToTournament(const TournamentService& service)
{
    // Connect this viewer window's session to the tournament service
    this->getSession().connect(service);
}
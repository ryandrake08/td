#include "TBViewerMainWindow.hpp"
#include "TBChipDisplayDelegate.hpp"
#include "TBConnectToDialog.hpp"
#include "TBPlayersModel.hpp"
#include "TBResultsModel.hpp"
#include "TBRuntimeError.hpp"
#include "TBVariantListTableModel.hpp"
#include "TBViewerMainWindow.hpp"
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

struct TBViewerMainWindow::impl
{
    // moc ui
    Ui::TBViewerMainWindow ui;
};

TBViewerMainWindow::TBViewerMainWindow() : TBBaseMainWindow(), pimpl(new impl)
{
    // set up moc
    this->pimpl->ui.setupUi(this);

    // set up models for table views
    auto chipsModel = new TBVariantListTableModel(this);
    chipsModel->addHeader("color", tr("Color"));
    chipsModel->addHeader("denomination", tr("Denomination"));
    this->pimpl->ui.chipsTableView->setModel(chipsModel);

    // set custom delegate for chip color display with ellipses
    auto chipDelegate = new TBChipDisplayDelegate(this);
    this->pimpl->ui.chipsTableView->setItemDelegate(chipDelegate);

    // configure column sizing for chips view
    QHeaderView* chipsHeader = this->pimpl->ui.chipsTableView->horizontalHeader();
    chipsHeader->setStretchLastSection(false); // Don't auto-stretch last column
    chipsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Color: fit content
    chipsHeader->setSectionResizeMode(1, QHeaderView::Stretch);          // Denomination: stretch to fill

    // right pane: results model (finished players with place/name/payout)
    auto resultsModel = new TBResultsModel(this->getSession(), this);
    this->pimpl->ui.resultsTableView->setModel(resultsModel);

    // configure column sizing for results view
    QHeaderView* resultsHeader = this->pimpl->ui.resultsTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false); // Don't auto-stretch last column
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Place: fit content
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);          // Player Name: stretch to fill
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Payout: fit content

    // connect button signals to action slots
    QObject::connect(this->pimpl->ui.previousRoundButton, SIGNAL(clicked()), this, SLOT(on_actionPreviousRound_triggered()));
    QObject::connect(this->pimpl->ui.pauseResumeButton, SIGNAL(clicked()), this, SLOT(on_actionPauseResume_triggered()));
    QObject::connect(this->pimpl->ui.nextRoundButton, SIGNAL(clicked()), this, SLOT(on_actionNextRound_triggered()));
    QObject::connect(this->pimpl->ui.callClockButton, SIGNAL(clicked()), this, SLOT(on_actionCallClock_triggered()));

    // hook up TournamentSession signals
    QObject::connect(&this->getSession(), SIGNAL(authorizedChanged(bool)), this, SLOT(on_authorizedChanged(bool)));
    QObject::connect(&this->getSession(), SIGNAL(connectedChanged(bool)), this, SLOT(on_connectedChanged(bool)));
    QObject::connect(&this->getSession(), SIGNAL(stateChanged(const QString&, const QVariant&)), this, SLOT(on_tournamentStateChanged(const QString&, const QVariant&)));

    // initialize connection state
    on_connectedChanged(false); // start with disconnected state

    // initialize UI with default values
    updateTournamentDisplay();
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
    this->getSession().toggle_pause_game();
}

void TBViewerMainWindow::on_actionPreviousRound_triggered()
{
    this->getSession().set_previous_level();
}

void TBViewerMainWindow::on_actionNextRound_triggered()
{
    this->getSession().set_next_level();
}

void TBViewerMainWindow::on_actionCallClock_triggered()
{
    this->getSession().set_action_clock(60000); // 60 seconds default
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

    // update tournament display when connection changes
    if (connected)
    {
        updateTournamentDisplay();
    }
}

void TBViewerMainWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)

    // update the display whenever tournament state changes
    updateTournamentDisplay();
}

void TBViewerMainWindow::updateTournamentDisplay()
{
    const QVariantMap& state = this->getSession().state();

    // Update tournament name and funding sources
    updateTournamentInfo(state);

    // Update tournament statistics
    updateTournamentStats(state);

    // Update tournament clock and round info
    updateTournamentClock(state);

    // Update chips and results models
    updateModels(state);
}

void TBViewerMainWindow::updateTournamentInfo(const QVariantMap& state)
{
    // Tournament name - from configuration that gets sent with state
    QString tournamentName = state.value("name", "Tournament").toString();
    this->pimpl->ui.tournamentNameLabel->setText(tournamentName);

    // Buyin information - use formatted buyin_text from derived state
    this->pimpl->ui.fundingSourcesLabel->setText(state.value("buyin_text").toString());
}

void TBViewerMainWindow::updateTournamentStats(const QVariantMap& state)
{
    // Current round - use formatted text from derived state
    this->pimpl->ui.currentRoundValue->setText(state.value("current_round_number_text").toString());

    // Players left - use formatted text from derived state
    this->pimpl->ui.playersLeftValue->setText(state.value("players_left_text").toString());

    // Total entries - use formatted text from derived state
    this->pimpl->ui.totalEntriesValue->setText(state.value("entries_text").toString());

    // Average stack - use formatted text from derived state
    this->pimpl->ui.averageStackValue->setText(state.value("average_stack_text").toString());

    // Elapsed time - use formatted text from derived state
    this->pimpl->ui.elapsedTimeValue->setText(state.value("elapsed_time_text").toString());
}

void TBViewerMainWindow::updateTournamentClock(const QVariantMap& state)
{
    // Tournament clock - use formatted text from derived state
    this->pimpl->ui.tournamentClockLabel->setText(state.value("clock_text").toString());

    // Current and next round info - use derived state formatted strings
    this->pimpl->ui.currentRoundInfoLabel->setText(state.value("current_round_text").toString());
    this->pimpl->ui.nextRoundInfoLabel->setText(state.value("next_round_text").toString());
}

void TBViewerMainWindow::updateModels(const QVariantMap& state)
{
    // Update chips model
    auto chipsModel = qobject_cast<TBVariantListTableModel*>(this->pimpl->ui.chipsTableView->model());
    if (chipsModel)
    {
        // Use available_chips from configuration state
        QVariantList chips = state.value("available_chips").toList();
        chipsModel->setListData(chips);
    }

    // Results model updates itself automatically via signal connection
}


void TBViewerMainWindow::connectToTournament(const TournamentService& service)
{
    // Connect this viewer window's session to the tournament service
    this->getSession().connect(service);
}
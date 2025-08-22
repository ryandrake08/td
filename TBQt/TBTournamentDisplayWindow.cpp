#include "TBTournamentDisplayWindow.hpp"
#include "ui_TBTournamentDisplayWindow.h"
#include "TBChipDisplayDelegate.hpp"
#include "TBPlayersModel.hpp"
#include "TBResultsModel.hpp"
#include "TBVariantListTableModel.hpp"
#include "TournamentSession.hpp"

#include <QHeaderView>
#include <QIcon>

struct TBTournamentDisplayWindow::impl
{
    // UI
    std::unique_ptr<Ui::TBTournamentDisplayWindow> ui;

    // Session reference
    TournamentSession& session;

    impl(TournamentSession& sess) : ui(new Ui::TBTournamentDisplayWindow), session(sess) {}
};

TBTournamentDisplayWindow::TBTournamentDisplayWindow(TournamentSession& session, QWidget* parent)
    : QMainWindow(parent), pimpl(new impl(session))
{
    setupUI();
    connectSignals();

    // Initial update
    updateWindowTitle();
    updateTournamentDisplay();
}

TBTournamentDisplayWindow::~TBTournamentDisplayWindow() = default;

TournamentSession& TBTournamentDisplayWindow::getSession() const
{
    return pimpl->session;
}

void TBTournamentDisplayWindow::setupUI()
{
    pimpl->ui->setupUi(this);

    // Set up chips model
    auto chipsModel = new TBVariantListTableModel(this);
    chipsModel->addHeader("color", tr("Color"));
    chipsModel->addHeader("denomination", tr("Denomination"));
    pimpl->ui->chipsTableView->setModel(chipsModel);

    // Set custom delegate for chip color display with ellipses
    auto chipDelegate = new TBChipDisplayDelegate(this);
    pimpl->ui->chipsTableView->setItemDelegate(chipDelegate);

    // Configure column sizing for chips view
    QHeaderView* chipsHeader = pimpl->ui->chipsTableView->horizontalHeader();
    chipsHeader->setStretchLastSection(false);
    chipsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    chipsHeader->setSectionResizeMode(1, QHeaderView::Stretch);

    // Set up results model
    auto resultsModel = new TBResultsModel(pimpl->session, this);
    pimpl->ui->resultsTableView->setModel(resultsModel);

    // Configure column sizing for results view
    QHeaderView* resultsHeader = pimpl->ui->resultsTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false);
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    // Set button icons if available
    pimpl->ui->previousRoundButton->setIcon(QIcon(":/Resources/b_previous_64x64.png"));
    pimpl->ui->pauseResumeButton->setIcon(QIcon(":/Resources/b_play_pause_64x64.png"));
    pimpl->ui->nextRoundButton->setIcon(QIcon(":/Resources/b_next_64x64.png"));
    pimpl->ui->callClockButton->setIcon(QIcon(":/Resources/b_call_clock_64x64.png"));
}

void TBTournamentDisplayWindow::connectSignals()
{
    // Connect to session state changes
    QObject::connect(&pimpl->session, &TournamentSession::stateChanged,
                    this, &TBTournamentDisplayWindow::on_tournamentStateChanged);

    // Connect button signals
    QObject::connect(pimpl->ui->previousRoundButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWindow::on_previousRoundButtonClicked);
    QObject::connect(pimpl->ui->pauseResumeButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWindow::on_pauseResumeButtonClicked);
    QObject::connect(pimpl->ui->nextRoundButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWindow::on_nextRoundButtonClicked);
    QObject::connect(pimpl->ui->callClockButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWindow::on_callClockButtonClicked);
}

void TBTournamentDisplayWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)

    // Update the display whenever tournament state changes
    updateTournamentDisplay();
    updateWindowTitle();
}

void TBTournamentDisplayWindow::on_previousRoundButtonClicked()
{
    pimpl->session.set_previous_level();
}

void TBTournamentDisplayWindow::on_pauseResumeButtonClicked()
{
    pimpl->session.toggle_pause_game();
}

void TBTournamentDisplayWindow::on_nextRoundButtonClicked()
{
    pimpl->session.set_next_level();
}

void TBTournamentDisplayWindow::on_callClockButtonClicked()
{
    pimpl->session.set_action_clock(60000); // 60 seconds default
}

void TBTournamentDisplayWindow::updateTournamentDisplay()
{
    const QVariantMap& state = pimpl->session.state();

    // Update tournament name and funding sources
    updateTournamentInfo(state);

    // Update tournament statistics
    updateTournamentStats(state);

    // Update tournament clock and round info
    updateTournamentClock(state);

    // Update chips and results models
    updateModels(state);
}

void TBTournamentDisplayWindow::updateTournamentInfo(const QVariantMap& state)
{
    // Tournament name - from configuration that gets sent with state
    QString tournamentName = state.value("name", "Tournament").toString();
    pimpl->ui->tournamentNameLabel->setText(tournamentName);

    // Buyin information - use formatted buyin_text from derived state
    pimpl->ui->fundingSourcesLabel->setText(state.value("buyin_text").toString());
}

void TBTournamentDisplayWindow::updateTournamentStats(const QVariantMap& state)
{
    // Current round - use formatted text from derived state
    pimpl->ui->currentRoundValue->setText(state.value("current_round_number_text").toString());

    // Players left - use formatted text from derived state
    pimpl->ui->playersLeftValue->setText(state.value("players_left_text").toString());

    // Total entries - use formatted text from derived state
    pimpl->ui->totalEntriesValue->setText(state.value("entries_text").toString());

    // Average stack - use formatted text from derived state
    pimpl->ui->averageStackValue->setText(state.value("average_stack_text").toString());

    // Elapsed time - use formatted text from derived state
    pimpl->ui->elapsedTimeValue->setText(state.value("elapsed_time_text").toString());
}

void TBTournamentDisplayWindow::updateTournamentClock(const QVariantMap& state)
{
    // Tournament clock - use formatted text from derived state
    pimpl->ui->tournamentClockLabel->setText(state.value("clock_text").toString());

    // Current and next round info - use derived state formatted strings
    pimpl->ui->currentRoundInfoLabel->setText(state.value("current_round_text").toString());
    pimpl->ui->nextRoundInfoLabel->setText(state.value("next_round_text").toString());
}

void TBTournamentDisplayWindow::updateModels(const QVariantMap& state)
{
    // Update chips model
    auto chipsModel = qobject_cast<TBVariantListTableModel*>(pimpl->ui->chipsTableView->model());
    if (chipsModel)
    {
        // Use available_chips from configuration state
        QVariantList chips = state.value("available_chips").toList();
        chipsModel->setListData(chips);
    }

    // Results model updates itself automatically via signal connection
}

void TBTournamentDisplayWindow::updateWindowTitle()
{
    const QVariantMap& state = pimpl->session.state();
    QString tournamentName = state.value("name").toString();

    if (tournamentName.isEmpty()) {
        setWindowTitle("Tournament Display");
    } else {
        setWindowTitle(QString("Tournament Display: %1").arg(tournamentName));
    }
}
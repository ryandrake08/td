#include "TBTournamentDisplayWindow.hpp"

#include "TBActionClockWindow.hpp"
#include "TBChipDisplayDelegate.hpp"
#include "TBPlayersModel.hpp"
#include "TBResultsModel.hpp"
#include "TBVariantListTableModel.hpp"

#include "TournamentSession.hpp"

#include "ui_TBTournamentDisplayWindow.h"

#include <QCloseEvent>
#include <QHeaderView>
#include <QIcon>

struct TBTournamentDisplayWindow::impl
{
    // UI
    Ui::TBTournamentDisplayWindow ui {};

    // Child windows
    TBActionClockWindow* actionClockWindow;

    explicit impl(TournamentSession& sess, TBTournamentDisplayWindow* parent) : actionClockWindow(new TBActionClockWindow(sess, parent))
    {
    }
};

TBTournamentDisplayWindow::TBTournamentDisplayWindow(TournamentSession& session, QWidget* parent) : TBBaseAuxiliaryWindow(parent), pimpl(new impl(session, this))
{
    pimpl->ui.setupUi(this);

    // Connect button signals (these are auto-connected by Qt's naming convention)
    QObject::connect(pimpl->ui.previousRoundButton, &QToolButton::clicked, this, &TBTournamentDisplayWindow::on_previousRoundButtonClicked);
    QObject::connect(pimpl->ui.pauseResumeButton, &QToolButton::clicked, this, &TBTournamentDisplayWindow::on_pauseResumeButtonClicked);
    QObject::connect(pimpl->ui.nextRoundButton, &QToolButton::clicked, this, &TBTournamentDisplayWindow::on_nextRoundButtonClicked);
    QObject::connect(pimpl->ui.callClockButton, &QToolButton::clicked, this, &TBTournamentDisplayWindow::on_callClockButtonClicked);

    // Set up chips model
    auto* chipsModel = new TBVariantListTableModel(this);
    chipsModel->addHeader("color", QObject::tr("Color"));
    chipsModel->addHeader("denomination", QObject::tr("Denomination"));
    pimpl->ui.chipsTableView->setModel(chipsModel);

    // Set custom delegate for chip color display with ellipses
    auto* chipDelegate = new TBChipDisplayDelegate(this);
    pimpl->ui.chipsTableView->setItemDelegate(chipDelegate);

    // Configure column sizing for chips view
    QHeaderView* chipsHeader = pimpl->ui.chipsTableView->horizontalHeader();
    chipsHeader->setStretchLastSection(false);
    chipsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    chipsHeader->setSectionResizeMode(1, QHeaderView::Stretch);

    // Set up results model
    auto* resultsModel = new TBResultsModel(session, this);
    pimpl->ui.resultsTableView->setModel(resultsModel);

    // Configure column sizing for results view
    QHeaderView* resultsHeader = pimpl->ui.resultsTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false);
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    // Connect to session state changes
    QObject::connect(&session, &TournamentSession::stateChanged, this, &TBTournamentDisplayWindow::on_tournamentStateChanged);

    // Connect action clock window signals
    QObject::connect(pimpl->actionClockWindow, &TBActionClockWindow::clockCanceled, this, &TBTournamentDisplayWindow::onActionClockCanceled);

    // Set QGroupBox title fonts to be large and bold
    QFont groupBoxTitleFont = this->font();
    groupBoxTitleFont.setPointSize(21);
    groupBoxTitleFont.setBold(true);

    pimpl->ui.tournamentStatsGroupBox->setFont(groupBoxTitleFont);
    pimpl->ui.chipsGroupBox->setFont(groupBoxTitleFont);
    pimpl->ui.tournamentClockGroupBox->setFont(groupBoxTitleFont);
    pimpl->ui.currentRoundGroupBox->setFont(groupBoxTitleFont);
    pimpl->ui.nextRoundGroupBox->setFont(groupBoxTitleFont);
    pimpl->ui.resultsGroupBox->setFont(groupBoxTitleFont);

    // Initial update
    this->updateFromState(session.state());
}

void TBTournamentDisplayWindow::onActionClockCanceled()
{
    Q_EMIT actionClockClearRequested();
}

TBTournamentDisplayWindow::~TBTournamentDisplayWindow() = default;

void TBTournamentDisplayWindow::updateFromState(const QVariantMap& state)
{
    this->updateTournamentName(state);
    this->updateTournamentBuyin(state);
    this->updateCurrentRoundNumber(state);
    this->updatePlayersLeft(state);
    this->updateTotalEntries(state);
    this->updateAverageStack(state);
    this->updateElapsedTime(state);
    this->updateTournamentClock(state);
    this->updateCurrentRoundInfo(state);
    this->updateNextRoundInfo(state);
    this->updateAvailableChips(state);
    this->updateBackgroundColor(state);
}

void TBTournamentDisplayWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(value)

    // Get the session that sent the signal
    auto* session = qobject_cast<TournamentSession*>(sender());
    if(!session)
    {
        return;
    }

    const QVariantMap& state = session->state();

    if(key == "name")
    {
        this->updateTournamentName(state);
    }
    else if(key == "buyin_text")
    {
        this->updateTournamentBuyin(state);
    }
    else if(key == "current_round_number_text")
    {
        this->updateCurrentRoundNumber(state);
    }
    else if(key == "players_left_text")
    {
        this->updatePlayersLeft(state);
    }
    else if(key == "entries_text")
    {
        this->updateTotalEntries(state);
    }
    else if(key == "average_stack_text")
    {
        this->updateAverageStack(state);
    }
    else if(key == "elapsed_time_text")
    {
        this->updateElapsedTime(state);
    }
    else if(key == "clock_text")
    {
        this->updateTournamentClock(state);
    }
    else if(key == "current_round_text")
    {
        this->updateCurrentRoundInfo(state);
    }
    else if(key == "next_round_text")
    {
        this->updateNextRoundInfo(state);
    }
    else if(key == "available_chips")
    {
        this->updateAvailableChips(state);
    }
    else if(key == "background_color")
    {
        this->updateBackgroundColor(state);
    }
}

void TBTournamentDisplayWindow::on_previousRoundButtonClicked()
{
    Q_EMIT previousLevelRequested();
}

void TBTournamentDisplayWindow::on_pauseResumeButtonClicked()
{
    Q_EMIT pauseToggleRequested();
}

void TBTournamentDisplayWindow::on_nextRoundButtonClicked()
{
    Q_EMIT nextLevelRequested();
}

void TBTournamentDisplayWindow::on_callClockButtonClicked()
{
    Q_EMIT actionClockStartRequested();
}

void TBTournamentDisplayWindow::updateTournamentName(const QVariantMap& state)
{
    QString tournamentName = state.value("name").toString();
    pimpl->ui.tournamentNameLabel->setText(tournamentName);

    // Window title
    if(tournamentName.isEmpty())
    {
        setWindowTitle(QObject::tr("Tournament Display"));
    }
    else
    {
        setWindowTitle(QString(QObject::tr("Tournament Display: %1")).arg(tournamentName));
    }
}

void TBTournamentDisplayWindow::updateTournamentBuyin(const QVariantMap& state)
{
    pimpl->ui.fundingSourcesLabel->setText(state.value("buyin_text").toString());
}

void TBTournamentDisplayWindow::updateCurrentRoundNumber(const QVariantMap& state)
{
    pimpl->ui.currentRoundValue->setText(state.value("current_round_number_text").toString());
}

void TBTournamentDisplayWindow::updatePlayersLeft(const QVariantMap& state)
{
    pimpl->ui.playersLeftValue->setText(state.value("players_left_text").toString());
}

void TBTournamentDisplayWindow::updateTotalEntries(const QVariantMap& state)
{
    pimpl->ui.totalEntriesValue->setText(state.value("entries_text").toString());
}

void TBTournamentDisplayWindow::updateAverageStack(const QVariantMap& state)
{
    pimpl->ui.averageStackValue->setText(state.value("average_stack_text").toString());
}

void TBTournamentDisplayWindow::updateElapsedTime(const QVariantMap& state)
{
    pimpl->ui.elapsedTimeValue->setText(state.value("elapsed_time_text").toString());
}

void TBTournamentDisplayWindow::updateTournamentClock(const QVariantMap& state)
{
    pimpl->ui.tournamentClockLabel->setText(state.value("clock_text").toString());
}

void TBTournamentDisplayWindow::updateCurrentRoundInfo(const QVariantMap& state)
{
    pimpl->ui.currentRoundInfoLabel->setText(state.value("current_round_text").toString());
}

void TBTournamentDisplayWindow::updateNextRoundInfo(const QVariantMap& state)
{
    pimpl->ui.nextRoundInfoLabel->setText(state.value("next_round_text").toString());
}

void TBTournamentDisplayWindow::updateAvailableChips(const QVariantMap& state)
{
    // Update chips model
    auto* chipsModel = qobject_cast<TBVariantListTableModel*>(pimpl->ui.chipsTableView->model());
    if(chipsModel)
    {
        // Use available_chips from configuration state
        chipsModel->setListData(state.value("available_chips").toList());
    }
}

void TBTournamentDisplayWindow::updateBackgroundColor(const QVariantMap& state)
{
    this->setBackgroundColorString(state.value("background_color").toString());
}


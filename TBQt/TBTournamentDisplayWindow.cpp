#include "TBTournamentDisplayWindow.hpp"

#include "TBActionClockWindow.hpp"
#include "TBChipDisplayDelegate.hpp"
#include "TBInvertableButton.hpp"
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
    Ui::TBTournamentDisplayWindow ui;

    // Session reference
    TournamentSession& session;

    // Child windows
    TBActionClockWindow* actionClockWindow;

    // Background theme tracking
    bool backgroundIsDark;

    explicit impl(TournamentSession& sess, TBTournamentDisplayWindow* parent) : session(sess),
                                                                                actionClockWindow(new TBActionClockWindow(sess, parent)),
                                                                                backgroundIsDark(false) {}
};

TBTournamentDisplayWindow::TBTournamentDisplayWindow(TournamentSession& session, QWidget* parent) : TBBaseAuxiliaryWindow(parent), pimpl(new impl(session, this))
{
    pimpl->ui.setupUi(this);

    // Set original icons using the setOriginalIcon method
    pimpl->ui.previousRoundButton->setOriginalIcon(QIcon(":/Resources/b_previous_64x64.png"));
    pimpl->ui.pauseResumeButton->setOriginalIcon(QIcon(":/Resources/b_play_pause_64x64.png"));
    pimpl->ui.nextRoundButton->setOriginalIcon(QIcon(":/Resources/b_next_64x64.png"));
    pimpl->ui.callClockButton->setOriginalIcon(QIcon(":/Resources/b_call_clock_64x64.png"));

    // Connect button signals (these are auto-connected by Qt's naming convention)
    QObject::connect(pimpl->ui.previousRoundButton, &QPushButton::clicked, this, &TBTournamentDisplayWindow::on_previousRoundButtonClicked);
    QObject::connect(pimpl->ui.pauseResumeButton, &QPushButton::clicked, this, &TBTournamentDisplayWindow::on_pauseResumeButtonClicked);
    QObject::connect(pimpl->ui.nextRoundButton, &QPushButton::clicked, this, &TBTournamentDisplayWindow::on_nextRoundButtonClicked);
    QObject::connect(pimpl->ui.callClockButton, &QPushButton::clicked, this, &TBTournamentDisplayWindow::on_callClockButtonClicked);

    // Connect background theme changes to button inversion
    QObject::connect(this, &TBTournamentDisplayWindow::backgroundIsDarkChanged, pimpl->ui.previousRoundButton, &TBInvertableButton::setImageInverted);
    QObject::connect(this, &TBTournamentDisplayWindow::backgroundIsDarkChanged, pimpl->ui.pauseResumeButton, &TBInvertableButton::setImageInverted);
    QObject::connect(this, &TBTournamentDisplayWindow::backgroundIsDarkChanged, pimpl->ui.nextRoundButton, &TBInvertableButton::setImageInverted);
    QObject::connect(this, &TBTournamentDisplayWindow::backgroundIsDarkChanged, pimpl->ui.callClockButton, &TBInvertableButton::setImageInverted);

    // Set up chips model
    auto chipsModel = new TBVariantListTableModel(this);
    chipsModel->addHeader("color", tr("Color"));
    chipsModel->addHeader("denomination", tr("Denomination"));
    pimpl->ui.chipsTableView->setModel(chipsModel);

    // Set custom delegate for chip color display with ellipses
    auto chipDelegate = new TBChipDisplayDelegate(this);
    pimpl->ui.chipsTableView->setItemDelegate(chipDelegate);

    // Configure column sizing for chips view
    QHeaderView* chipsHeader = pimpl->ui.chipsTableView->horizontalHeader();
    chipsHeader->setStretchLastSection(false);
    chipsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    chipsHeader->setSectionResizeMode(1, QHeaderView::Stretch);

    // Set up results model
    auto resultsModel = new TBResultsModel(pimpl->session, this);
    pimpl->ui.resultsTableView->setModel(resultsModel);

    // Configure column sizing for results view
    QHeaderView* resultsHeader = pimpl->ui.resultsTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false);
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    // Connect to session state changes
    QObject::connect(&pimpl->session, &TournamentSession::stateChanged, this, &TBTournamentDisplayWindow::on_tournamentStateChanged);

    // Connect action clock window signals
    QObject::connect(pimpl->actionClockWindow, &TBActionClockWindow::clockCanceled, this, &TBTournamentDisplayWindow::onActionClockCanceled);

    // Initial update
    this->updateTournamentName();
    this->updateTournamentBuyin();
    this->updateCurrentRoundNumber();
    this->updatePlayersLeft();
    this->updateTotalEntries();
    this->updateAverageStack();
    this->updateElapsedTime();
    this->updateTournamentClock();
    this->updateCurrentRoundInfo();
    this->updateNextRoundInfo();
    this->updateAvailableChips();
}

void TBTournamentDisplayWindow::onActionClockCanceled()
{
    pimpl->session.clear_action_clock();
}

TBTournamentDisplayWindow::~TBTournamentDisplayWindow() = default;

void TBTournamentDisplayWindow::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(value)

    if(key == "name")
    {
        this->updateTournamentName();
    }
    else if(key == "buyin_text")
    {
        this->updateTournamentBuyin();
    }
    else if(key == "current_round_number_text")
    {
        this->updateCurrentRoundNumber();
    }
    else if(key == "players_left_text")
    {
        this->updatePlayersLeft();
    }
    else if(key == "entries_text")
    {
        this->updateTotalEntries();
    }
    else if(key == "average_stack_text")
    {
        this->updateAverageStack();
    }
    else if(key == "elapsed_time_text")
    {
        this->updateElapsedTime();
    }
    else if(key == "clock_text")
    {
        this->updateTournamentClock();
    }
    else if(key == "current_round_text")
    {
        this->updateCurrentRoundInfo();
    }
    else if(key == "next_round_text")
    {
        this->updateNextRoundInfo();
    }
    else if(key == "available_chips")
    {
        this->updateAvailableChips();
    }
}

void TBTournamentDisplayWindow::on_previousRoundButtonClicked()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_previous_level();
    }
}

void TBTournamentDisplayWindow::on_pauseResumeButtonClicked()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.toggle_pause_game();
    }
    else
    {
        pimpl->session.start_game();
    }
}

void TBTournamentDisplayWindow::on_nextRoundButtonClicked()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        pimpl->session.set_next_level();
    }
}

void TBTournamentDisplayWindow::on_callClockButtonClicked()
{
    const auto& current_blind_level(pimpl->session.state()["current_blind_level"].toInt());
    const auto& actionClockTimeRemaining(pimpl->session.state()["action_clock_time_remaining"].toInt());
    if(current_blind_level != 0 && actionClockTimeRemaining == 0)
    {
        pimpl->session.set_action_clock(TournamentSession::kActionClockRequestTime);
    }
}

void TBTournamentDisplayWindow::updateTournamentName()
{
    const QVariantMap& state = pimpl->session.state();
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

void TBTournamentDisplayWindow::updateTournamentBuyin()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.fundingSourcesLabel->setText(state.value("buyin_text").toString());
}

void TBTournamentDisplayWindow::updateCurrentRoundNumber()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.currentRoundValue->setText(state.value("current_round_number_text").toString());
}

void TBTournamentDisplayWindow::updatePlayersLeft()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.playersLeftValue->setText(state.value("players_left_text").toString());
}

void TBTournamentDisplayWindow::updateTotalEntries()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.totalEntriesValue->setText(state.value("entries_text").toString());
}

void TBTournamentDisplayWindow::updateAverageStack()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.averageStackValue->setText(state.value("average_stack_text").toString());
}

void TBTournamentDisplayWindow::updateElapsedTime()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.elapsedTimeValue->setText(state.value("elapsed_time_text").toString());
}

void TBTournamentDisplayWindow::updateTournamentClock()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.tournamentClockLabel->setText(state.value("clock_text").toString());
}

void TBTournamentDisplayWindow::updateCurrentRoundInfo()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.currentRoundInfoLabel->setText(state.value("current_round_text").toString());
}

void TBTournamentDisplayWindow::updateNextRoundInfo()
{
    const QVariantMap& state = pimpl->session.state();
    pimpl->ui.nextRoundInfoLabel->setText(state.value("next_round_text").toString());
}

void TBTournamentDisplayWindow::updateAvailableChips()
{
    // Update chips model
    auto chipsModel = qobject_cast<TBVariantListTableModel*>(pimpl->ui.chipsTableView->model());
    if(chipsModel)
    {
        const QVariantMap& state = pimpl->session.state();

        // Use available_chips from configuration state
        chipsModel->setListData(state.value("available_chips").toList());
    }
}

bool TBTournamentDisplayWindow::backgroundIsDark() const
{
    return pimpl->backgroundIsDark;
}

void TBTournamentDisplayWindow::setBackgroundIsDark(bool isDark)
{
    if(pimpl->backgroundIsDark == isDark)
    {
        return;
    }

    pimpl->backgroundIsDark = isDark;
    Q_EMIT backgroundIsDarkChanged(isDark);
}

#include "TBTournamentDisplayWidget.hpp"
#include "TBChipDisplayDelegate.hpp"
#include "TBPlayersModel.hpp"
#include "TBResultsModel.hpp"
#include "TBVariantListTableModel.hpp"
#include "TournamentSession.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QGroupBox>
#include <QFrame>
#include <QSizePolicy>
#include <QIcon>
#include <QDebug>

struct TBTournamentDisplayWidget::impl
{
    TournamentSession& session;

    // Keep only widgets that are accessed after construction
    // Labels for runtime updates
    QLabel* tournamentNameLabel;
    QLabel* fundingSourcesLabel;
    QLabel* currentRoundValue;
    QLabel* playersLeftValue;
    QLabel* totalEntriesValue;
    QLabel* averageStackValue;
    QLabel* elapsedTimeValue;
    QLabel* tournamentClockLabel;
    QLabel* currentRoundInfoLabel;
    QLabel* nextRoundInfoLabel;

    // Table views for model access
    QTableView* chipsTableView;
    QTableView* resultsTableView;

    impl(TournamentSession& sess) : session(sess) {}
};

TBTournamentDisplayWidget::TBTournamentDisplayWidget(TournamentSession& session, QWidget* parent) : QWidget(parent), pimpl(new impl(session))
{
    setupUI();

    // Connect to session state changes
    QObject::connect(&pimpl->session, &TournamentSession::stateChanged,
                    this, &TBTournamentDisplayWidget::on_tournamentStateChanged);

    // Initial update
    updateTournamentDisplay();
}

TBTournamentDisplayWidget::~TBTournamentDisplayWidget() = default;

TournamentSession& TBTournamentDisplayWidget::getSession() const
{
    return pimpl->session;
}

void TBTournamentDisplayWidget::setupUI()
{
    // Main vertical layout matching the original UI structure
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    // Top pane: Tournament info (full width, fixed height)
    QFrame* tournamentInfoFrame = new QFrame();
    tournamentInfoFrame->setFrameShape(QFrame::StyledPanel);
    tournamentInfoFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    QVBoxLayout* infoLayout = new QVBoxLayout(tournamentInfoFrame);

    pimpl->tournamentNameLabel = new QLabel("Tournament Name");
    QFont nameFont = pimpl->tournamentNameLabel->font();
    nameFont.setPointSize(16);
    nameFont.setBold(true);
    pimpl->tournamentNameLabel->setFont(nameFont);
    pimpl->tournamentNameLabel->setAlignment(Qt::AlignCenter);
    infoLayout->addWidget(pimpl->tournamentNameLabel);

    pimpl->fundingSourcesLabel = new QLabel("Funding Sources");
    pimpl->fundingSourcesLabel->setAlignment(Qt::AlignCenter);
    pimpl->fundingSourcesLabel->setWordWrap(true);
    infoLayout->addWidget(pimpl->fundingSourcesLabel);

    mainLayout->addWidget(tournamentInfoFrame);

    // Main content area: Horizontal layout with 1:2:1 ratio
    QHBoxLayout* contentLayout = new QHBoxLayout();

    // Left column container (ratio 1) - use local variable, managed by Qt parent-child
    auto* leftPaneWidget = new QWidget();
    leftPaneWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftPaneWidget->setProperty("horstretch", 1);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPaneWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    // Upper-left: Tournament stats
    QFrame* statsFrame = new QFrame();
    statsFrame->setFrameShape(QFrame::StyledPanel);
    statsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFormLayout* statsLayout = new QFormLayout(statsFrame);

    statsLayout->addRow(new QLabel("Current Round:"), pimpl->currentRoundValue = new QLabel(""));
    statsLayout->addRow(new QLabel("Players Left:"), pimpl->playersLeftValue = new QLabel(""));
    statsLayout->addRow(new QLabel("Total Entries:"), pimpl->totalEntriesValue = new QLabel(""));
    statsLayout->addRow(new QLabel("Average Stack:"), pimpl->averageStackValue = new QLabel(""));
    statsLayout->addRow(new QLabel("Elapsed Time:"), pimpl->elapsedTimeValue = new QLabel(""));

    leftLayout->addWidget(statsFrame, 1);

    // Lower-left: Chips table
    pimpl->chipsTableView = new QTableView();
    pimpl->chipsTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set up chips model
    auto chipsModel = new TBVariantListTableModel(this);
    chipsModel->addHeader("color", tr("Color"));
    chipsModel->addHeader("denomination", tr("Denomination"));
    pimpl->chipsTableView->setModel(chipsModel);

    // Set custom delegate for chip color display with ellipses
    auto chipDelegate = new TBChipDisplayDelegate(this);
    pimpl->chipsTableView->setItemDelegate(chipDelegate);

    // Configure column sizing for chips view
    QHeaderView* chipsHeader = pimpl->chipsTableView->horizontalHeader();
    chipsHeader->setStretchLastSection(false);
    chipsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    chipsHeader->setSectionResizeMode(1, QHeaderView::Stretch);

    leftLayout->addWidget(pimpl->chipsTableView, 1);

    // Center column container (ratio 2) - use local variable, managed by Qt parent-child
    auto* centerPaneWidget = new QWidget();
    centerPaneWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    centerPaneWidget->setProperty("horstretch", 2);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerPaneWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    // Action buttons (top of center) - use local variables, only need during construction
    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    auto* previousRoundButton = new QPushButton("Previous");
    previousRoundButton->setIcon(QIcon(":/Resources/b_previous_64x64.png"));
    QObject::connect(previousRoundButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWidget::on_previousRoundButtonClicked);
    buttonsLayout->addWidget(previousRoundButton);

    auto* pauseResumeButton = new QPushButton("Pause");
    pauseResumeButton->setIcon(QIcon(":/Resources/b_play_pause_64x64.png"));
    QObject::connect(pauseResumeButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWidget::on_pauseResumeButtonClicked);
    buttonsLayout->addWidget(pauseResumeButton);

    auto* nextRoundButton = new QPushButton("Next");
    nextRoundButton->setIcon(QIcon(":/Resources/b_next_64x64.png"));
    QObject::connect(nextRoundButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWidget::on_nextRoundButtonClicked);
    buttonsLayout->addWidget(nextRoundButton);

    auto* callClockButton = new QPushButton("Call Clock");
    callClockButton->setIcon(QIcon(":/Resources/b_call_clock_64x64.png"));
    QObject::connect(callClockButton, &QPushButton::clicked,
                    this, &TBTournamentDisplayWidget::on_callClockButtonClicked);
    buttonsLayout->addWidget(callClockButton);

    centerLayout->addLayout(buttonsLayout);

    // Tournament clock (middle of center)
    QFrame* clockFrame = new QFrame();
    clockFrame->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout* clockLayout = new QVBoxLayout(clockFrame);

    pimpl->tournamentClockLabel = new QLabel("00:00");
    QFont clockFont = pimpl->tournamentClockLabel->font();
    clockFont.setPointSize(24);
    clockFont.setBold(true);
    pimpl->tournamentClockLabel->setFont(clockFont);
    pimpl->tournamentClockLabel->setAlignment(Qt::AlignCenter);
    clockLayout->addWidget(pimpl->tournamentClockLabel);

    pimpl->currentRoundInfoLabel = new QLabel("Round 1: 25/50");
    pimpl->currentRoundInfoLabel->setAlignment(Qt::AlignCenter);
    clockLayout->addWidget(pimpl->currentRoundInfoLabel);

    pimpl->nextRoundInfoLabel = new QLabel("Next: 50/100");
    pimpl->nextRoundInfoLabel->setAlignment(Qt::AlignCenter);
    clockLayout->addWidget(pimpl->nextRoundInfoLabel);

    centerLayout->addWidget(clockFrame);

    // Spacer (bottom of center)
    centerLayout->addStretch();

    // Right column container (ratio 1) - use local variable, managed by Qt parent-child
    auto* rightPaneWidget = new QWidget();
    rightPaneWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightPaneWidget->setProperty("horstretch", 1);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPaneWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    // Results table (full height of right column)
    pimpl->resultsTableView = new QTableView();
    pimpl->resultsTableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Set up results model
    auto resultsModel = new TBResultsModel(pimpl->session, this);
    pimpl->resultsTableView->setModel(resultsModel);

    // Configure column sizing for results view
    QHeaderView* resultsHeader = pimpl->resultsTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false);
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    rightLayout->addWidget(pimpl->resultsTableView);

    // Add columns to content layout with stretch ratios (1:2:1)
    contentLayout->addWidget(leftPaneWidget, 1);
    contentLayout->addWidget(centerPaneWidget, 2);
    contentLayout->addWidget(rightPaneWidget, 1);

    mainLayout->addLayout(contentLayout);
}

void TBTournamentDisplayWidget::on_tournamentStateChanged(const QString& key, const QVariant& value)
{
    Q_UNUSED(key)
    Q_UNUSED(value)

    // Update the display whenever tournament state changes
    updateTournamentDisplay();
}

void TBTournamentDisplayWidget::on_previousRoundButtonClicked()
{
    pimpl->session.set_previous_level();
}

void TBTournamentDisplayWidget::on_pauseResumeButtonClicked()
{
    pimpl->session.toggle_pause_game();
}

void TBTournamentDisplayWidget::on_nextRoundButtonClicked()
{
    pimpl->session.set_next_level();
}

void TBTournamentDisplayWidget::on_callClockButtonClicked()
{
    pimpl->session.set_action_clock(60000); // 60 seconds default
}

void TBTournamentDisplayWidget::updateTournamentDisplay()
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

void TBTournamentDisplayWidget::updateTournamentInfo(const QVariantMap& state)
{
    // Tournament name - from configuration that gets sent with state
    QString tournamentName = state.value("name", "Tournament").toString();
    pimpl->tournamentNameLabel->setText(tournamentName);

    // Buyin information - use formatted buyin_text from derived state
    pimpl->fundingSourcesLabel->setText(state.value("buyin_text").toString());
}

void TBTournamentDisplayWidget::updateTournamentStats(const QVariantMap& state)
{
    // Current round - use formatted text from derived state
    pimpl->currentRoundValue->setText(state.value("current_round_number_text").toString());

    // Players left - use formatted text from derived state
    pimpl->playersLeftValue->setText(state.value("players_left_text").toString());

    // Total entries - use formatted text from derived state
    pimpl->totalEntriesValue->setText(state.value("entries_text").toString());

    // Average stack - use formatted text from derived state
    pimpl->averageStackValue->setText(state.value("average_stack_text").toString());

    // Elapsed time - use formatted text from derived state
    pimpl->elapsedTimeValue->setText(state.value("elapsed_time_text").toString());
}

void TBTournamentDisplayWidget::updateTournamentClock(const QVariantMap& state)
{
    // Tournament clock - use formatted text from derived state
    pimpl->tournamentClockLabel->setText(state.value("clock_text").toString());

    // Current and next round info - use derived state formatted strings
    pimpl->currentRoundInfoLabel->setText(state.value("current_round_text").toString());
    pimpl->nextRoundInfoLabel->setText(state.value("next_round_text").toString());
}

void TBTournamentDisplayWidget::updateModels(const QVariantMap& state)
{
    // Update chips model
    auto chipsModel = qobject_cast<TBVariantListTableModel*>(pimpl->chipsTableView->model());
    if (chipsModel)
    {
        // Use available_chips from configuration state
        QVariantList chips = state.value("available_chips").toList();
        chipsModel->setListData(chips);
    }

    // Results model updates itself automatically via signal connection
}
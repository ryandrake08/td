#include "TBBuddyMainWindow.hpp"
#include "TBManageButtonDelegate.hpp"
#include "TBMovementDialog.hpp"
#include "TBPlayersModel.hpp"
#include "TBResultsModel.hpp"
#include "TBRuntimeError.hpp"
#include "TBSeatingChartWindow.hpp"
#include "TBSeatingModel.hpp"
#include "TBSetupDialog.hpp"
#include "TBTournamentDisplayWindow.hpp"

#include "TournamentDaemon.hpp"
#include "TournamentDocument.hpp"
#include "TournamentSession.hpp"

#include "ui_TBBuddyMainWindow.h"

#include <QAction>
#include <QCursor>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

struct TBBuddyMainWindow::impl
{
    // moc ui
    Ui::TBBuddyMainWindow ui;

    // tournamentd thread (full client runs its own daemon)
    TournamentDaemon server;

    // tournament document
    TournamentDocument doc;

    // current filename for window title
    QString currentFilename;
};

TBBuddyMainWindow::TBBuddyMainWindow() : TBBaseMainWindow(), pimpl(new impl())
{
    // set up moc
    this->pimpl->ui.setupUi(this);

    // left pane: players model (all players with seat/unseat checkboxes)
    auto playersModel(new TBPlayersModel(this->getSession(), this));
    this->pimpl->ui.leftTableView->setModel(playersModel);

    // configure column sizing for players view
    QHeaderView* playersHeader = this->pimpl->ui.leftTableView->horizontalHeader();
    playersHeader->setStretchLastSection(false); // Don't auto-stretch last column
    playersHeader->setSectionResizeMode(0, QHeaderView::Stretch);        // Player Name: stretch to fill
    playersHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Seated: fit content

    // connect player model signals to session
    QObject::connect(playersModel, &TBPlayersModel::seatPlayerRequested, &this->getSession(), &TournamentSession::seat_player);
    QObject::connect(playersModel, &TBPlayersModel::unseatPlayerRequested, &this->getSession(), &TournamentSession::unseat_player);

    // center pane: seating model (seated players with table/seat/paid/manage)
    auto seatingModel(new TBSeatingModel(this->getSession(), this));
    this->pimpl->ui.centerTableView->setModel(seatingModel);

    // set up button delegate for the "Manage" column (column 4)
    auto manageDelegate(new TBManageButtonDelegate(this));
    this->pimpl->ui.centerTableView->setItemDelegateForColumn(4, manageDelegate);

    // connect manage button delegate signal to context menu handler
    QObject::connect(manageDelegate, &TBManageButtonDelegate::buttonClicked, this, &TBBuddyMainWindow::on_manageButtonClicked);

    // configure column sizing for seating view
    QHeaderView* seatingHeader = this->pimpl->ui.centerTableView->horizontalHeader();
    seatingHeader->setStretchLastSection(false); // Don't auto-stretch last column
    seatingHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Table: fit content
    seatingHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Seat: fit content
    seatingHeader->setSectionResizeMode(2, QHeaderView::Stretch);          // Player Name: stretch to fill
    seatingHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Paid: fit content
    seatingHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Manage: fit content

    // connect seating model signals to session methods
    QObject::connect(seatingModel, &TBSeatingModel::fundPlayerRequested, &this->getSession(), &TournamentSession::fund_player);
    QObject::connect(seatingModel, &TBSeatingModel::bustPlayerRequested, &this->getSession(), &TournamentSession::bust_player);
    QObject::connect(seatingModel, &TBSeatingModel::unseatPlayerRequested, &this->getSession(), &TournamentSession::unseat_player);

    // right pane: results model (finished players with place/name/payout)
    auto resultsModel(new TBResultsModel(this->getSession(), this));
    this->pimpl->ui.rightTableView->setModel(resultsModel);

    // configure column sizing for results view
    QHeaderView* resultsHeader = this->pimpl->ui.rightTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false); // Don't auto-stretch last column
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Place: fit content
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);          // Player Name: stretch to fill
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Payout: fit content

    // hook up TournamentDocument signals
    QObject::connect(&this->pimpl->doc, SIGNAL(filenameChanged(const QString&)), this, SLOT(on_filenameChanged(const QString&)));

    // hook up TournamentSession signals
    QObject::connect(&this->getSession(), SIGNAL(authorizedChanged(bool)), this, SLOT(on_authorizedChanged(bool)));
    QObject::connect(&this->getSession(), SIGNAL(stateChanged(const QString&, const QVariant&)), this, SLOT(on_tournamentStateChanged(const QString&, const QVariant&)));

    // start tournament thread (full client runs its own daemon)
    auto service(this->pimpl->server.start(TournamentSession::client_identifier()));

    // connect to service
    this->getSession().connect(service);

    // set initial window title
    this->updateWindowTitle();

    // initialize display menu text
    this->updateDisplayMenuText();
    this->updateSeatingChartMenuText();
}

TBBuddyMainWindow::~TBBuddyMainWindow() = default;

// load a document to be managed by this window
bool TBBuddyMainWindow::load_document(const QString& filename)
{
    try
    {
        qDebug() << "loading document:" << filename;
        return this->pimpl->doc.load(filename);
    }
    catch(const TBRuntimeError& e)
    {
        QMessageBox::warning(this, tr("Open File"), e.q_what());
        return false;
    }
}

// closeEvent is now handled by base class

void TBBuddyMainWindow::on_actionAbout_Poker_Buddy_triggered()
{
    // show about box
    QMessageBox message(this);
    message.setIconPixmap(QPixmap(":/Resources/icon_64x64.png"));
    message.setWindowTitle(QObject::tr("About %1...").arg(QCoreApplication::applicationName()));
    message.setText(QCoreApplication::applicationName());
    message.setInformativeText(QObject::tr("Version %1").arg(QCoreApplication::applicationVersion()));
    message.exec();
}

void TBBuddyMainWindow::on_actionNew_triggered()
{
    // open a window with no document
    auto window(new TBBuddyMainWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void TBBuddyMainWindow::on_actionOpen_triggered()
{
    // display file picker and open a window for each selected file
    QFileDialog picker(this);
    picker.setAcceptMode(QFileDialog::AcceptOpen);
    picker.setFileMode(QFileDialog::ExistingFiles);
    picker.setNameFilter(QObject::tr("%1 Files (*.pokerbuddy)").arg(QCoreApplication::applicationName()));
    picker.setViewMode(QFileDialog::Detail);
    if(picker.exec())
    {
        for(const auto& filename : picker.selectedFiles())
        {
            auto window(new TBBuddyMainWindow);
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->load_document(filename);
            window->show();
        }
    }
}

void TBBuddyMainWindow::on_actionClose_triggered()
{
    // close window
    this->close();
}

void TBBuddyMainWindow::on_actionSave_triggered()
{
    // save file
    this->pimpl->doc.save();
}

void TBBuddyMainWindow::on_actionSaveAs_triggered()
{
    // display file picker and save to selected file
    QFileDialog picker(this);
    picker.setAcceptMode(QFileDialog::AcceptSave);
    picker.setFileMode(QFileDialog::AnyFile);
    picker.setNameFilter(QObject::tr("%1 Files (*.pokerbuddy)").arg(QCoreApplication::applicationName()));
    picker.setViewMode(QFileDialog::Detail);
    if(picker.exec())
    {
        for(const auto& filename : picker.selectedFiles())
        {
            this->pimpl->doc.save_as(filename);
        }
    }
}

void TBBuddyMainWindow::on_actionQuickStart_triggered()
{
    const auto& seats(this->getSession().state()["seats"].toList());
    const auto& buyins(this->getSession().state()["buyins"].toList());

    if(seats.size() > 0 || buyins.size() > 0)
    {
        // alert because this is a very destructive action
        QMessageBox message(this);
        message.setIconPixmap(QPixmap(":/Resources/icon_64x64.png"));
        message.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        message.setText(QObject::tr("Quick Start"));

        // display a different message if the game is running
        const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
        if(current_blind_level != 0)
        {
            message.setInformativeText(QObject::tr("Quick Start will end the current tournament immediately, then re-seat and buy in all players."));
        }
        else
        {
            message.setInformativeText(QObject::tr("Quick Start will clear any existing seats and buy-ins, then re-seat and buy in all players."));
        }

        // present and only perform setup if confirmed by user
        if(message.exec() == QMessageBox::Ok)
        {
            this->getSession().quick_setup();
        }
    }
    else
    {
        this->getSession().quick_setup();
    }
}

void TBBuddyMainWindow::on_actionReset_triggered()
{
    const auto& seats(this->getSession().state()["seats"].toList());
    const auto& buyins(this->getSession().state()["buyins"].toList());

    if(seats.size() > 0 || buyins.size() > 0)
    {
        // alert because this is a very destructive action
        QMessageBox message(this);
        message.setIconPixmap(QPixmap(":/Resources/icon_64x64.png"));
        message.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        message.setText(QObject::tr("Reset tournament"));

        // display a different message if the game is running
        const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
        if(current_blind_level != 0)
        {
            message.setInformativeText(QObject::tr("This will end the current tournament immediately, then clear any existing seats and buy-ins."));
        }
        else
        {
            message.setInformativeText(QObject::tr("This will clear any existing seats and buy-ins."));
        }

        // present and only perform setup if confirmed by user
        if(message.exec() == QMessageBox::Ok)
        {
            this->getSession().reset_state();
        }
    }
    else
    {
        this->getSession().reset_state();
    }
}

void TBBuddyMainWindow::on_actionConfigure_triggered()
{
    // Create and show the setup dialog
    TBSetupDialog dialog(this);

    // Set current configuration from tournament document
    dialog.setConfiguration(pimpl->doc.configuration());

    if (dialog.exec() == QDialog::Accepted)
    {
        // Get the updated configuration and apply it to the tournament document
        QVariantMap newConfiguration = dialog.configuration();
        pimpl->doc.setConfiguration(newConfiguration);
    }
}

void TBBuddyMainWindow::on_actionAuthorize_triggered()
{
    // Create a simple authorization dialog
    bool ok;
    QString codeText = QInputDialog::getText(this, tr("Authorize Remote Device"),
                                            tr("Enter 5-digit authorization code:"), QLineEdit::Normal,
                                            "", &ok);

    if (ok && !codeText.isEmpty())
    {
        // Parse the code as an integer
        bool valid;
        int authCode = codeText.toInt(&valid);

        if (valid && authCode >= 10000 && authCode <= 99999)
        {
            // TODO: Add authorized client to configuration
            // Need to implement configuration management similar to TBMacDocument
            // The client should be added to the "authorized_clients" array in configuration
            // and then configure() should be called to update the session

            QMessageBox::information(this, tr("Authorize Remote Device"),
                                    tr("Authorization functionality requires configuration management.\n\n"
                                    "This will be implemented when the Setup Tournament dialog is added."));
        }
        else
        {
            QMessageBox::warning(this, tr("Invalid Code"),
                                tr("Authorization code must be a 5-digit number."));
        }
    }
}

void TBBuddyMainWindow::on_actionPlan_triggered()
{
    // Get current number of players from configuration
    const QVariantMap& config = this->getSession().state();
    QVariantList players = config["players"].toList();
    int defaultPlayerCount = players.size();

    // Create dialog to get number of players to plan for
    bool ok;
    int playerCount = QInputDialog::getInt(this, tr("Plan Tournament"),
                                          tr("Number of players to plan seating for:"),
                                          defaultPlayerCount, 1, 10000, 1, &ok);

    if (ok)
    {
        // Plan seating for the specified number of players
        this->getSession().plan_seating_for_with_handler(playerCount, [this](const QVariantList& movements) {
            if (!movements.isEmpty())
            {
                this->showPlayerMovements(movements);
            }
            else
            {
                QMessageBox::information(this, tr("Plan Tournament"),
                                        tr("No player movements required."));
            }
        });
    }
}

void TBBuddyMainWindow::on_actionShowMoves_triggered()
{
    // Get current movements from session state and display them
    const QVariantMap& state = this->getSession().state();
    QVariantList movements = state["movements"].toList();

    if (!movements.isEmpty())
    {
        this->showPlayerMovements(movements);
    }
    else
    {
        QMessageBox::information(this, tr("Show Player Moves"),
                                tr("No player movements are currently pending."));
    }
}

void TBBuddyMainWindow::on_actionRebalance_triggered()
{
    // Rebalance seating and show movements if any are generated
    this->getSession().rebalance_seating_with_handler([this](const QVariantList& movements) {
        if (!movements.isEmpty())
        {
            this->showPlayerMovements(movements);
        }
        else
        {
            QMessageBox::information(this, tr("Rebalance Tables"),
                                    tr("Tables are already balanced. No player movements required."));
        }
    });
}


void TBBuddyMainWindow::on_actionEndGame_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().stop_game();
    }
}

void TBBuddyMainWindow::on_actionExport_triggered()
{
    QFileDialog picker(this);
    picker.setAcceptMode(QFileDialog::AcceptSave);
    picker.setFileMode(QFileDialog::AnyFile);
    picker.setNameFilter(QObject::tr("CSV Files (*.csv)"));
    picker.setViewMode(QFileDialog::Detail);
    if(picker.exec())
    {
        for(const auto& filename : picker.selectedFiles())
        {
            // get results
            auto results(this->getSession().results_as_csv());

            // create and open file
            QFile file_obj(filename);
            if(!file_obj.open(QFile::WriteOnly | QFile::Text))
            {
                // handle file open failure
                throw TBRuntimeError(QObject::tr("Cannot write file %1:\n%2.").arg(QDir::toNativeSeparators(filename), file_obj.errorString()));
            }

            // write to file
            file_obj.write(results);

            // close file
            file_obj.close();

        }
    }
}

void TBBuddyMainWindow::on_authorizedChanged(bool auth)
{
    qDebug() << "TBBuddyMainWindow::on_authorized:" << auth;

    if(auth)
    {
        qDebug() << "sending configuration:" << this->pimpl->doc.configuration().size() << "items";
        this->getSession().configure_with_handler(this->pimpl->doc.configuration(), [](const QVariantMap& config)
        {
            qDebug() << "configured:" << config.size() << "items";
        });
    }

    // Update action button states when authorization changes
    this->updateActionButtons();
}

void TBBuddyMainWindow::on_filenameChanged(const QString& filename)
{
    // store filename and update window title
    this->pimpl->currentFilename = QFileInfo(filename).fileName();
    this->updateWindowTitle();
}

void TBBuddyMainWindow::on_tournamentStateChanged(const QString& key, const QVariant& /*value*/)
{
    if(key == "name")
    {
        // update window title with tournament name
        this->updateWindowTitle();
    }
    else if(key == "running" || key == "current_time" || key == "end_of_round" ||
            key == "current_blind_level" || key == "action_clock_time_remaining")
    {
        // update tournament clock display
        this->updateTournamentClock();
        // update action button states
        this->updateActionButtons();
    }
}

void TBBuddyMainWindow::on_manageButtonClicked(const QModelIndex& index)
{
    // Get player ID directly from the model using Qt::UserRole
    QString playerId = index.data(Qt::UserRole).toString();

    if (playerId.isEmpty())
        return;

    // Get the tournament session state to find the player data
    const QVariantMap& sessionState = this->getSession().state();
    QVariantList seatedPlayers = sessionState["seated_players"].toList();

    // Find the player data by player_id (reliable lookup)
    QVariantMap playerData;
    bool found = false;

    for (const QVariant& playerVariant : seatedPlayers)
    {
        QVariantMap player = playerVariant.toMap();
        if (player["player_id"].toString() == playerId)
        {
            playerData = player;
            found = true;
            break;
        }
    }

    if (!found)
        return;

    // BUSINESS LOGIC FOR CREATING CONTEXT MENU (matching TBSeatingViewController.m)

    // Get tournament state information
    int currentBlindLevel = sessionState["current_blind_level"].toInt();
    bool playerHasBuyin = playerData["buyin"].toBool();
    QVariantList uniqueEntries = sessionState["unique_entries"].toList();
    QVariantList fundingSources = sessionState["funding_sources"].toList();

    // Build QMenu
    QMenu contextMenu(this);

    // BUSINESS LOGIC FOR FUNDING SOURCES (matching TBSeatingViewController.m)

    // Add funding source menu items
    bool hasFundingSources = false;
    for (int idx = 0; idx < fundingSources.size(); ++idx)
    {
        QVariantMap source = fundingSources[idx].toMap();
        QString sourceName = source["name"].toString();
        int sourceType = source["type"].toInt();
        QVariant forbidAfterLevel = source["forbid_after_blind_level"];

        // Check if this funding source is still allowed (not past forbid_after_blind_level)
        bool allowed = true;
        if (!forbidAfterLevel.isNull())
        {
            int forbidLevel = forbidAfterLevel.toInt();
            if (currentBlindLevel > forbidLevel)
                allowed = false;
        }

        if (!allowed)
            continue;

        // Determine if this funding source should be enabled based on business rules
        bool enabled = false;

        if (TournamentSession::toFundingType(sourceType) == TournamentSession::FundingType::Buyin)
        {
            // Buyins can happen at any time before forbid_after_blind_level, for any non-playing player
            if (!playerHasBuyin)
                enabled = true;
        }
        else if (TournamentSession::toFundingType(sourceType) == TournamentSession::FundingType::Rebuy)
        {
            // Rebuys can happen after round 0, before forbid_after_blind_level, for any player that has bought in at least once
            if (currentBlindLevel > 0 && uniqueEntries.contains(playerId))
                enabled = true;
        }
        else // Addon (FundingType::Addon or any other type)
        {
            // Addons can happen at any time before forbid_after_blind_level, for any playing player
            if (playerHasBuyin)
                enabled = true;
        }

        // Create menu action for this funding source
        QAction* action = contextMenu.addAction(sourceName);
        action->setEnabled(enabled);

        if (enabled)
        {
            QObject::connect(action, &QAction::triggered, this, [this, playerId, idx]() {
                this->getSession().fund_player(playerId, idx);
            });
        }

        hasFundingSources = true;
    }

    // Add separator if we have funding sources
    if (hasFundingSources)
    {
        contextMenu.addSeparator();
    }

    // BUSINESS LOGIC FOR BUST AND UNSEAT (matching TBSeatingViewController.m)

    // Bust Player: enabled if game is running (current_blind_level > 0) AND player has bought in
    QAction* bustAction = contextMenu.addAction(tr("Bust Player"));
    bool bustEnabled = (currentBlindLevel > 0) && playerHasBuyin;
    bustAction->setEnabled(bustEnabled);

    if (bustEnabled)
    {
        QObject::connect(bustAction, &QAction::triggered, this, [this, playerId]() {
            this->getSession().bust_player(playerId);
        });
    }

    // Unseat Player: enabled if player has NOT bought in
    QAction* unseatAction = contextMenu.addAction(tr("Unseat Player"));
    bool unseatEnabled = !playerHasBuyin;
    unseatAction->setEnabled(unseatEnabled);

    if (unseatEnabled)
    {
        QObject::connect(unseatAction, &QAction::triggered, this, [this, playerId]() {
            this->getSession().unseat_player(playerId);
        });
    }

    // Show context menu at the mouse cursor position
    contextMenu.exec(QCursor::pos());
}

void TBBuddyMainWindow::updateTournamentClock()
{
    const QVariantMap& state = this->getSession().state();

    bool running = state["running"].toBool();
    int currentBlindLevel = state["current_blind_level"].toInt();
    qint64 currentTime = state["current_time"].toLongLong();
    qint64 endOfRound = state["end_of_round"].toLongLong();

    QString clockText;

    if(!running || currentBlindLevel == 0)
    {
        clockText = tr("PAUSED");
    }
    else if(endOfRound > currentTime)
    {
        // Show round countdown
        qint64 timeRemaining = endOfRound - currentTime;
        int seconds = (timeRemaining / 1000) % 60;
        int minutes = (timeRemaining / 60000) % 60;
        int hours = timeRemaining / 3600000;

        if(hours > 0)
        {
            clockText = QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
        }
        else
        {
            clockText = QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
        }
    }
    else
    {
        clockText = tr("BREAK");
    }

    this->pimpl->ui.actionTournamentClock->setText(clockText);
}

void TBBuddyMainWindow::updateActionButtons()
{
    const QVariantMap& state = this->getSession().state();
    bool authorized = this->getSession().is_authorized();

    bool running = state["running"].toBool();
    int currentBlindLevel = state["current_blind_level"].toInt();
    int actionClockTimeRemaining = state["action_clock_time_remaining"].toInt();

    // Enable/disable actions based on tournament state and authorization
    this->pimpl->ui.actionQuickStart->setEnabled(authorized);
    this->pimpl->ui.actionReset->setEnabled(authorized);
    this->pimpl->ui.actionConfigure->setEnabled(authorized && currentBlindLevel == 0);
    this->pimpl->ui.actionAuthorize->setEnabled(authorized);
    this->pimpl->ui.actionPlan->setEnabled(authorized);

    // Tournament control actions
    this->pimpl->ui.actionPauseResume->setEnabled(authorized);
    this->pimpl->ui.actionPreviousRound->setEnabled(authorized && currentBlindLevel > 0);
    this->pimpl->ui.actionNextRound->setEnabled(authorized && currentBlindLevel > 0);
    this->pimpl->ui.actionCallClock->setEnabled(authorized && currentBlindLevel > 0 && actionClockTimeRemaining == 0);
    this->pimpl->ui.actionEndGame->setEnabled(authorized && currentBlindLevel > 0);

    // Update pause/resume button text
    if(running)
    {
        this->pimpl->ui.actionPauseResume->setText(tr("Pause"));
        this->pimpl->ui.actionPauseResume->setIconText(tr("Pause"));
    }
    else
    {
        if(currentBlindLevel == 0)
        {
            this->pimpl->ui.actionPauseResume->setText(tr("Start Tournament"));
            this->pimpl->ui.actionPauseResume->setIconText(tr("Start"));
        }
        else
        {
            this->pimpl->ui.actionPauseResume->setText(tr("Resume"));
            this->pimpl->ui.actionPauseResume->setIconText(tr("Resume"));
        }
    }
}

void TBBuddyMainWindow::updateWindowTitle(const QString& filename)
{
    // Update current filename if provided
    if(!filename.isEmpty())
    {
        this->pimpl->currentFilename = filename;
    }

    // Get tournament name from session state
    const QVariantMap& state = this->getSession().state();
    QString tournamentName = state["name"].toString();

    // Build window title
    QString windowTitle;
    if(this->pimpl->currentFilename.isEmpty())
    {
        windowTitle = tr("Untitled");
    }
    else
    {
        windowTitle = this->pimpl->currentFilename;
    }

    // Add tournament name if available
    if(!tournamentName.isEmpty())
    {
        windowTitle += QString(" (%1)").arg(tournamentName);
    }

    this->setWindowTitle(windowTitle);
}

void TBBuddyMainWindow::updateDisplayMenuText()
{
    // Update menu text based on display window visibility
    bool isVisible = this->isDisplayWindowVisible();
    QString menuText = isVisible ? tr("Hide Main Display") : tr("Show Main Display");
    this->pimpl->ui.actionShowHideMainDisplay->setText(menuText);
}

void TBBuddyMainWindow::updateSeatingChartMenuText()
{
    // Update menu text based on seating chart window visibility
    bool isVisible = this->isSeatingChartWindowVisible();
    QString menuText = isVisible ? tr("Hide Seating Chart") : tr("Show Seating Chart");
    this->pimpl->ui.actionShowHideSeatingChart->setText(menuText);
}

void TBBuddyMainWindow::showPlayerMovements(const QVariantList& movements)
{
    TBMovementDialog dialog(this);
    dialog.setMovements(movements);
    dialog.exec();
}
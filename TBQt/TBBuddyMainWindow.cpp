#include "TBBuddyMainWindow.hpp"
#include "TBAuthCodeDialog.hpp"
#include "TBManageButtonDelegate.hpp"
#include "TBMovementDialog.hpp"
#include "TBPlayersModel.hpp"
#include "TBResultsModel.hpp"
#include "TBRuntimeError.hpp"
#include "TBSeatingChartWindow.hpp"
#include "TBSeatingCompoundSortProxyModel.hpp"
#include "TBSeatingModel.hpp"
#include "TBSettingsDialog.hpp"
#include "TBSetupDialog.hpp"
#include "TBTableViewUtils.hpp"
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
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

struct TBBuddyMainWindow::impl
{
    // moc ui
    Ui::TBBuddyMainWindow ui {};

    // tournamentd thread (full client runs its own daemon)
    TournamentDaemon server;

    // tournament document
    TournamentDocument doc;

    // current filename for window title
    QString currentFilename;

    // recent files menu
    QMenu* recentFilesMenu;
    static const int MaxRecentFiles = 10;

    // player movement tracking
    QVariantList pendingMovements;
};

TBBuddyMainWindow::TBBuddyMainWindow() : pimpl(new impl())
{
    // set up moc
    pimpl->ui.setupUi(this);

    // left pane: players model (all players with seat/unseat checkboxes)
    auto* playersModel(new TBPlayersModel(this->getSession(), this));
    (void)TBTableViewUtils::setupTableViewWithSorting(this, this->pimpl->ui.leftTableView, playersModel, -1);

    // configure column sizing for players view
    QHeaderView* playersHeader = this->pimpl->ui.leftTableView->horizontalHeader();
    playersHeader->setStretchLastSection(false);                           // Don't auto-stretch last column
    playersHeader->setSectionResizeMode(0, QHeaderView::Stretch);          // Player Name: stretch to fill
    playersHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Seated: fit content

    // connect player model signals to session
    QObject::connect(playersModel, &TBPlayersModel::seatPlayerRequested, &this->getSession(), &TournamentSession::seat_player);
    QObject::connect(playersModel, &TBPlayersModel::unseatPlayerRequested, &this->getSession(), &TournamentSession::unseat_player);

    // center pane: seating model (seated players with table/seat/paid/manage)
    auto* seatingModel(new TBSeatingModel(this->getSession(), this));

    // Set up center pane with compound sorting (table first, then seat)
    auto* seatingProxyModel = new TBSeatingCompoundSortProxyModel(this);
    seatingProxyModel->setSourceModel(seatingModel);
    this->pimpl->ui.centerTableView->setModel(seatingProxyModel);
    this->pimpl->ui.centerTableView->setSortingEnabled(true);

    // Reset to unsorted state by default
    seatingProxyModel->sort(-1);

    // set up button delegate for the "Manage" column (column 4)
    auto* manageDelegate(new TBManageButtonDelegate(this));
    this->pimpl->ui.centerTableView->setItemDelegateForColumn(4, manageDelegate);

    // connect manage button delegate signal to context menu handler
    QObject::connect(manageDelegate, &TBManageButtonDelegate::buttonClicked, this, &TBBuddyMainWindow::on_manageButtonClicked);

    // configure column sizing for seating view
    QHeaderView* seatingHeader = this->pimpl->ui.centerTableView->horizontalHeader();
    seatingHeader->setStretchLastSection(false);                           // Don't auto-stretch last column
    seatingHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Table: fit content
    seatingHeader->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Seat: fit content
    seatingHeader->setSectionResizeMode(2, QHeaderView::Stretch);          // Player Name: stretch to fill
    seatingHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Paid: fit content
    seatingHeader->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Manage: fit content

    // connect seating model signals to session methods
    QObject::connect(seatingModel, &TBSeatingModel::fundPlayerRequested, &this->getSession(), &TournamentSession::fund_player);
    QObject::connect(seatingModel, &TBSeatingModel::bustPlayerRequested, this, &TBBuddyMainWindow::on_bustPlayer);
    QObject::connect(seatingModel, &TBSeatingModel::unseatPlayerRequested, &this->getSession(), &TournamentSession::unseat_player);

    // right pane: results model (finished players with place/name/payout)
    auto* resultsModel(new TBResultsModel(this->getSession(), this));

    // Set up table view with sorting, using UserRole for numeric sorting
    auto* resultsProxyModel = TBTableViewUtils::setupTableViewWithSorting(this, this->pimpl->ui.rightTableView, resultsModel, 0, Qt::AscendingOrder);
    resultsProxyModel->setSortRole(Qt::UserRole); // Use UserRole for numeric sorting

    // configure column sizing for results view
    QHeaderView* resultsHeader = this->pimpl->ui.rightTableView->horizontalHeader();
    resultsHeader->setStretchLastSection(false);                           // Don't auto-stretch last column
    resultsHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Place: fit content
    resultsHeader->setSectionResizeMode(1, QHeaderView::Stretch);          // Player Name: stretch to fill
    resultsHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Payout: fit content

    // Create the recent files submenu
    pimpl->recentFilesMenu = new QMenu(this);
    pimpl->ui.actionOpenRecent->setMenu(pimpl->recentFilesMenu);

    // hook up TournamentDocument signals
    QObject::connect(&this->pimpl->doc, &TournamentDocument::filenameChanged, this, &TBBuddyMainWindow::on_filenameChanged);
    QObject::connect(&this->pimpl->doc, &TournamentDocument::configurationChanged, this, &TBBuddyMainWindow::on_configurationChanged);

    // hook up TournamentSession signals
    QObject::connect(&this->getSession(), &TournamentSession::authorizedChanged, this, &TBBuddyMainWindow::on_authorizedChanged);
    QObject::connect(&this->getSession(), &TournamentSession::stateChanged, this, &TBBuddyMainWindow::on_tournamentStateChanged);
    QObject::connect(&this->getSession(), &TournamentSession::playerMovementsUpdated, this, &TBBuddyMainWindow::on_playerMovementsUpdated);

    // start tournament thread (full client runs its own daemon)
    auto service(this->pimpl->server.start(TournamentSession::client_identifier()));

    // connect to service
    this->getSession().connect(service);

    // set initial window title
    this->updateWindowTitle(this->getSession().state());

    // initialize display menu text
    this->updateDisplayMenuText();
    this->updateSeatingChartMenuText();

    // Update the recent files list
    this->updateRecentFilesMenu();
}

TBBuddyMainWindow::~TBBuddyMainWindow() = default;

void TBBuddyMainWindow::updateRecentFilesMenu()
{
    QSettings settings;
    QStringList recentFiles = settings.value("recentFiles").toStringList();

    // Clear existing actions
    pimpl->recentFilesMenu->clear();

    // Add actions for each recent file
    for(int i = 0; i < recentFiles.size() && i < pimpl->MaxRecentFiles; ++i)
    {
        const QString& filePath = recentFiles.at(i);
        QFileInfo fileInfo(filePath);

        if(fileInfo.exists())
        {
            QString displayName = fileInfo.fileName();
            QAction* action = pimpl->recentFilesMenu->addAction(displayName);
            action->setData(filePath);
            action->setToolTip(filePath);
            QObject::connect(action, &QAction::triggered, this, &TBBuddyMainWindow::on_openRecentFileTriggered);
        }
    }

    // Add separator and "Clear Recent Files" action if we have recent files
    if(pimpl->recentFilesMenu->actions().size() > 0)
    {
        pimpl->recentFilesMenu->addSeparator();
        QAction* clearAction = pimpl->recentFilesMenu->addAction(QObject::tr("Clear Menu"));
        QObject::connect(clearAction, &QAction::triggered, this, &TBBuddyMainWindow::on_clearRecentFilesTriggered);
    }
    else
    {
        QAction* clearAction = pimpl->recentFilesMenu->addAction(QObject::tr("Clear Menu"));
        clearAction->setEnabled(false);
    }
}

void TBBuddyMainWindow::addRecentFile(const QString& filePath)
{
    QSettings settings;
    QStringList recentFiles = settings.value("recentFiles").toStringList();

    // Remove if already exists
    recentFiles.removeAll(filePath);

    // Add to front
    recentFiles.prepend(filePath);

    // Limit to MaxRecentFiles
    while(recentFiles.size() > pimpl->MaxRecentFiles)
    {
        recentFiles.removeLast();
    }

    // Save and update menu
    settings.setValue("recentFiles", recentFiles);
    this->updateRecentFilesMenu();
}

// load a document to be managed by this window
bool TBBuddyMainWindow::load_document(const QString& filename)
{
    try
    {
        qDebug() << "loading document:" << filename;
        bool success = this->pimpl->doc.load(filename);
        if(success)
        {
            addRecentFile(filename);
        }
        return success;
    }
    catch(const TBRuntimeError& e)
    {
        QMessageBox::warning(this, QObject::tr("Open File"), e.q_what());
        return false;
    }
}

// closeEvent is now handled by base class

void TBBuddyMainWindow::on_actionSettings_triggered()
{
    TBSettingsDialog dialog(this);
    dialog.exec();
}

void TBBuddyMainWindow::on_actionAbout_Poker_Buddy_triggered()
{
    // show about box
    QMessageBox message(this);
    message.setIconPixmap(QIcon(":/icons/i_application.svg").pixmap(64, 64));
    message.setWindowTitle(QObject::tr("About %1...").arg(QCoreApplication::applicationName()));
    message.setText(QCoreApplication::applicationName());
    message.setInformativeText(QObject::tr("Version %1").arg(QCoreApplication::applicationVersion()));
    message.exec();
}

void TBBuddyMainWindow::on_actionNew_triggered()
{
    // open a window with no document
    auto* window(new TBBuddyMainWindow);
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
            // Create new window and load the file
            auto* window = new TBBuddyMainWindow;
            window->setAttribute(Qt::WA_DeleteOnClose);
            if(window->load_document(filename))
            {
                window->show();
            }
            else
            {
                delete window;
            }
        }
    }
}

void TBBuddyMainWindow::on_openRecentFileTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(action)
    {
        QString filePath = action->data().toString();
        if(!filePath.isEmpty())
        {
            // Create new window and load the file
            auto* window = new TBBuddyMainWindow;
            window->setAttribute(Qt::WA_DeleteOnClose);
            if(window->load_document(filePath))
            {
                window->show();
            }
            else
            {
                delete window;
            }
        }
    }
}

void TBBuddyMainWindow::on_clearRecentFilesTriggered()
{
    QSettings settings;
    settings.remove("recentFiles");
    this->updateRecentFilesMenu();
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

    if(!seats.empty() || !buyins.empty())
    {
        // alert because this is a very destructive action
        const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
        QString informativeText;
        if(current_blind_level != 0)
        {
            informativeText = QObject::tr("Quick Start will end the current tournament immediately, then re-seat and buy in all players.");
        }
        else
        {
            informativeText = QObject::tr("Quick Start will clear any existing seats and buy-ins, then re-seat and buy in all players.");
        }

        // present and only perform setup if confirmed by user
        if(QMessageBox::warning(this, QObject::tr("Quick Start"), informativeText, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
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

    if(!seats.empty() || !buyins.empty())
    {
        // alert because this is a very destructive action
        const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
        QString informativeText;
        if(current_blind_level != 0)
        {
            informativeText = QObject::tr("This will end the current tournament immediately, then clear any existing seats and buy-ins.");
        }
        else
        {
            informativeText = QObject::tr("This will clear any existing seats and buy-ins.");
        }

        // present and only perform setup if confirmed by user
        if(QMessageBox::warning(this, QObject::tr("Reset tournament"), informativeText, QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
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

    if(dialog.exec() == QDialog::Accepted)
    {
        // Get the updated configuration and apply it to the tournament document
        QVariantMap newConfiguration = dialog.configuration();
        pimpl->doc.setConfiguration(newConfiguration);

        // Apply configuration changes to the session if we're authorized
        if(this->getSession().is_authorized())
        {
            this->getSession().configure(newConfiguration);
        }
    }
}

void TBBuddyMainWindow::on_actionAuthorize_triggered()
{
    // Create the specialized authorization code dialog
    TBAuthCodeDialog dialog(this);

    if(dialog.exec() == QDialog::Accepted)
    {
        int authCode = dialog.getAuthCode();
        if(authCode >= 10000 && authCode <= 99999)
        {
            // Create new authorized client entry
            QVariantMap newClient;
            newClient["code"] = authCode;
            newClient["added_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

            // Add to document (this will automatically reconfigure the session via signal)
            this->pimpl->doc.addAuthorizedClient(newClient);

            QMessageBox::information(this, QObject::tr("Remote Device Authorized"), QObject::tr("Authorization code %1 has been added to the tournament configuration.").arg(authCode));
        }
        else
        {
            QMessageBox::warning(this, QObject::tr("Invalid Code"), QObject::tr("Authorization code must be a 5-digit number."));
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
    bool ok = false;
    int playerCount = QInputDialog::getInt(this, QObject::tr("Plan Tournament"), QObject::tr("Number of players to plan seating for:"), defaultPlayerCount, 1, 10000, 1, &ok);

    if(ok)
    {
        // Plan seating for the specified number of players
        this->getSession().plan_seating_with_handler(playerCount, [this](const QVariantList& movements)
        {
            if(!movements.isEmpty())
            {
                this->showPlayerMovements(movements);
            }
            else
            {
                QMessageBox::information(this, QObject::tr("Plan Tournament"), QObject::tr("No player movements required."));
            }
        });
    }
}

void TBBuddyMainWindow::on_actionShowMoves_triggered()
{
    // Show pending player movements
    const QVariantList& movements = pimpl->pendingMovements;

    if(!movements.isEmpty())
    {
        this->showPlayerMovements(movements);
    }
    else
    {
        QMessageBox::information(this, QObject::tr("Show Player Moves"), QObject::tr("No player movements are currently pending."));
    }
}

void TBBuddyMainWindow::on_actionRebalance_triggered()
{
    // Rebalance seating and show movements if any are generated
    this->getSession().rebalance_seating_with_handler([this](const QVariantList& movements)
    {
        if(!movements.isEmpty())
        {
            this->showPlayerMovements(movements);
        }
        else
        {
            QMessageBox::information(this, QObject::tr("Rebalance Tables"), QObject::tr("Tables are already balanced. No player movements required."));
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
    this->updateActionButtons(this->getSession().state(), auth);
}

void TBBuddyMainWindow::on_filenameChanged(const QString& filename)
{
    // store filename and update window title
    this->pimpl->currentFilename = QFileInfo(filename).fileName();
    this->updateWindowTitle(this->getSession().state());

    // Add to recent files if the file exists (was saved)
    if(QFileInfo::exists(filename))
    {
        addRecentFile(filename);
    }
}

void TBBuddyMainWindow::on_configurationChanged(const QVariantMap& config)
{
    // Automatically reconfigure the session when document configuration changes
    this->getSession().configure(config);
}

void TBBuddyMainWindow::on_tournamentStateChanged(const QString& key, const QVariant& /*value*/)
{
    // Get the session that sent the signal and current state
    auto* session = qobject_cast<TournamentSession*>(sender());
    if(!session)
    {
        return;
    }

    const QVariantMap& state = session->state();

    if(key == "name")
    {
        // update window title with tournament name
        this->updateWindowTitle(state);
    }
    else if(key == "running" || key == "current_time" || key == "end_of_round" ||
            key == "current_blind_level" || key == "action_clock_time_remaining")
    {
        // update tournament clock display
        this->updateTournamentClock(state);
        // update action button states
        this->updateActionButtons(state, session->is_authorized());
    }
}

void TBBuddyMainWindow::on_manageButtonClicked(const QModelIndex& index)
{
    // Get player ID directly from the model using Qt::UserRole
    QString playerId = index.data(Qt::UserRole).toString();

    if(playerId.isEmpty())
        return;

    // Get the tournament session state to find the player data
    const QVariantMap& sessionState = this->getSession().state();
    QVariantList seatedPlayers = sessionState["seated_players"].toList();

    // Find the player data by player_id (reliable lookup)
    QVariantMap playerData;
    bool found = false;

    for(const QVariant& playerVariant : seatedPlayers)
    {
        QVariantMap player = playerVariant.toMap();
        if(player["player_id"].toString() == playerId)
        {
            playerData = player;
            found = true;
            break;
        }
    }

    if(!found)
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
    for(int idx = 0; idx < fundingSources.size(); ++idx)
    {
        QVariantMap source = fundingSources[idx].toMap();
        QString sourceName = source["name"].toString();
        int sourceType = source["type"].toInt();
        QVariant forbidAfterLevel = source["forbid_after_blind_level"];

        // Check if this funding source is still allowed (not past forbid_after_blind_level)
        bool allowed = true;
        if(!forbidAfterLevel.isNull())
        {
            int forbidLevel = forbidAfterLevel.toInt();
            if(currentBlindLevel > forbidLevel)
                allowed = false;
        }

        if(!allowed)
            continue;

        // Determine if this funding source should be enabled based on business rules
        bool enabled = false;

        if(TournamentSession::toFundingType(sourceType) == TournamentSession::FundingType::Buyin)
        {
            // Buyins can happen at any time before forbid_after_blind_level, for any non-playing player
            if(!playerHasBuyin)
                enabled = true;
        }
        else if(TournamentSession::toFundingType(sourceType) == TournamentSession::FundingType::Rebuy)
        {
            // Rebuys can happen after round 0, before forbid_after_blind_level, for any player that has bought in at least once
            if(currentBlindLevel > 0 && uniqueEntries.contains(playerId))
                enabled = true;
        }
        else // Addon (FundingType::Addon or any other type)
        {
            // Addons can happen at any time before forbid_after_blind_level, for any playing player
            if(playerHasBuyin)
                enabled = true;
        }

        // Create menu action for this funding source
        QAction* action = contextMenu.addAction(sourceName);
        action->setEnabled(enabled);

        if(enabled)
        {
            QObject::connect(action, &QAction::triggered, this, [this, playerId, idx]()
            {
                this->getSession().fund_player(playerId, idx);
            });
        }

        hasFundingSources = true;
    }

    // Add separator if we have funding sources
    if(hasFundingSources)
    {
        contextMenu.addSeparator();
    }

    // BUSINESS LOGIC FOR BUST AND UNSEAT (matching TBSeatingViewController.m)

    // Bust Player: enabled if game is running (current_blind_level > 0) AND player has bought in
    QAction* bustAction = contextMenu.addAction(QObject::tr("Bust Player"));
    bool bustEnabled = (currentBlindLevel > 0) && playerHasBuyin;
    bustAction->setEnabled(bustEnabled);

    if(bustEnabled)
    {
        QObject::connect(bustAction, &QAction::triggered, this, [this, playerId]()
        {
            this->on_bustPlayer(playerId);
        });
    }

    // Unseat Player: enabled if player has NOT bought in
    QAction* unseatAction = contextMenu.addAction(QObject::tr("Unseat Player"));
    bool unseatEnabled = !playerHasBuyin;
    unseatAction->setEnabled(unseatEnabled);

    if(unseatEnabled)
    {
        QObject::connect(unseatAction, &QAction::triggered, this, [this, playerId]()
        {
            this->getSession().unseat_player(playerId);
        });
    }

    // Show context menu at the mouse cursor position
    contextMenu.exec(QCursor::pos());
}

void TBBuddyMainWindow::updateTournamentClock(const QVariantMap& state)
{
    bool running = state["running"].toBool();
    int currentBlindLevel = state["current_blind_level"].toInt();
    qint64 currentTime = state["current_time"].toLongLong();
    qint64 endOfRound = state["end_of_round"].toLongLong();

    QString clockText;

    if(!running || currentBlindLevel == 0)
    {
        clockText = QObject::tr("PAUSED");
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
        clockText = QObject::tr("BREAK");
    }

    this->pimpl->ui.actionTournamentClock->setText(clockText);
}

void TBBuddyMainWindow::updateActionButtons(const QVariantMap& state, bool authorized)
{
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
        this->pimpl->ui.actionPauseResume->setText(QObject::tr("Pause"));
        this->pimpl->ui.actionPauseResume->setIconText(QObject::tr("Pause"));
    }
    else
    {
        if(currentBlindLevel == 0)
        {
            this->pimpl->ui.actionPauseResume->setText(QObject::tr("Start Tournament"));
            this->pimpl->ui.actionPauseResume->setIconText(QObject::tr("Start"));
        }
        else
        {
            this->pimpl->ui.actionPauseResume->setText(QObject::tr("Resume"));
            this->pimpl->ui.actionPauseResume->setIconText(QObject::tr("Resume"));
        }
    }
}

void TBBuddyMainWindow::updateWindowTitle(const QVariantMap& state, const QString& filename)
{
    // Update current filename if provided
    if(!filename.isEmpty())
    {
        this->pimpl->currentFilename = filename;
    }

    // Get tournament name from session state
    QString tournamentName = state["name"].toString();

    // Build window title
    QString windowTitle;
    if(this->pimpl->currentFilename.isEmpty())
    {
        windowTitle = QObject::tr("Untitled");
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

void TBBuddyMainWindow::updateMovementBadge()
{
    // Update the Show Moves action text with movement count badge
    int count = pimpl->pendingMovements.size();
    QString baseText = QObject::tr("Show Player Moves...");

    if(count > 0)
    {
        QString badgeText = QString("%1 (%2)").arg(baseText).arg(count);
        pimpl->ui.actionShowMoves->setText(badgeText);
    }
    else
    {
        pimpl->ui.actionShowMoves->setText(baseText);
    }
}

void TBBuddyMainWindow::showPlayerMovements(const QVariantList& movements)
{
    TBMovementDialog dialog(this);
    dialog.setMovements(movements);

    if(dialog.exec() == QDialog::Accepted)
    {
        // User clicked OK - clear pending movements
        pimpl->pendingMovements.clear();
        updateMovementBadge();
    }
}

void TBBuddyMainWindow::on_playerMovementsUpdated(const QVariantList& movements)
{
    // Add new movements to pending movements list
    for(const auto& movement : movements)
    {
        pimpl->pendingMovements.append(movement);
    }

    // Update movement badge
    updateMovementBadge();
}

void TBBuddyMainWindow::on_bustPlayer(const QString& playerId)
{
    this->getSession().bust_player_with_handler(playerId, [this](const QVariantList& movements)
    {
        if(!movements.isEmpty())
        {
            this->showPlayerMovements(movements);
        }
    });
}
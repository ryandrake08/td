#include "TBMainWindow.hpp"
#include "TBPlayersModel.hpp"
#include "TBRuntimeError.hpp"
#include "TournamentDocument.hpp"
#include "TournamentSession.hpp"
#include "TournamentDaemon.hpp"

#include "ui_TBMainWindow.h"

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QString>
#include <QWidget>

struct TBMainWindow::impl
{
    // moc ui
    Ui::TBMainWindow ui;

    // tournamentd thread (full client runs its own daemon)
    TournamentDaemon server;

    // tournament document
    TournamentDocument doc;
};

TBMainWindow::TBMainWindow() : TBBaseMainWindow(), pimpl(new impl)
{
    // set up moc
    this->pimpl->ui.setupUi(this);

    // set up left model and view
    qDebug() << "setting up the models and views";
    auto playersModel(new TBPlayersModel(this->getSession(), this));
    this->pimpl->ui.leftTableView->setModel(playersModel);

    // hook up TournamentDocument signals
    QObject::connect(&this->pimpl->doc, SIGNAL(filenameChanged(const QString&)), this, SLOT(on_filenameChanged(const QString&)));

    // hook up TournamentSession signals
    QObject::connect(&this->getSession(), SIGNAL(authorizedChanged(bool)), this, SLOT(on_authorizedChanged(bool)));

    // start tournament thread (full client runs its own daemon)
    auto service(this->pimpl->server.start(TournamentSession::client_identifier()));

    // connect to service
    this->getSession().connect(service);
}

TBMainWindow::~TBMainWindow() = default;

// load a document to be managed by this window
bool TBMainWindow::load_document(const QString& filename)
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

void TBMainWindow::on_actionAbout_Poker_Buddy_triggered()
{
    // show about box
    QMessageBox message(this);
    message.setIconPixmap(QPixmap(":/Resources/icon_64x64.png"));
    message.setWindowTitle(QObject::tr("About %1...").arg(QCoreApplication::applicationName()));
    message.setText(QCoreApplication::applicationName());
    message.setInformativeText(QObject::tr("Version %1").arg(QCoreApplication::applicationVersion()));
    message.exec();
}

void TBMainWindow::on_actionNew_triggered()
{
    // open a window with no document
    auto window(new TBMainWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void TBMainWindow::on_actionOpen_triggered()
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
            auto window(new TBMainWindow);
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->load_document(filename);
            window->show();
        }
    }
}

void TBMainWindow::on_actionClose_triggered()
{
    // close window
    this->close();
}

void TBMainWindow::on_actionSave_triggered()
{
    // save file
    this->pimpl->doc.save();
}

void TBMainWindow::on_actionSaveAs_triggered()
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

void TBMainWindow::on_actionExit_triggered()
{
    // close window
    this->close();
}

void TBMainWindow::on_actionQuickStart_triggered()
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

void TBMainWindow::on_actionReset_triggered()
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

void TBMainWindow::on_actionConfigure_triggered()
{

}

void TBMainWindow::on_actionAuthorize_triggered()
{

}

void TBMainWindow::on_actionPlan_triggered()
{

}

void TBMainWindow::on_actionShowDisplay_triggered()
{

}

void TBMainWindow::on_actionShowMoves_triggered()
{

}

void TBMainWindow::on_actionRebalance_triggered()
{

}

void TBMainWindow::on_actionPauseResume_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().toggle_pause_game();
    }
    else
    {
        this->getSession().start_game();
    }
}

void TBMainWindow::on_actionPreviousRound_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().set_previous_level();
    }
}

void TBMainWindow::on_actionNextRound_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().set_next_level();
    }
}

void TBMainWindow::on_actionCallClock_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        const auto& action_clock_time_remaining(this->getSession().state()["action_clock_time_remaining"].toInt());
        if(action_clock_time_remaining == 0)
        {
            this->getSession().set_action_clock();
        }
        else
        {
            this->getSession().clear_action_clock();
        }
    }
}

void TBMainWindow::on_actionEndGame_triggered()
{
    const auto& current_blind_level(this->getSession().state()["current_blind_level"].toInt());
    if(current_blind_level != 0)
    {
        this->getSession().stop_game();
    }
}

void TBMainWindow::on_actionExport_triggered()
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

void TBMainWindow::on_authorizedChanged(bool auth)
{
    qDebug() << "TBMainWindow::on_authorized:" << auth;

    if(auth)
    {
        qDebug() << "sending configuration:" << this->pimpl->doc.configuration().size() << "items";
        this->getSession().configure(this->pimpl->doc.configuration(), [](const QVariantMap& config)
        {
            qDebug() << "configured:" << config.size() << "items";
        });
    }
}

void TBMainWindow::on_filenameChanged(const QString& filename)
{
    // change window title
    this->setWindowTitle(QFileInfo(filename).fileName());
}

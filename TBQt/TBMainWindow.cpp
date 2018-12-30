#include "TBMainWindow.hpp"
#include "TBPlayersModel.hpp"
#include "TBRuntimeError.hpp"
#include "TournamentDocument.hpp"
#include "TournamentSession.hpp"
#include "TournamentDaemon.hpp"

#include "ui_TBMainWindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QWidget>

struct TBMainWindow::impl
{
    // moc ui
    Ui::TBMainWindow ui;

    // tournament session
    TournamentSession session;

    // tournamentd thread
    TournamentDaemon server;

    // tournament document
    TournamentDocument doc;
};

TBMainWindow::TBMainWindow() : pimpl(new impl)
{
    // set up moc
    this->pimpl->ui.setupUi(this);

    // set up rest of window
    this->setUnifiedTitleAndToolBarOnMac(true);

    // set up left model and view
    qDebug() << "setting up the models and views";
    auto playersModel(new TBPlayersModel(this->pimpl->session, this));
    this->pimpl->ui.leftTableView->setModel(playersModel);

    // hook up TournamentSession signals
    QObject::connect(&this->pimpl->session, SIGNAL(authorizedChanged(bool)), this, SLOT(on_authorizedChanged(bool)));

    // start tournament thread
    auto service(this->pimpl->server.start(TournamentSession::client_identifier()));

    // connect to service
    this->pimpl->session.connect(service);
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

void TBMainWindow::on_actionExit_triggered()
{
    this->close();
}

void TBMainWindow::on_authorizedChanged(bool auth)
{
    qDebug() << "TBMainWindow::on_authorized:" << auth;

    if(auth)
    {
        qDebug() << "sending configuration:" << this->pimpl->doc.configuration().size() << "items";
        this->pimpl->session.configure(this->pimpl->doc.configuration(), [](const QVariantMap& config)
        {
            qDebug() << "configured:" << config.size() << "items";
        });
    }
}

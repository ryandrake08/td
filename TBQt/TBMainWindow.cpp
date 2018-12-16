#include "TBMainWindow.hpp"
#include "TBRuntimeError.hpp"
#include "TournamentDocument.hpp"
#include "TournamentSession.hpp"
#include "TournamentDaemon.hpp"

#include "ui_TBMainWindow.h"

#include <QMessageBox>
#include <QString>
#include <QWidget>

struct TBMainWindow::impl
{
    // tournament session
    TournamentSession session;

    // tournamentd thread
    TournamentDaemon server;

    // tournament document
    TournamentDocument doc;

public:
    impl()
    {
        // start tournament thread
        auto service(this->server.start(TournamentSession::client_identifier()));

        // connect to service
        session.connect(service);
    }
};

TBMainWindow::TBMainWindow() : pimpl(new impl), ui(new Ui::TBMainWindow)
{
    // set up moc
    ui->setupUi(this);

    // set up rest of window
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setUnifiedTitleAndToolBarOnMac(true);
}

TBMainWindow::~TBMainWindow() = default;

// load a document to be managed by this window
bool TBMainWindow::load_document(const QString& filename)
{
    try
    {
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

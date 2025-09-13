#include "TBViewerMainWindow.hpp"

#include "TBConnectToDialog.hpp"
#include "TBRuntimeError.hpp"
#include "TBSeatingChartWindow.hpp"
#include "TBSettingsDialog.hpp"
#include "TBTournamentDisplayWindow.hpp"
#include "TournamentBrowser.hpp"

#include "TournamentService.hpp"
#include "TournamentSession.hpp"

#include "ui_TBViewerMainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <QTime>
#include <QTimer>
#include <QWidget>

struct TBViewerMainWindow::impl
{
    // moc ui
    Ui::TBViewerMainWindow ui {};

    // tournament browser for service discovery
    TournamentBrowser* browser { nullptr };

    // services menu actions (for cleanup)
    QList<QAction*> serviceActions;
};

TBViewerMainWindow::TBViewerMainWindow() : pimpl(new impl())
{
    // set up moc ui
    pimpl->ui.setupUi(this);

    // create tournament browser
    pimpl->browser = new TournamentBrowser(this);

    // hook up TournamentSession signals
    QObject::connect(&this->getSession(), &TournamentSession::authorizedChanged, this, &TBViewerMainWindow::on_authorizedChanged);
    QObject::connect(&this->getSession(), &TournamentSession::connectedChanged, this, &TBViewerMainWindow::on_connectedChanged);

    // hook up TournamentBrowser signals
    QObject::connect(pimpl->browser, &TournamentBrowser::servicesUpdated, this, &TBViewerMainWindow::on_servicesUpdated);

    // initialize connection state
    on_connectedChanged(false); // start with disconnected state

    // initialize display menu text
    this->updateDisplayMenuText();
    this->updateSeatingChartMenuText();

    // initialize service menu
    this->updateServiceMenu();
}

TBViewerMainWindow::~TBViewerMainWindow() = default;

void TBViewerMainWindow::on_actionSettings_triggered()
{
    TBSettingsDialog dialog(this);
    dialog.exec();
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

    // update menu state using UI actions
    pimpl->ui.actionConnectToTournament->setEnabled(!connected);
    pimpl->ui.actionDisconnect->setEnabled(connected);

    // automatically show display window when connected
    if(connected && !this->isDisplayWindowVisible())
    {
        // Use the same logic as the menu action to show the display window
        pimpl->ui.actionShowHideMainDisplay->trigger();
    }
}

void TBViewerMainWindow::connectToTournament(const TournamentService& service)
{
    // Connect this viewer window's session to the tournament service
    this->getSession().connect(service);
}

void TBViewerMainWindow::on_servicesUpdated(const QVariantList& services)
{
    // Update the service menu
    this->updateServiceMenu();

    // If just one service and not already connected, automatically connect (like macOS)
    bool connected = this->getSession().is_connected();
    if(services.size() == 1 && !connected)
    {
        QVariantMap serviceMap = services[0].toMap();

        // Create TournamentService from the service data and connect directly
        if(serviceMap["isRemote"].toBool())
        {
            TournamentService service(serviceMap["address"].toString().toStdString(), serviceMap["port"].toInt());
            this->connectToTournament(service);
        }
        else
        {
            TournamentService service(serviceMap["unixSocketPath"].toString().toStdString());
            this->connectToTournament(service);
        }
    }
}

void TBViewerMainWindow::updateServiceMenu()
{
    // Get the File menu
    QMenu* fileMenu = pimpl->ui.menuFile;

    // Remove existing service actions (items with tag 0, like macOS implementation)
    for(QAction* action : pimpl->serviceActions)
    {
        fileMenu->removeAction(action);
        delete action;
    }
    pimpl->serviceActions.clear();

    // Get services from browser
    QVariantList localServices = pimpl->browser->localServiceList();
    QVariantList remoteServices = pimpl->browser->remoteServiceList();

    // Find insertion point (before Exit action, after Disconnect action)
    QAction* insertBefore = pimpl->ui.actionExit;

    // Add local services section
    if(!localServices.isEmpty())
    {
        // Add separator
        auto* separator = new QAction(this);
        separator->setSeparator(true);
        pimpl->serviceActions.append(separator);
        fileMenu->insertAction(insertBefore, separator);

        // Add section header
        auto* header = new QAction(QObject::tr("On this Computer"), this);
        header->setEnabled(false);
        pimpl->serviceActions.append(header);
        fileMenu->insertAction(insertBefore, header);

        // Add local service items
        for(const QVariant& serviceVar : localServices)
        {
            QVariantMap serviceMap = serviceVar.toMap();
            auto* serviceAction = new QAction(serviceMap["name"].toString(), this);

            // Store service data in action
            serviceAction->setData(serviceVar);

            // Connect to slot that will call connectToTournament
            QObject::connect(serviceAction, &QAction::triggered, [this, serviceAction]()
            {
                QVariantMap serviceMap = serviceAction->data().toMap();
                TournamentService service(serviceMap["unixSocketPath"].toString().toStdString());
                this->connectToTournament(service);
            });

            pimpl->serviceActions.append(serviceAction);
            fileMenu->insertAction(insertBefore, serviceAction);
        }
    }

    // Add remote services section
    if(!remoteServices.isEmpty())
    {
        // Add separator
        auto* separator = new QAction(this);
        separator->setSeparator(true);
        pimpl->serviceActions.append(separator);
        fileMenu->insertAction(insertBefore, separator);

        // Add section header
        auto* header = new QAction(QObject::tr("On the Network"), this);
        header->setEnabled(false);
        pimpl->serviceActions.append(header);
        fileMenu->insertAction(insertBefore, header);

        // Add remote service items
        for(const QVariant& serviceVar : remoteServices)
        {
            QVariantMap serviceMap = serviceVar.toMap();
            auto* serviceAction = new QAction(serviceMap["name"].toString(), this);

            // Store service data in action
            serviceAction->setData(serviceVar);

            // Connect to slot that will call connectToTournament
            QObject::connect(serviceAction, &QAction::triggered, [this, serviceAction]()
            {
                QVariantMap serviceMap = serviceAction->data().toMap();
                TournamentService service(serviceMap["address"].toString().toStdString(),
                                          serviceMap["port"].toInt());
                this->connectToTournament(service);
            });

            pimpl->serviceActions.append(serviceAction);
            fileMenu->insertAction(insertBefore, serviceAction);
        }
    }
}
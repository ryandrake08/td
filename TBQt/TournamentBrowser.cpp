#include "TournamentBrowser.hpp"

#include "TournamentService.hpp"

#include <qmdnsengine/browser.h>
#include <qmdnsengine/server.h>
#include <qmdnsengine/service.h>

#include <QDir>
#include <QStandardPaths>
#include <QVector>

#include <algorithm>
#include <memory>

// Simple C++ version of TournamentSocketDirectory for Qt
static QString tournamentSocketDirectory()
{
    // Try cache directory first (equivalent to NSCachesDirectory)
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if(!cacheDir.isEmpty())
    {
        return cacheDir;
    }

    // Try temp directory
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if(!tempDir.isEmpty())
    {
        return tempDir;
    }

    // Fallback to /tmp
    return QStringLiteral("/tmp");
}

struct TournamentBrowser::impl
{
    QList<TournamentService*> services;
    QMdnsEngine::Server* mdnsServer;
    QMdnsEngine::Browser* mdnsBrowser;

    impl(QObject* parent) :
        mdnsServer(new QMdnsEngine::Server(parent)),
        mdnsBrowser(new QMdnsEngine::Browser(mdnsServer, TournamentService::type, nullptr, parent))
    {
    }

    ~impl()
    {
        qDeleteAll(services);
    }

    // no copy constructors/assignment
    impl(const impl& other) = delete;
    impl& operator=(const impl& other) = delete;

    // no move constructors/assignment
    impl(impl&& other) = delete;
    impl& operator=(impl&& other) = delete;
};

TournamentBrowser::TournamentBrowser(QObject* parent) : QObject(parent), pimpl(new impl(this))
{
    // Add local services immediately
    addLocalServices();

    // Connect browser signals
    QObject::connect(pimpl->mdnsBrowser, &QMdnsEngine::Browser::serviceAdded, this, &TournamentBrowser::on_serviceAdded);
    QObject::connect(pimpl->mdnsBrowser, &QMdnsEngine::Browser::serviceRemoved, this, &TournamentBrowser::on_serviceRemoved);
    QObject::connect(pimpl->mdnsBrowser, &QMdnsEngine::Browser::serviceUpdated, this, &TournamentBrowser::on_serviceUpdated);
}

TournamentBrowser::~TournamentBrowser() = default;

QVariantList TournamentBrowser::serviceList() const
{
    QVariantList list;
    for(const auto& service : pimpl->services)
    {
        QVariantMap map;
        map["name"] = QString::fromStdString(service->name());
        map["isRemote"] = service->is_remote();
        if(service->is_remote())
        {
            map["address"] = QString::fromStdString(service->address());
            map["port"] = service->port();
        }
        else
        {
            map["unixSocketPath"] = QString::fromStdString(service->path());
        }
        list.append(map);
    }
    return list;
}

QVariantList TournamentBrowser::localServiceList() const
{
    QVariantList list;
    for(const auto& service : pimpl->services)
    {
        if(!service->is_remote())
        {
            QVariantMap map;
            map["name"] = QString::fromStdString(service->name());
            map["isRemote"] = service->is_remote();
            map["unixSocketPath"] = QString::fromStdString(service->path());
            list.append(map);
        }
    }
    return list;
}

QVariantList TournamentBrowser::remoteServiceList() const
{
    QVariantList list;
    for(const auto& service : pimpl->services)
    {
        if(service->is_remote())
        {
            QVariantMap map;
            map["name"] = QString::fromStdString(service->name());
            map["isRemote"] = service->is_remote();
            map["address"] = QString::fromStdString(service->address());
            map["port"] = service->port();
            list.append(map);
        }
    }
    return list;
}

void TournamentBrowser::on_serviceAdded(const QMdnsEngine::Service& service)
{
    // Check if we already have this service
    for(const auto& existingService : pimpl->services)
    {
        if(*existingService == service)
        {
            return; // Already have this service
        }
    }

    // Add new remote service
    pimpl->services.append(new TournamentService(service));

    updateServiceList();
}

void TournamentBrowser::on_serviceRemoved(const QMdnsEngine::Service& service)
{
    // Find and remove the service
    for(int i = 0; i < pimpl->services.size(); ++i)
    {
        const auto& existingService = pimpl->services[i];
        if(*existingService == service)
        {
            delete existingService;
            pimpl->services.removeAt(i);
            break;
        }
    }

    updateServiceList();
}

void TournamentBrowser::on_serviceUpdated(const QMdnsEngine::Service& service)
{
    // Find and update the service
    auto it = std::find_if(pimpl->services.begin(), pimpl->services.end(),
                          [&service](const TournamentService* existingService) {
                              return *existingService == service;
                          });
    
    if(it != pimpl->services.end())
    {
        // Service details might have changed - recreate it
        delete *it;
        *it = new TournamentService(service);
    }

    updateServiceList();
}

void TournamentBrowser::addLocalServices()
{
    QString socketDir = tournamentSocketDirectory();
    QDir dir(socketDir);

    // Find all tournamentd socket files
    QStringList filters;
    filters << QStringLiteral("tournamentd.*.sock");
    QStringList socketFiles = dir.entryList(filters, QDir::Files);

    for(const QString& fileName : socketFiles)
    {
        QString fullPath = dir.absoluteFilePath(fileName);
        pimpl->services.append(new TournamentService(fullPath.toStdString()));
    }
}

void TournamentBrowser::updateServiceList()
{
    Q_EMIT servicesUpdated(serviceList());
}

#pragma once

#include <QObject>
#include <QVariantList>

#include <memory>

namespace QMdnsEngine
{
    class Server;
    class Browser;
    class Service;
}

class TournamentService;

class TournamentBrowser : public QObject
{
    Q_OBJECT

    struct impl;
    std::unique_ptr<impl> pimpl;

    // Utility
    void addLocalServices();
    void updateServiceList();

public:
    explicit TournamentBrowser(QObject* parent = nullptr);
    ~TournamentBrowser();

    // Service lists
    QVariantList serviceList() const;
    QVariantList localServiceList() const;
    QVariantList remoteServiceList() const;

Q_SIGNALS:
    // Called after services are updated
    void servicesUpdated(const QVariantList& services);

private Q_SLOTS:
    void on_serviceAdded(const QMdnsEngine::Service& service);
    void on_serviceRemoved(const QMdnsEngine::Service& service);
    void on_serviceUpdated(const QMdnsEngine::Service& service);
};
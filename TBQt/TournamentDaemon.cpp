#include "TournamentDaemon.hpp"

#include "TournamentService.hpp"

#include "../tournamentd/bonjour.hpp"
#include "../tournamentd/logger.hpp"
#include "../tournamentd/tournament.hpp"

#include <cstdlib>
#include <thread>

struct TournamentDaemon::impl
{
    // flag to control whether or not daemon is running, and mutex to synchronize access
    bool running;
    std::mutex running_mutex;

    // background thread
    std::thread thread;

    // listening port
    int port;

    // published Bonjour service
    bonjour_publisher publisher;
};

TournamentDaemon::TournamentDaemon() : pimpl(new impl)
{
}

TournamentDaemon::~TournamentDaemon()
{
    // ensure thread is stopped/joined
    this->stop();
}

// start the daemon, pre-authorizing given client code, returning service
TournamentService TournamentDaemon::start(int code)
{
    logger_enable(ll::error, ll::warning, ll::info);

    // obtain temporary directory from TMPDIR environment
    const char* tmpdir(std::getenv("TMPDIR"));
    if(tmpdir == nullptr)
    {
        tmpdir = P_tmpdir;
    }

    // set up tournament and authorize
    std::shared_ptr<tournament> tourney(new tournament);
    tourney->authorize(code);
    auto service(tourney->listen(tmpdir));

    // server is listening. mark as running and run in background
    this->pimpl->running = true;
    this->pimpl->thread = std::thread([this,tourney]()
    {
        std::unique_lock<std::mutex> lock(this->pimpl->running_mutex);
        while(this->pimpl->running)
        {
            lock.unlock();

            auto quit(tourney->run());

            lock.lock();
            this->pimpl->running = this->pimpl->running && !quit;
        }
    });

    // store the port
    this->pimpl->port = service.second;

    // return the unix socket path, for subsequent local connection
    if(service.first.empty())
    {
        return TournamentService("localhost", service.second);
    }
    else
    {
        return TournamentService(service.first);
    }
}

// publish over Bojour using name
void TournamentDaemon::publish(const std::string& name)
{
    this->pimpl->publisher.publish(name.c_str(), this->pimpl->port);
}

// stop the daemon (and stop publishing if doing so)
void TournamentDaemon::stop()
{
    // signal thread to stop running
    std::unique_lock<std::mutex> lock(this->pimpl->running_mutex);
    this->pimpl->running = false;
    lock.unlock();

    // wait for thread to quit
    if(this->pimpl->thread.joinable())
    {
        this->pimpl->thread.join();
    }
}

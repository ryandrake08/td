#include "tournament_thread.hpp"
#include "bonjour.hpp"
#include "logger.hpp"
#include "tournament_service.hpp"
#include "tournament.hpp"

#include <cstdlib>
#include <thread>

struct tournament_thread::impl
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

tournament_thread::tournament_thread() : pimpl(new impl)
{
}

tournament_thread::~tournament_thread()
{
    // ensure thread is stopped/joined
    this->stop();
}

// start the daemon, pre-authorizing given client code, returning service
tournament_service tournament_thread::start(int code)
{
#if defined(DEBUG)
    logger_enable(ll::error, ll::warning, ll::info, ll::debug);
#else
    logger_enable(ll::error, ll::warning, ll::info);
#endif

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
        return tournament_service("localhost", service.second);
    }
    else
    {
        return tournament_service(service.first);
    }
}

// publish over Bojour using name
void tournament_thread::publish(const std::string& name)
{
    this->pimpl->publisher.publish(name.c_str(), this->pimpl->port);
}

// stop the daemon (and stop publishing if doing so)
void tournament_thread::stop()
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

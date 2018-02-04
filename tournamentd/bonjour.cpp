#include "bonjour.hpp"
#include "logger.hpp"
#include <system_error>

#if defined(__APPLE__)
#include <CFNetwork/CFNetwork.h>

struct bonjour_publisher::impl
{
    class cf_error_category : public std::error_category
    {
    public:
        virtual const char* name() const throw() override
        {
            return "CFStreamError";
        }

        virtual bool equivalent (const std::error_code& code, int condition) const throw() override
        {
            return *this==code.category() && static_cast<int>(default_error_condition(code.value()).value())==condition;
        }

        virtual std::string message(int ev) const override
        {
            return std::to_string(ev);
        }
    };

    CFNetServiceRef netService;

    static void registerCallback(CFNetServiceRef /* theService */, CFStreamError* /* error */, void* /* info */)
    {
    }

public:
    impl(const char* name, int port)
    {
        logger(ll::info) << "setting up bonjour service for " << name << " with port " << port << '\n';

        // describe net service
        CFStringRef theDomain = CFSTR("local.");
        CFStringRef serviceType = CFSTR("_pokerbuddy._tcp");
        CFStringRef serviceName = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingUTF8);

        // create net service
        netService = CFNetServiceCreate(nullptr, theDomain, serviceType, serviceName, port);

        // release serviceName
        CFRelease(serviceName);

        CFNetServiceClientContext clientContext = { 0, nullptr, nullptr, nullptr, nullptr };
        CFNetServiceSetClient(netService, registerCallback, &clientContext);
        CFNetServiceScheduleWithRunLoop(netService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

        CFOptionFlags options = 0;
        CFStreamError error;
        if(CFNetServiceRegisterWithOptions(netService, options, &error) == 0)
        {
            CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
            CFNetServiceSetClient(netService, nullptr, nullptr);
            CFRelease(netService);

            throw std::system_error(error.error, cf_error_category(), "CFNetServiceRegisterWithOptions");
        }
    }

    ~impl()
    {
        logger(ll::info) << "shutting down bonjour\n";

        CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFNetServiceSetClient(netService, nullptr, nullptr);
        CFNetServiceCancel(netService);
        CFRelease(netService);
    }
};
#else

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <cassert>
#include <string>

struct bonjour_publisher::impl
{
    class avahi_error_category : public std::error_category
    {
    public:
        virtual const char* name() const throw() override
        {
            return "avahi";
        }

        virtual bool equivalent (const std::error_code& code, int condition) const throw() override
        {
            return *this==code.category() && static_cast<int>(default_error_condition(code.value()).value())==condition;
        }

        virtual std::string message(int ev) const override
        {
            return avahi_strerror(ev);
        }
    };

    class avahi_scope_lock
    {
        AvahiThreadedPoll* threaded_poll;

    public:
        explicit avahi_scope_lock(AvahiThreadedPoll* p) : threaded_poll(p)
        {
            avahi_threaded_poll_lock(this->threaded_poll);
        }
        ~avahi_scope_lock()
        {
            avahi_threaded_poll_unlock(this->threaded_poll);
        }
    };

    class avahi_poller
    {
        AvahiThreadedPoll* threaded_poll;

    public:
        // create poller
        avahi_poller() : threaded_poll(avahi_threaded_poll_new())
        {
            if(this->threaded_poll == nullptr)
            {
                throw std::system_error(AVAHI_ERR_FAILURE, avahi_error_category(), "avahi_threaded_poll_new");
            }

            logger(ll::debug) << "created poller\n";
        }

        // shut down poller
        ~avahi_poller()
        {
            if(this->threaded_poll != nullptr)
            {
                logger(ll::debug) << "freeing poller\n";

                // stop
                avahi_threaded_poll_stop(this->threaded_poll);

                // avahi bug: looks like avahi_client_free will try to free this for us, causing double-free if we free it here
                // might be fixed in avahi 0.7
                //avahi_threaded_poll_free(this->threaded_poll);
                this->threaded_poll = nullptr;
            }
        }

        // get the AvahiPoll* object
        const AvahiPoll* get() const
        {
            return avahi_threaded_poll_get(this->threaded_poll);
        }

        // start (from main thread)
        void start()
        {
            logger(ll::debug) << "starting poller\n";

            if(avahi_threaded_poll_start(this->threaded_poll) != 0)
            {
                throw std::runtime_error("could not start poller");
            }
        }

        // stop (from main thread)
        void stop()
        {
            logger(ll::debug) << "stopping poller\n";

            if(avahi_threaded_poll_stop(this->threaded_poll) != 0)
            {
                logger(ll::warning) << "stop() called on already stopped poller\n";
            }
        }

        // quit (from polling thread)
        void quit()
        {
            logger(ll::debug) << "quitting poller\n";

            avahi_threaded_poll_quit(this->threaded_poll);
        }

        // get a scope_lock object
        avahi_scope_lock scope_lock()
        {
            return avahi_scope_lock(this->threaded_poll);
        }
    };

    class avahi_group
    {
        AvahiEntryGroup* group;
        std::string service_name;
        int service_port;

        AvahiClient* client() const
        {
            return avahi_entry_group_get_client(this->group);
        }

        static std::string alternative_name(const std::string& name)
        {
            logger(ll::debug) << "generating alternative name for: " << name << "\n";

            // generate alternative name
            auto tmp(avahi_alternative_service_name(name.c_str()));
            std::string alt_name(tmp);
            avahi_free(tmp);

            logger(ll::debug) << "generated alternative name: " << alt_name << "\n";
            return alt_name;
        }

        void callback(AvahiEntryGroup* /* g */, AvahiEntryGroupState state)
        {
            // called whenever the entry group state changes
            switch (state)
            {
                case AVAHI_ENTRY_GROUP_ESTABLISHED :
                    /* The entry group has been established successfully */
                    logger(ll::debug) << "AVAHI_ENTRY_GROUP_ESTABLISHED\n";
                    break;

                case AVAHI_ENTRY_GROUP_COLLISION :
                {
                    /* A service name collision with a remote service
                     * happened. Let's pick a new name */
                    logger(ll::debug) << "AVAHI_ENTRY_GROUP_COLLISION\n";
                    this->add_service   (avahi_group::alternative_name(this->service_name), this->service_port);
                    break;

                }
                case AVAHI_ENTRY_GROUP_FAILURE :
                    logger(ll::debug) << "AVAHI_ENTRY_GROUP_FAILURE\n";
                    throw std::system_error(avahi_client_errno(this->client()), avahi_error_category(), "avahi_group callback");
                    break;

                case AVAHI_ENTRY_GROUP_UNCOMMITED:
                    logger(ll::debug) << "AVAHI_ENTRY_GROUP_UNCOMMITED\n";
                    break;

                case AVAHI_ENTRY_GROUP_REGISTERING:
                    logger(ll::debug) << "AVAHI_ENTRY_GROUP_REGISTERING\n";
                    break;
            }
        }

        // static handler
        static void static_callback(AvahiEntryGroup* g, AvahiEntryGroupState state, void* userdata)
        {
            assert(userdata != nullptr);
            reinterpret_cast<bonjour_publisher::impl::avahi_group*>(userdata)->callback(g, state);
        }

        void add_service(const std::string& name, int port)
        {
            if(avahi_entry_group_is_empty(this->group))
            {
                logger(ll::info) << "adding avahi service: " << name << ", port: " << port << '\n';

                // get client if we need it
                auto client(avahi_entry_group_get_client(this->group));

                int ret(0);
                ret = avahi_entry_group_add_service(this->group,
                                                    AVAHI_IF_UNSPEC,
                                                    AVAHI_PROTO_UNSPEC,
                                                    static_cast<AvahiPublishFlags>(0),
                                                    name.c_str(),
                                                    "_pokerbuddy._tcp",
                                                    "local.",
                                                    nullptr,
                                                    port,
                                                    nullptr);
                if(ret == AVAHI_ERR_COLLISION)
                {
                    // handle collision
                    logger(ll::warning) << "avahi_entry_group_add_service returned AVAHI_ERR_COLLISION\n";

                    // reset
                    this->reset();

                    // try re-publishing
                    this->add_service(avahi_group::alternative_name(name), port);
                }
                else if(ret != 0)
                {
                    throw std::system_error(avahi_client_errno(client), avahi_error_category(), "avahi_entry_group_add_service");
                }
                else
                {
                    // commit
                    ret = avahi_entry_group_commit(this->group);
                    if(ret != 0)
                    {
                        throw std::system_error(avahi_client_errno(client), avahi_error_category(), "avahi_entry_group_add_service");
                    }
                }
            }
        }

    public:
        avahi_group(const char* name, int port) : group(nullptr), service_name(name), service_port(port)
        {
            logger(ll::debug) << "constructing entry group object\n";
        }

        ~avahi_group()
        {
            if(this->group != nullptr)
            {
                logger(ll::debug) << "freeing entry group\n";

                avahi_entry_group_free(this->group);
                this->group = nullptr;
            }
        }

        // initialize once we have a client
        void init(AvahiClient* c)
        {
            if(this->group == nullptr)
            {
                logger(ll::debug) << "creating entry group\n";

                this->group = avahi_entry_group_new(c, static_callback, this);
                if(this->group == nullptr)
                {
                    throw std::system_error(avahi_client_errno(c), avahi_error_category(), "avahi_client_new");
                }
            }

            // publish the service
            add_service(this->service_name, this->service_port);
        }

        void reset()
        {
            logger(ll::debug) << "resetting entry group\n";

            if(this->group != nullptr)
            {
                avahi_entry_group_reset(this->group);
            }
        }
    };

    // client pointer
    AvahiClient* client;

    // poller
    std::unique_ptr<avahi_poller> poller;

    // entry group object
    std::unique_ptr<avahi_group> group;

    void callback(AvahiClient* c, AvahiClientState state)
    {
        // Called whenever the client or server state changes
        switch (state)
        {
            case AVAHI_CLIENT_S_RUNNING:
                /* The server has startup successfully and registered its host
                 * name on the network, so it's time to create our services */
                logger(ll::debug) << "AVAHI_CLIENT_S_RUNNING\n";
                this->group->init(c);
                break;

            case AVAHI_CLIENT_FAILURE:
                logger(ll::debug) << "AVAHI_CLIENT_FAILURE\n";
                throw std::system_error(avahi_client_errno(c), avahi_error_category(), "avahi_client callback");
                break;

            case AVAHI_CLIENT_S_COLLISION:
                logger(ll::debug) << "AVAHI_CLIENT_S_COLLISION\n";
                /* Let's drop our registered services. When the server is back
                 * in AVAHI_SERVER_RUNNING state we will register them
                 * again with the new host name. */
                this->group->reset();
                break;

            case AVAHI_CLIENT_S_REGISTERING:
                logger(ll::debug) << "AVAHI_CLIENT_S_REGISTERING\n";
                /* The server records are now being established. This
                 * might be caused by a host name change. We need to wait
                 * for our own records to register until the host name is
                 * properly esatblished. */
                this->group->reset();
                break;

            case AVAHI_CLIENT_CONNECTING:
                logger(ll::debug) << "AVAHI_CLIENT_CONNECTING\n";
                break;
        }
    }

    // static handler
    static void static_callback(AvahiClient* c, AvahiClientState state, void* userdata)
    {
        assert(userdata != nullptr);
        reinterpret_cast<bonjour_publisher::impl*>(userdata)->callback(c, state);
    }

public:
    impl(const char* name, int port) : client(nullptr), poller(new avahi_poller()), group(new avahi_group(name, port))
    {
        logger(ll::info) << "setting up avahi service for " << name << " with port " << port << '\n';

        int error;
        this->client = avahi_client_new(this->poller->get(), static_cast<AvahiClientFlags>(0), static_callback, this, &error);
        if(this->client == nullptr)
        {
            throw std::system_error(error, avahi_error_category(), "avahi_client_new");
        }

        // start polling
        this->poller->start();
    }

    ~impl()
    {
        logger(ll::info) << "shutting down avahi service\n";

        // destroy group first
        this->group.reset();

        // destroy poller
        this->poller.reset();

        // free client
        if(this->client != nullptr)
        {
            logger(ll::debug) << "freeing avahi client\n";

            avahi_client_free(this->client);
            this->client = nullptr;
        }
    }
};
#endif

bonjour_publisher::bonjour_publisher() = default;
bonjour_publisher::~bonjour_publisher() = default;

void bonjour_publisher::publish(const char* name, int port)
{
    this->pimpl = std::unique_ptr<impl>(new impl(name, port));
}

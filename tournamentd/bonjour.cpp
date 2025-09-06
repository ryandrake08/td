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
        const char* name() const noexcept override
        {
            return "CFStreamError";
        }

        bool equivalent(const std::error_code& code, int condition) const noexcept override
        {
            return *this == code.category() && default_error_condition(code.value()).value() == condition;
        }

        std::string message(int ev) const override
        {
            return std::to_string(ev);
        }
    };

    CFNetServiceRef netService;

    static void registerCallback(CFNetServiceRef /* theService */, CFStreamError* /* error */, void* /* info */)
    {
    }

public:
    impl(const std::string& name, int port)
    {
        logger(ll::info) << "setting up bonjour service for " << name << " with port " << port << '\n';

        // describe net service
        CFStringRef theDomain = CFSTR("local.");
        CFStringRef serviceType = CFSTR("_pokerbuddy._tcp");
        CFStringRef serviceName = CFStringCreateWithCString(kCFAllocatorDefault, name.c_str(), kCFStringEncodingUTF8);

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
            CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
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

    // no copy constructors/assignment
    impl(const impl& other) = delete;
    impl& operator=(const impl& other) = delete;

    // no move constructors/assignment
    impl(impl&& other) = delete;
    impl& operator=(impl&& other) = delete;
};

#elif defined(__linux__)

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>
#include <avahi-common/domain.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/thread-watch.h>
#include <cassert>
#include <string>

struct bonjour_publisher::impl
{
    class avahi_error_category : public std::error_category
    {
    public:
        const char* name() const noexcept override
        {
            return "avahi";
        }

        bool equivalent(const std::error_code& code, int condition) const noexcept override
        {
            return *this == code.category() && default_error_condition(code.value()).value() == condition;
        }

        std::string message(int ev) const override
        {
            return avahi_strerror(ev);
        }
    };

    // service name and port
    std::string service_name;
    int service_port;

    // threaded poller
    AvahiThreadedPoll* threaded_poll { nullptr };

    // client pointer
    AvahiClient* client { nullptr };

    // entry group object
    AvahiEntryGroup* group { nullptr };

    // private utility methods
    static std::string alternative_name(const std::string& name)
    {
        logger(ll::debug) << "generating alternative name for: " << name << "\n";

        // generate alternative name
        auto* tmp(avahi_alternative_service_name(name.c_str()));
        std::string alt_name(tmp);
        avahi_free(tmp);

        logger(ll::debug) << "generated alternative name: " << alt_name << "\n";
        return alt_name;
    }

    void add_service(const std::string& name, int port)
    {
        if(avahi_entry_group_is_empty(this->group) != AVAHI_OK)
        {
            logger(ll::info) << "adding avahi service: " << name << ", port: " << port << '\n';

            int ret(0);
            ret = avahi_entry_group_add_service(this->group, // NOLINT(cppcoreguidelines-pro-type-vararg)
                                                AVAHI_IF_UNSPEC,
                                                AVAHI_PROTO_UNSPEC,
                                                static_cast<AvahiPublishFlags>(0), // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
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
                if(avahi_entry_group_reset(this->group) != AVAHI_OK)
                {
                    throw std::system_error(avahi_client_errno(this->client), avahi_error_category(), "avahi_entry_group_reset");
                }

                // try re-publishing with alternative name
                this->add_service(bonjour_publisher::impl::alternative_name(name), port);
                return; // recursive call handles commit, avoid double commit
            }
            else if(ret != AVAHI_OK)
            {
                throw std::system_error(avahi_client_errno(this->client), avahi_error_category(), "avahi_entry_group_add_service");
            }
            else
            {
                // commit
                ret = avahi_entry_group_commit(this->group);
                if(ret != AVAHI_OK)
                {
                    throw std::system_error(avahi_client_errno(this->client), avahi_error_category(), "avahi_entry_group_commit");
                }
            }
        }
    }

    void entry_group_callback(AvahiEntryGroup* /* g */, AvahiEntryGroupState state)
    {
        // called whenever the entry group state changes
        switch(state)
        {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:
            /* The entry group has been established successfully */
            logger(ll::debug) << "AVAHI_ENTRY_GROUP_ESTABLISHED\n";
            break;

        case AVAHI_ENTRY_GROUP_COLLISION:
        {
            /* A service name collision with a remote service
             * happened. Let's pick a new name */
            logger(ll::debug) << "AVAHI_ENTRY_GROUP_COLLISION\n";
            this->add_service(bonjour_publisher::impl::alternative_name(this->service_name), this->service_port);
            break;
        }
        case AVAHI_ENTRY_GROUP_FAILURE:
            logger(ll::debug) << "AVAHI_ENTRY_GROUP_FAILURE\n";
            throw std::system_error(avahi_client_errno(this->client), avahi_error_category(), "entry_group_callback callback");
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
    static void static_entry_group_callback(AvahiEntryGroup* g, AvahiEntryGroupState state, void* userdata)
    {
        assert(userdata != nullptr);
        static_cast<bonjour_publisher::impl*>(userdata)->entry_group_callback(g, state);
    }

    void client_callback(AvahiClient* c, AvahiClientState state)
    {
        // Called whenever the client or server state changes
        switch(state)
        {
        case AVAHI_CLIENT_S_RUNNING:
            logger(ll::debug) << "AVAHI_CLIENT_S_RUNNING\n";
            /* The server has startup successfully and registered its host
             * name on the network, so it's time to create our services */

            // first create our entry group if it doesn't yet exist
            if(this->group == nullptr)
            {
                logger(ll::debug) << "creating entry group\n";

                this->group = avahi_entry_group_new(c, static_entry_group_callback, this);
                if(this->group == nullptr)
                {
                    throw std::system_error(avahi_client_errno(c), avahi_error_category(), "avahi_entry_group_new");
                }
            }

            // publish the service
            this->add_service(this->service_name, this->service_port);
            break;

        case AVAHI_CLIENT_FAILURE:
            logger(ll::debug) << "AVAHI_CLIENT_FAILURE\n";
            throw std::system_error(avahi_client_errno(c), avahi_error_category(), "client_callback callback");
            break;

        case AVAHI_CLIENT_S_COLLISION:
            logger(ll::debug) << "AVAHI_CLIENT_S_COLLISION\n";
            /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */
            if(this->group != nullptr)
            {
                if(avahi_entry_group_reset(this->group) != AVAHI_OK)
                {
                    throw std::system_error(avahi_client_errno(c), avahi_error_category(), "avahi_entry_group_reset");
                }
            }
            break;

        case AVAHI_CLIENT_S_REGISTERING:
            logger(ll::debug) << "AVAHI_CLIENT_S_REGISTERING\n";
            /* The server records are now being established. This
             * might be caused by a host name change. We need to wait
             * for our own records to register until the host name is
             * properly esatblished. */
            if(this->group != nullptr)
            {
                if(avahi_entry_group_reset(this->group) != AVAHI_OK)
                {
                    throw std::system_error(avahi_client_errno(c), avahi_error_category(), "avahi_entry_group_reset");
                }
            }
            break;

        case AVAHI_CLIENT_CONNECTING:
            logger(ll::debug) << "AVAHI_CLIENT_CONNECTING\n";
            break;
        }
    }

    // static handler
    static void static_client_callback(AvahiClient* c, AvahiClientState state, void* userdata)
    {
        assert(userdata != nullptr);
        static_cast<bonjour_publisher::impl*>(userdata)->client_callback(c, state);
    }

public:
    impl(std::string name, int port) : service_name(std::move(name)), service_port(port)
    {
        // handle empty string by using fallback name
        if(this->service_name.empty())
        {
            this->service_name = "service-" + std::to_string(this->service_port);
            logger(ll::warning) << "empty service name provided, using fallback: " << this->service_name << "\n";
        }

        // validate service name before attempting to do anything
        if(!avahi_is_valid_service_name(this->service_name.c_str()))
        {
            logger(ll::error) << this->service_name << " is an invalid service name!\n";

            // for other invalid names (like very long ones), throw exception
            throw std::invalid_argument("Invalid service name: " + this->service_name);
        }

        logger(ll::debug) << "creating poller\n";

        // create poller
        this->threaded_poll = avahi_threaded_poll_new();
        if(this->threaded_poll == nullptr)
        {
            throw std::system_error(AVAHI_ERR_FAILURE, avahi_error_category(), "avahi_threaded_poll_new");
        }

        logger(ll::info) << "setting up avahi service for " << this->service_name << " with port " << this->service_port << '\n';

        // create client
        int error {};
        this->client = avahi_client_new(avahi_threaded_poll_get(this->threaded_poll),
                                        static_cast<AvahiClientFlags>(0), // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)
                                        static_client_callback,
                                        this,
                                        &error);
        if(this->client == nullptr)
        {
            throw std::system_error(error, avahi_error_category(), "avahi_client_new");
        }

        logger(ll::debug) << "starting poller\n";

        // start polling
        if(avahi_threaded_poll_start(this->threaded_poll) != AVAHI_OK)
        {
            throw std::runtime_error("could not start poller");
        }
    }

    ~impl()
    {
        logger(ll::info) << "shutting down avahi service for " << this->service_name << " with port " << this->service_port << '\n';

        // stop poller
        if(this->threaded_poll != nullptr)
        {
            logger(ll::debug) << "stopping poller\n";

            // stop
            avahi_threaded_poll_stop(this->threaded_poll);
        }

        // destroy group first
        if(this->group != nullptr)
        {
            logger(ll::debug) << "freeing entry group\n";

            avahi_entry_group_free(this->group);
            this->group = nullptr;
        }

        // free client
        if(this->client != nullptr)
        {
            logger(ll::debug) << "freeing avahi client\n";

            avahi_client_free(this->client);
            this->client = nullptr;
        }

        // free poller
        if(this->threaded_poll != nullptr)
        {
            logger(ll::debug) << "freeing poller\n";

            avahi_threaded_poll_free(this->threaded_poll);
            this->threaded_poll = nullptr;
        }
    }

    // no copy constructors/assignment
    impl(const impl& other) = delete;
    impl& operator=(const impl& other) = delete;

    // no move constructors/assignment
    impl(impl&& other) = delete;
    impl& operator=(impl&& other) = delete;
};

#else

struct bonjour_publisher::impl
{
public:
    impl(const char* name, int port)
    {
        logger(ll::info) << "platform does not support zeroconf publishing for " << name << ':' << port << '\n';
    }
};

#endif

bonjour_publisher::bonjour_publisher() = default;
bonjour_publisher::~bonjour_publisher() = default;

void bonjour_publisher::publish(const std::string& name, int port)
{
    this->pimpl = std::unique_ptr<impl>(new impl(name, port));
}

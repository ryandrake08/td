#include "bonjour.hpp"
#include "logger.hpp"
#include <system_error>

#if defined(__APPLE__)
#include <CFNetwork/CFNetwork.h>

class bonjour_impl
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

    static void registerCallback(CFNetServiceRef theService, CFStreamError* error, void* info)
    {
    }

public:
    bonjour_impl(const std::string& name, int port)
    {
        logger(LOG_INFO) << "setting up bonjour service for " << name << " with port " << port << '\n';

        // describe net service
        CFStringRef theDomain = CFSTR("local.");
        CFStringRef serviceType = CFSTR("_tournbuddy._tcp");
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
            CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(),kCFRunLoopCommonModes);
            CFNetServiceSetClient(netService, nullptr, nullptr);
            CFRelease(netService);

            throw std::system_error(error.error, cf_error_category(), "CFNetServiceRegisterWithOptions");
        }
    }

    ~bonjour_impl()
    {
        logger(LOG_INFO) << "shutting down bonjour\n";

        CFNetServiceUnscheduleFromRunLoop(netService, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
        CFNetServiceSetClient(netService, nullptr, nullptr);
        CFNetServiceCancel(netService);
        CFRelease(netService);
    }
};
#else
class bonjour_impl
{
public:
    bonjour_impl(const std::string& name, int port)
    {
    }

    ~bonjour_impl()
    {
    }
};
#endif

void bonjour_publisher::publish(const std::string& name, int port)
{
    this->impl = std::shared_ptr<bonjour_impl>(new bonjour_impl(name, port));
}

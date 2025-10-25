#define CATCH_CONFIG_RUNNER
#include "../logger.hpp"
#include <Catch2/catch.hpp>

// Disable all logging for tests to reduce noise
struct LoggerDisabler
{
    LoggerDisabler()
    {
        logger_enable(); // Empty list disables all logging
    }
};

static const LoggerDisabler logger_disabler;

int main(int argc, char* argv[])
{
    Catch::Session session;

#if defined(_WIN32)
    // On Windows, exclude unix_socket and bonjour tests by default since they're not supported
    // unix_socket: Windows doesn't support Unix domain sockets
    // bonjour: Windows zeroconf implementation not yet available
    // Users can override with command line arguments if needed
    if(argc == 1)
    {
        const char* default_args[] = { argv[0], "~[unix_socket]~[bonjour]" };
        return session.run(2, default_args);
    }
#endif

    return session.run(argc, argv);
}

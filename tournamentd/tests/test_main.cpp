#define CATCH_CONFIG_MAIN
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

static LoggerDisabler logger_disabler;

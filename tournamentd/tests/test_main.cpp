#define CATCH_CONFIG_MAIN
#include <catch_amalgamated.hpp>
#include "../logger.hpp"

// Disable all logging for tests to reduce noise
struct LoggerDisabler {
    LoggerDisabler() {
        logger_enable(); // Empty list disables all logging
    }
};

static LoggerDisabler logger_disabler;


#pragma once
#include <initializer_list>
#include <iostream>

// Macro to include current function in log
#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif
#define logger(...) logger_internal::instance().get_stream(__func__, __VA_ARGS__)

enum logger_level
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

class logger_internal
{
    // Bitmask enabling each log level
    unsigned mask;

    // Only the singleton can construct
    logger_internal();

    // No copy constructors/assignment
    logger_internal(const logger_internal& other) = delete;
    logger_internal& operator=(const logger_internal& other) = delete;

public:
    // Get singleton instance
    static logger_internal& instance();

    // Set enabled logs
    void set_enabled(std::initializer_list<logger_level> logs);

    // Returns an ostream appropriate for logging, primed with the current timestamp and a function name
    std::ostream& get_stream(const char* function, logger_level level=LOG_DEBUG);
};
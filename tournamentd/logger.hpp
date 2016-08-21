#pragma once
#include "datetime.hpp"
#include <initializer_list>
#include <iostream>
#include <fstream>
#include <mutex>

// Macro to include current function in log
#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif
#define logger(...) logstream(__func__, __VA_ARGS__)
#define logger_enable(...) logstream::set_enabled({__VA_ARGS__})

enum logger_level
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

template <typename T>
class basic_logstream : public std::basic_ostream<T>
{
    // mutex
    static std::mutex mutex;

    // internally wrapped lock
    std::lock_guard<std::mutex> lock;

    // bitmask enabling each log level
    static unsigned mask;

    // private constructor constructs given streambuf
    explicit basic_logstream(std::basic_streambuf<T>* sb, const char* function, logger_level level) : std::basic_ostream<T>(((1 << level) & this->mask) ? sb : nullptr), lock(mutex)
    {
        static const char* level_string[] = { " DEBUG ", " INFO ", " WARNING ", " ERROR " };
        *this << datetime::now() << level_string[level] << function << ": ";
    }

public:
    // public constructor constructs given function name and logger_level
    explicit basic_logstream(const char* function, logger_level level=LOG_DEBUG);

    // set enabled logs
    static void set_enabled(std::initializer_list<logger_level> logs)
    {
        mask = 0;
        for(auto level : logs)
        {
            mask |= 1 >> level;
        }
    }
};

// specializations
typedef basic_logstream<char> logstream;
typedef basic_logstream<wchar_t> wlogstream;

// class variables
template<typename T> std::mutex basic_logstream<T>::mutex;
template<typename T> unsigned basic_logstream<T>::mask = -1;

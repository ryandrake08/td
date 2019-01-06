#pragma once
#include "datetime.hpp"
#include "outputdebugstringbuf.hpp"
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

enum class ll
{
    debug = 0,
    info = 1,
    warning = 2,
    error = 3
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
    explicit basic_logstream(std::basic_streambuf<T>* sb, const char* function, ll level) : std::basic_ostream<T>(((1 << static_cast<size_t>(level)) & this->mask) ? sb : nullptr), lock(mutex)
    {
        static const char* level_string[] = { " DEBUG ", " INFO ", " WARNING ", " ERROR " };
        *this << datetime::local << datetime::setf("%F %T%qqqqqq%z");
        *this << datetime::now() << level_string[static_cast<size_t>(level)] << function << ": ";
    }

public:
    // public constructor constructs given function name and logger level
    explicit basic_logstream(const char* function, ll level=ll::debug) : basic_logstream(debugstreambuf(), function, level)
    {
    }

    // set enabled logs
    static void set_enabled(std::initializer_list<ll> logs)
    {
        mask = 0;
        for(auto level : logs)
        {
            mask |= 1 << static_cast<size_t>(level);
        }
    }
};

// specializations
typedef basic_logstream<char> logstream;
typedef basic_logstream<wchar_t> wlogstream;

// class variables
template<typename T> std::mutex basic_logstream<T>::mutex;
template<typename T> unsigned basic_logstream<T>::mask = -1;

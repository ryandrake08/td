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
    // global mutex to serialize access to basic_logstream and members
    static std::mutex mutex;

    // lock held for the life of the object to serialize basic_logstream creation and limit number globally to one
    std::lock_guard<std::mutex> lock;

    // bitmask enabling each log level
    static unsigned mask;

    // private constructor constructs given streambuf, function name, and log level
    explicit basic_logstream(std::basic_streambuf<T>* sb, const char* function, ll level) : std::basic_ostream<T>(nullptr), lock(mutex)
    {
        // set the streambuf only if given loglevel is allowed by the global mask
        if((1 << static_cast<size_t>(level)) & this->mask)
        {
            this->rdbuf(sb);
        }

        // prepend the time, log level, and function
        static const char* level_string[] = { " DEBUG ", " INFO ", " WARNING ", " ERROR " };
        *this << datetime::local << datetime::setf("%F %T%qqqqqq%z") << datetime::now() << level_string[static_cast<size_t>(level)] << function << ": ";
    }

public:
    // public constructor creates a debugstreambuf and constructs given function name and logger level
    explicit basic_logstream(const char* function, ll level=ll::debug) : basic_logstream(debugstreambuf(), function, level)
    {
    }

    // set enabled logs
    static void set_enabled(std::initializer_list<ll> logs)
    {
        // synchronize access to global mask
        std::lock_guard<std::mutex> set_lock(basic_logstream<T>::mutex);

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

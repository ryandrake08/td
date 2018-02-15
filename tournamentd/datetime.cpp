#include "datetime.hpp"
#include <ctime> // for struct tm
#include <iomanip>
#include <sstream>
#include <utility>

#if defined(_WIN32)
#define gmtime_r(a,b) gmtime((a))
#define localtime_r(a,b) localtime((a))
#define timegm(a) _mkgmtime((a))
#endif

typedef std::chrono::system_clock sc;

datetime::datetime(const std::time_t& tt) : datetime(sc::from_time_t(tt))
{
}

// Named constructor (now)
datetime datetime::now()
{
    return datetime(sc::now());
}

datetime datetime::from_gm(const std::string& iso8601)
{
    std::istringstream iss(iso8601);
    datetime dt;
    iss >> gm >> dt;
    return dt;
}

datetime datetime::from_local(const std::string& iso8601)
{
    std::istringstream iss(iso8601);
    datetime dt;
    iss >> local >> dt;
    return dt;
}

datetime datetime::from_gm(const char* iso8601)
{
    std::istringstream iss(iso8601);
    datetime dt;
    iss >> gm >> dt;
    return dt;
}

datetime datetime::from_local(const char* iso8601)
{
    std::istringstream iss(iso8601);
    datetime dt;
    iss >> local >> dt;
    return dt;
}

static int current_century()
{
    auto tt(std::time(nullptr));
    struct tm tm;
    ::gmtime_r(&tt, &tm);
    return (tm.tm_year / 100) * 100;
}

// Named constructor (from NMEA0183)
datetime datetime::from_nmea0183(const std::string& timebuf, const std::string& datebuf)
{
    struct tm tm {0,0,0,0,0,0,0,0,0,0,nullptr};
    if(datebuf != std::string())
    {
        // Parse month, date, year from NMEA
        tm.tm_mday = std::stoi(datebuf.substr(0, 2));
        tm.tm_mon = std::stoi(datebuf.substr(2, 2)) - 1;
        tm.tm_year = std::stoi(datebuf.substr(4, 2)) + current_century();
    }

    std::chrono::system_clock::duration fractional;

    if(timebuf != std::string())
    {
        // Parse hour, minute, second from NMEA
        tm.tm_hour = std::stoi(timebuf.substr(0, 2));
        tm.tm_min = std::stoi(timebuf.substr(2, 2));
        tm.tm_sec = std::stoi(timebuf.substr(4, 2));

        // Fractional part
        auto decimal(timebuf.find('.'));
        if(decimal != std::string::npos)
        {
            auto fractional_part(std::stod(timebuf.substr(decimal)));
            fractional = std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::duration<double>(fractional_part));
        }
    }

    // timegm() not portable, but convenient
    auto tt(::timegm(&tm));
    auto time_pt(std::chrono::system_clock::from_time_t(tt));
    return datetime(time_pt + fractional);
}


// Operators
bool datetime::operator!=(const datetime& other) const
{
    return this->tp != other.tp;
}

bool datetime::operator<(const datetime& other) const
{
    return this->tp < other.tp;
}

// Cast
datetime::operator std::chrono::system_clock::time_point() const
{
    return this->tp;
}

datetime::operator std::time_t() const
{
    return sc::to_time_t(this->tp);
}

static inline int mode_iword()
{
    static int i = std::ios_base::xalloc();
    return i;
}

static inline int millis_iword()
{
    static int i = std::ios_base::xalloc();
    return i;
}

// Renderers
std::string datetime::gmtime() const
{
    std::ostringstream ss;
    ss << gm << *this;
    return ss.str();
}

std::string datetime::localtime() const
{
    std::ostringstream ss;
    ss << local << *this;
    return ss.str();
}

// datetime to stream
std::ostream& operator<<(std::ostream& os, const datetime& t)
{
    auto tt(sc::to_time_t(t.tp));

    // convert to gmtime or localtime components depending on mode
    std::tm tm;
    if(os.iword(mode_iword()) == 0)
    {
        ::gmtime_r(&tt, &tm);
    }
    else
    {
        ::localtime_r(&tt, &tm);
    }

    // output components in iso-8601
    os << std::put_time(&tm, "%FT%T");

    // output milliseconds if required
    if(os.iword(millis_iword()) == 0)
    {
        auto tpbase(sc::from_time_t(tt));
        auto millis(std::chrono::duration_cast<std::chrono::milliseconds>(t.tp-tpbase).count());
        os << "." << std::setfill('0') << std::setw(3) << millis;
    }

    return os;
}

// datetime from stream
std::istream& operator>>(std::istream& is, datetime& t)
{
    struct tm tm({});
    is >> std::get_time(&tm, "%FT%T");
    if(!is.fail())
    {
        if(is.iword(mode_iword()) == 0)
        {
            // timegm() not portable, but convenient
            t.tp = sc::from_time_t(::timegm(&tm));
        }
        else
        {
            t.tp = sc::from_time_t(::mktime(&tm));
        }
    }

    return is;
}

std::ios& datetime::gm(std::ios& os)
{
    os.iword(mode_iword()) = 0;
    return os;
}

std::ios& datetime::local(std::ios& os)
{
    os.iword(mode_iword()) = 1;
    return os;
}

std::ostream& datetime::millis(std::ostream& os)
{
    os.iword(millis_iword()) = 0;
    return os;
}

std::ostream& datetime::nomillis(std::ostream& os)
{
    os.iword(millis_iword()) = 1;
    return os;
}

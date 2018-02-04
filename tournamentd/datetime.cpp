#include "datetime.hpp"
#include <iomanip>
#include <sstream>
#include <utility>

#if defined(_WIN32)
#define gmtime_r(a,b) gmtime((a))
#define localtime_r(a,b) localtime((a))
#define timegm(a) _mkgmtime((a))
extern "C" char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

typedef std::chrono::system_clock sc;

static std::tm inline_strptime(const char* iso8601)
{
    struct std::tm tmp = {0,0,0,0,0,0,0,0,0,0,nullptr};
    ::strptime(iso8601, "%Y-%m-%dT%H:%M:%S%z", &tmp);
    return tmp;
}

datetime::datetime(const std::time_t& tt) : datetime(sc::from_time_t(tt))
{
}

// Named constructor (now)
datetime datetime::now()
{
    return datetime(sc::now());
}

datetime datetime::from_gm(const std::tm& tm_s)
{
    // timegm() not portable, but convenient
    return datetime(::timegm(const_cast<std::tm*>(&tm_s)));
}

datetime datetime::from_local(const std::tm& tm_s)
{
    return datetime(std::mktime(const_cast<std::tm*>(&tm_s)));
}

datetime datetime::from_gm(const std::string& iso8601)
{
    return from_gm(iso8601.c_str());
}

datetime datetime::from_local(const std::string& iso8601)
{
    return from_local(iso8601.c_str());
}

datetime datetime::from_gm(const char* iso8601)
{
    return from_gm(inline_strptime(iso8601));
}

datetime datetime::from_local(const char* iso8601)
{
    return from_local(inline_strptime(iso8601));
}

// Named constructor (from NMEA0183)
datetime datetime::from_nmea0183(const std::string& timebuf, const std::string& datebuf)
{
    struct tm tm;
    datetime::now().gmtime(tm);

    if(datebuf != std::string())
    {
        // Store current century (NMEA0183 specifies only two digits for year)
        int century = (tm.tm_year / 100) * 100;

        // Parse month, date, year from NMEA
        std::stringstream(datebuf.substr(0, 2)) >> tm.tm_mday;
        std::stringstream(datebuf.substr(2, 2)) >> tm.tm_mon;
        std::stringstream(datebuf.substr(4, 2)) >> tm.tm_year;

        // Restore century
        tm.tm_year += century;

        // Adjust mon from 1->mon to 0->mon-1
        tm.tm_mon -= 1;
    }

    double fractional(0.0);

    if(timebuf != std::string())
    {
        // Parse hour, minute, second from NMEA
        std::stringstream(timebuf.substr(0, 2)) >> tm.tm_hour;
        std::stringstream(timebuf.substr(2, 2)) >> tm.tm_min;
        std::stringstream(timebuf.substr(4, 2)) >> tm.tm_sec;

        // Fractional part
        auto decimal(timebuf.find('.'));
        if(decimal != std::string::npos)
        {
            std::stringstream(timebuf.substr(decimal)) >> fractional;
        }
    }

    // timegm() not portable, but convenient
    auto time_pt(std::chrono::system_clock::from_time_t(::timegm(&tm)));
    auto ms(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(fractional)));
    return datetime(time_pt + ms);
}


// To tm structure
std::tm& datetime::gmtime(std::tm& tm_s) const
{
    auto tt(sc::to_time_t(this->tp));
    return *::gmtime_r(&tt, &tm_s);
}

std::tm& datetime::localtime(std::tm& tm_s) const
{
    auto tt(sc::to_time_t(this->tp));
    return *::localtime_r(&tt, &tm_s);
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

static std::ostream& operator<<(std::ostream& os, const std::tm* date_time)
{
    const std::size_t size(1024);
    char buffer[size] = {0};
    if(std::strftime(buffer, size, "%Y-%m-%dT%H:%M:%S", date_time) != 0)
    {
        return os << buffer;
    }
    else
    {
        return os;
    }
}

// Renderers
std::string datetime::gmtime() const
{
    std::tm tm;
    tm = this->gmtime(tm);
    std::ostringstream ss;
    ss << &tm;
    return ss.str();
}

std::string datetime::localtime() const
{
    std::tm tm;
    tm = this->localtime(tm);
    std::ostringstream ss;
    ss << &tm;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const datetime& t)
{
    auto tt(sc::to_time_t(t.tp));
    auto tpbase(sc::from_time_t(tt));
    auto millis(std::chrono::duration_cast<std::chrono::milliseconds>(t.tp-tpbase).count());
    auto mode(os.iword(mode_iword()));
    auto nomillis(os.iword(millis_iword()));

    std::tm tm;
    if(mode == 0)
    {
        tm = t.gmtime(tm);
    }
    else
    {
        tm = t.localtime(tm);
    }

    if(nomillis != 0)
    {
        return os << &tm;
    }
    else
    {
        return os << &tm << "." << std::setfill('0') << std::setw(3) << millis;
    }
}

std::ostream& datetime::gm(std::ostream& os)
{
    os.iword(mode_iword()) = 0;
    return os;
}

std::ostream& datetime::local(std::ostream& os)
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

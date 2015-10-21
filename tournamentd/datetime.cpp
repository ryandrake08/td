#include "datetime.hpp"
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#define gmtime_r(a,b) gmtime((a))
#define localtime_r(a,b) localtime((a))
#define timegm(a) _mkgmtime((a))
extern "C" char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

typedef std::chrono::system_clock sc;

static std::tm inline_strptime(const char* iso8601)
{
    struct std::tm tmp = {};
    ::strptime(iso8601, "%Y-%m-%dT%H:%M:%S%z", &tmp);
    return tmp;
}

datetime::datetime()
{
}

datetime::datetime(const sc::time_point& time_pt) : tp(time_pt)
{
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

static inline int geti()
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
    auto mode(os.iword(geti()));

    std::tm tm;
    if(mode == 0)
    {
        tm = t.gmtime(tm);
    }
    else
    {
        tm = t.localtime(tm);
    }

    if(millis != 0)
    {
        return os << &tm << "." << std::setfill('0') << std::setw(3) << millis;
    }
    else
    {
        return os << &tm;
    }
}

std::ostream& datetime::gm(std::ostream& os)
{
    os.iword(geti()) = 0;
    return os;
}

std::ostream& datetime::local(std::ostream& os)
{
    os.iword(geti()) = 1;
    return os;
}

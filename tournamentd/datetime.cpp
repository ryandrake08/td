#include "datetime.hpp"
#include <iomanip>

#if defined(_WIN32)
#define gmtime_r(a,b) gmtime((a))
#define localtime_r(a,b) localtime((a))
#endif

typedef std::chrono::system_clock sc;

datetime::datetime()
{
}

datetime::datetime(const sc::time_point& time_pt) : tp(time_pt)
{
}

datetime::datetime(const std::time_t& tt) : datetime(sc::from_time_t(tt))
{
}

datetime::datetime(const std::tm& tm_s) : datetime(std::mktime(const_cast<std::tm*>(&tm_s)))
{
}

// Named constructor (now)
datetime datetime::now()
{
    return datetime(sc::now());
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
    if(std::strftime(buffer, size, "%Y-%m-%d %H:%M:%S", date_time) != 0)
    {
        return os << buffer;
    }
    else
    {
        return os;
    }
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

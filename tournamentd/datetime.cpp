#include "datetime.hpp"
#include <iomanip>

datetime::datetime()
{
}

datetime::datetime(const std::chrono::system_clock::time_point& time_pt) : tp(time_pt)
{
}

// Named constructor (now)
datetime datetime::now()
{
    return datetime(std::chrono::system_clock::now());
}

// To tm structure
std::tm& datetime::gmtime(std::tm& tm_s) const
{
    auto tt = std::chrono::system_clock::to_time_t(this->tp);
    return *::gmtime_r(&tt, &tm_s);
}

std::tm& datetime::localtime(std::tm& tm_s) const
{
    auto tt = std::chrono::system_clock::to_time_t(this->tp);
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

static inline int geti()
{
    static int i = std::ios_base::xalloc();
    return i;
}

static std::ostream& operator<<(std::ostream& os, const std::tm* date_time)
{
    const size_t size(1024);
    char buffer[size] = {0};
    if(std::strftime(buffer, size, "%F %T", date_time) != 0)
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
    auto tt(std::chrono::system_clock::to_time_t(t.tp));
    auto tpbase(std::chrono::system_clock::from_time_t(tt));
    auto millis(std::chrono::duration_cast<std::chrono::milliseconds>(t.tp-tpbase).count());
    auto mode(os.iword(geti()));

    std::tm tm;
    if(mode == 0)
    {
        t.gmtime(tm);
    }
    else
    {
        t.localtime(tm);
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

#pragma once
#include <chrono>
#include <ostream>
#include <string>

class datetime
{
    std::chrono::system_clock::time_point tp;

public:
    datetime() = default;
    explicit constexpr datetime(const std::chrono::system_clock::time_point& time_pt) : tp(time_pt) {}
    explicit datetime(const std::time_t& tt);

    // Named constructors
    static datetime now();
    static datetime from_gm(const std::tm& tm_s);
    static datetime from_local(const std::tm& tm_s);
    static datetime from_gm(const std::string& iso8601);
    static datetime from_local(const std::string& iso8601);
    static datetime from_gm(const char* iso8601);
    static datetime from_local(const char* iso8601);
    static datetime from_nmea0183(const std::string& timebuf, const std::string& datebuf=std::string());

    // Renderers
    std::string gmtime() const;
    std::string localtime() const;

    // Operators
    bool operator==(const datetime& other) const;
    bool operator!=(const datetime& other) const;
    bool operator<(const datetime& other) const;

    // Cast
    operator std::chrono::system_clock::time_point() const;
    operator std::time_t() const;

    // Stream insertion/extraction
    friend std::ostream& operator<<(std::ostream& os, const datetime& t);
    friend std::istream& operator>>(std::istream& is, datetime& t);

    // Stream manipulator: put/get datetime as gmtime
    static std::ios& gm(std::ios& os);

    // Stream manipulator: put/get datetime is localtime
    static std::ios& local(std::ios& os);

    // Stream manipulator: put/get datetime is iso8601 format
    static std::ios& iso8601(std::ios& os);

    // Stream manipulator: put/get datetime format string
    class setf
    {
        const char* f;
    public:
        explicit setf(const char* format);
        friend std::ostream& operator<<(std::ostream& os, const setf& obj);
        friend std::istream& operator>>(std::istream& is, const setf& obj);
    };
};

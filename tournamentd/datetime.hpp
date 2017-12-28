#pragma once
#include <ctime> // for struct tm
#include <chrono>
#include <ostream>
#include <string>

class datetime
{
    std::chrono::system_clock::time_point tp;

    // To tm structure
    std::tm& gmtime(std::tm& tm_s) const;
    std::tm& localtime(std::tm& tm_s) const;

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
    bool operator!=(const datetime& other) const;
    bool operator<(const datetime& other) const;

    // Cast
    operator std::chrono::system_clock::time_point() const;
    operator std::time_t() const;

    // Stream insertion
    friend std::ostream& operator<<(std::ostream& os, const datetime& t);

    // Stream insertion manipulator: datetime output is gmtime
    static std::ostream& gm(std::ostream& os);

    // Stream insertion manipulator: datetime output is localtime
    static std::ostream& local(std::ostream& os);

    // Stream insertion manipulator: datetime output includes milliseconds
    static std::ostream& millis(std::ostream& os);

    // Stream insertion manipulator: datetime output does not include milliseconds
    static std::ostream& nomillis(std::ostream& os);
};

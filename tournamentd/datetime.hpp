#pragma once
#include <ctime> // for struct tm
#include <chrono>
#include <ostream>

class datetime
{
    std::chrono::system_clock::time_point tp;

    // To tm structure
    std::tm& gmtime(std::tm& tm_s) const;
    std::tm& localtime(std::tm& tm_s) const;

public:
    datetime();
    explicit datetime(const std::chrono::system_clock::time_point& time_pt);

    // Named constructor (now)
    static datetime now();

    // Operators
    bool operator!=(const datetime& other) const;
    bool operator<(const datetime& other) const;

    // Stream insertion
    friend std::ostream& operator<<(std::ostream& os, const datetime& t);

    // Manipulators for stream insertion
    static std::ostream& gm(std::ostream& os);
    static std::ostream& local(std::ostream& os);
};
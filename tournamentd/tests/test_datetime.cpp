#include <catch_amalgamated.hpp>
#include "../datetime.hpp"
#include <sstream>
#include <chrono>
#include <thread>

TEST_CASE("DateTime creation and basic operations", "[datetime]") {
    SECTION("Default constructor") {
        datetime dt;
        REQUIRE_NOTHROW(dt);
    }

    SECTION("Constructor from time_point") {
        auto now_tp = std::chrono::system_clock::now();
        datetime dt(now_tp);
        REQUIRE(static_cast<std::chrono::system_clock::time_point>(dt) == now_tp);
    }

    SECTION("Constructor from time_t") {
        std::time_t tt = std::time(nullptr);
        datetime dt(tt);
        REQUIRE(static_cast<std::time_t>(dt) == tt);
    }
}

TEST_CASE("DateTime named constructors", "[datetime]") {
    SECTION("now() returns current time") {
        auto dt1 = datetime::now();
        auto dt2 = datetime::now();
        // Check that times are within 1 second of each other (very generous)
        auto tp1 = static_cast<std::chrono::system_clock::time_point>(dt1);
        auto tp2 = static_cast<std::chrono::system_clock::time_point>(dt2);
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1);
        REQUIRE(std::abs(diff.count()) < 1000); // Less than 1 second difference
    }

    SECTION("from_gm with tm struct") {
        std::tm tm_s = {};
        tm_s.tm_year = 123; // 2023
        tm_s.tm_mon = 0;    // January
        tm_s.tm_mday = 1;   // 1st
        tm_s.tm_hour = 12;
        tm_s.tm_min = 30;
        tm_s.tm_sec = 45;

        REQUIRE_NOTHROW(datetime::from_gm(tm_s));
    }

    SECTION("from_local with tm struct") {
        std::tm tm_s = {};
        tm_s.tm_year = 123; // 2023
        tm_s.tm_mon = 5;    // June
        tm_s.tm_mday = 15;  // 15th

        REQUIRE_NOTHROW(datetime::from_local(tm_s));
    }

    SECTION("from_gm with ISO8601 string") {
        REQUIRE_NOTHROW(datetime::from_gm("2023-01-01T12:30:45Z"));
        REQUIRE_NOTHROW(datetime::from_gm("2023-12-31T23:59:59Z"));
    }

    SECTION("from_local with ISO8601 string") {
        REQUIRE_NOTHROW(datetime::from_local("2023-06-15T14:30:00"));
    }

    SECTION("from_gm with C string") {
        REQUIRE_NOTHROW(datetime::from_gm("2023-01-01T00:00:00Z"));
    }

    SECTION("from_local with C string") {
        REQUIRE_NOTHROW(datetime::from_local("2023-01-01T00:00:00"));
    }

    SECTION("from_nmea0183 with time and date") {
        REQUIRE_NOTHROW(datetime::from_nmea0183("123045", "010123"));
        REQUIRE_NOTHROW(datetime::from_nmea0183("235959", "311223"));
    }

    SECTION("from_nmea0183 with time only") {
        REQUIRE_NOTHROW(datetime::from_nmea0183("120000"));
    }
}

TEST_CASE("DateTime rendering", "[datetime]") {
    auto dt = datetime::now();

    SECTION("gmtime() returns string") {
        std::string gmt_str = dt.gmtime();
        REQUIRE_FALSE(gmt_str.empty());
        REQUIRE(gmt_str.length() > 10); // Should be a reasonable date string
    }

    SECTION("localtime() returns string") {
        std::string local_str = dt.localtime();
        REQUIRE_FALSE(local_str.empty());
        REQUIRE(local_str.length() > 10);
    }
}

TEST_CASE("DateTime operators", "[datetime]") {
    auto dt1 = datetime::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    auto dt2 = datetime::now();

    SECTION("Inequality operator") {
        REQUIRE(dt1 != dt2);
        REQUIRE_FALSE(dt1 != dt1);
    }

    SECTION("Less than operator") {
        REQUIRE(dt1 < dt2);
        REQUIRE_FALSE(dt2 < dt1);
        REQUIRE_FALSE(dt1 < dt1);
    }
}

TEST_CASE("DateTime casts", "[datetime]") {
    auto now_tp = std::chrono::system_clock::now();
    auto now_tt = std::chrono::system_clock::to_time_t(now_tp);

    SECTION("Cast to time_point") {
        datetime dt(now_tp);
        std::chrono::system_clock::time_point tp = dt;
        REQUIRE(tp == now_tp);
    }

    SECTION("Cast to time_t") {
        datetime dt(now_tt);
        std::time_t tt = dt;
        REQUIRE(tt == now_tt);
    }
}

TEST_CASE("DateTime stream operations", "[datetime]") {
    auto dt = datetime::now();

    SECTION("Stream insertion") {
        std::ostringstream oss;
        oss << dt;
        REQUIRE_FALSE(oss.str().empty());
    }

    SECTION("Stream extraction") {
        std::istringstream iss("2023-01-01T12:30:45");
        datetime dt_extracted;
        REQUIRE_NOTHROW(iss >> dt_extracted);
    }
}

TEST_CASE("DateTime stream manipulators", "[datetime]") {
    SECTION("gm manipulator") {
        std::ostringstream oss;
        REQUIRE_NOTHROW(oss << datetime::gm);
    }

    SECTION("local manipulator") {
        std::ostringstream oss;
        REQUIRE_NOTHROW(oss << datetime::local);
    }

    SECTION("iso8601 manipulator") {
        std::ostringstream oss;
        REQUIRE_NOTHROW(oss << datetime::iso8601);
    }

    SECTION("setf manipulator") {
        std::ostringstream oss;
        REQUIRE_NOTHROW(oss << datetime::setf("%Y-%m-%d"));
    }
}

TEST_CASE("DateTime edge cases", "[datetime]") {
    SECTION("Invalid ISO8601 strings should not crash") {
        REQUIRE_NOTHROW(datetime::from_gm("invalid-date"));
        REQUIRE_NOTHROW(datetime::from_local("not-a-date"));
    }

    SECTION("Invalid NMEA strings should throw exceptions") {
        REQUIRE_THROWS(datetime::from_nmea0183("invalid"));
        REQUIRE_THROWS(datetime::from_nmea0183("abc", "def"));
        REQUIRE_THROWS(datetime::from_nmea0183("12", "34"));  // Too short strings
    }

    SECTION("Boundary dates") {
        REQUIRE_NOTHROW(datetime::from_gm("1970-01-01T00:00:00Z"));
        REQUIRE_NOTHROW(datetime::from_gm("2038-01-19T03:14:07Z"));
    }
}


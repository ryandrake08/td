#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "json.hpp"

static const auto bad_json = "{1";
static const auto good_json = "1";

TEST_CASE("eval named constructor", "[json]")
{
    SECTION("invalid data")
    {
        CHECK_THROWS_AS(json::eval(bad_json), std::runtime_error);
    }

    SECTION("valid data")
    {
        // make sure eval does not throw
        auto v(json::eval(good_json));
        // make sure json is valid
        CHECK(v.valid());
        // make sure json is not empty
        CHECK_FALSE(v.empty());
        // make sure json value is 1
        CHECK(v.value<int>() == 1);
        // make sure access as non-number throws
        CHECK_THROWS_AS(v.value<std::string>(), std::invalid_argument);
    }
}

TEST_CASE("load named constructor", "[json]")
{
    SECTION("valid file")
    {
        // make sure load does not throw
        auto v(json::load("defaults.json"));
        // make sure json is valid
        CHECK(v.valid());
    }

    SECTION("invalid file")
    {
        // make sure load does not throw
        CHECK_THROWS_AS(json::load("invalid.json"), std::runtime_error);
    }
}

TEST_CASE("default constructor", "[json]")
{
    json v;
    CHECK(v.empty());
    CHECK_THROWS_AS(v.value<int>(), std::invalid_argument);
}

TEST_CASE("copy constructor", "[json]")
{
    auto v0(json::eval(good_json));
    // make sure json value is 1
    CHECK(v0.value<int>() == 1);
    // make sure copy constructor does not throw
    auto v1(v0);
    // make sure pointers are different (copy constructor constructs a new object)
    CHECK_FALSE(v0 == v1);
    // make sure contents are the same
    CHECK(v0.identical(v1));
}

TEST_CASE("move constructor", "[json]")
{
    auto v0(json::eval(good_json));
    // make sure v0 value is 1
    CHECK(v0.value<int>() == 1);
    // make sure move constructor does not throw
    auto v1(std::move(v0));
    // make sure v0 is invalid
    CHECK_FALSE(v0.valid());
    // make sure v1 value is 1
    CHECK(v1.value<int>() == 1);
}

TEST_CASE("copy assignment", "[json]")
{
    auto v0(json::eval(good_json));
    // make sure json value is 1
    CHECK(v0.value<int>() == 1);
    json v1;
    // make sure assignment does not throw
    CHECK_NOTHROW(v1 = v0);
    // make sure pointers are different (copy constructor constructs a new object)
    CHECK_FALSE(v0 == v1);
    // make sure contents are the same
    CHECK(v0.identical(v1));
}

TEST_CASE("move assignment", "[json]")
{
    auto v0(json::eval(good_json));
    // make sure v0 value is 1
    CHECK(v0.value<int>() == 1);
    json v1;
    // make sure assignment does not throw
    CHECK_NOTHROW(v1 = std::move(v0));
    // make sure v0 is invalid
    CHECK_FALSE(v0.valid());
    // make sure v1 value is 1
    CHECK(v1.value<int>() == 1);
}

TEST_CASE("construction from int", "[json]")
{
    json v(1);
    // make sure value is retained
    CHECK(v.value<int>() == 1);
    // make sure access as non-number throws
    CHECK_THROWS_AS(v.value<std::string>(), std::invalid_argument);
}

TEST_CASE("construction from double", "[json]")
{
    json v(1.0);
    // make sure value is retained
    CHECK(v.value<double>() == 1.0);
    // make sure value can be returned as int
    CHECK(v.value<int>() == 1);
    // make sure access as non-number throws
    CHECK_THROWS_AS(v.value<std::string>(), std::invalid_argument);
}

TEST_CASE("construction from bool", "[json]")
{
    json v(true);
    // make sure value is retained
    CHECK(v.value<bool>() == true);
    // make sure access as non-bool throws
    CHECK_THROWS_AS(v.value<int>(), std::invalid_argument);
}

TEST_CASE("construction from string", "[json]")
{
    json v(std::string("test"));
    // make sure value is retained
    CHECK(v.value<std::string>() == std::string("test"));
    // make sure access as non-string throws
    CHECK_THROWS_AS(v.value<int>(), std::invalid_argument);
}

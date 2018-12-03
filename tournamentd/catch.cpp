#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("case", "[class]")
{
    SECTION("section")
    {
        CHECK(true);
    }
}

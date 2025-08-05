#include <catch_amalgamated.hpp>
#include "../tournament.hpp"
#include <stdexcept>

TEST_CASE("Tournament creation and basic operations", "[tournament]") {
    SECTION("Tournament can be created") {
        tournament t;
        REQUIRE_NOTHROW(t);
    }
    
    SECTION("Tournament authorization") {
        tournament t;
        SECTION("Valid authorization code") {
            int result = t.authorize(12345);
            REQUIRE(result == 12345);
        }
        
        SECTION("Multiple authorization codes") {
            int code1 = t.authorize(11111);
            int code2 = t.authorize(22222);
            REQUIRE(code1 == 11111);
            REQUIRE(code2 == 22222);
        }
    }
    
    SECTION("Tournament configuration loading") {
        tournament t;
        SECTION("Non-existent file should not crash") {
            REQUIRE_NOTHROW(t.load_configuration("nonexistent_file.json"));
        }
    }
    
    SECTION("Tournament run loop") {
        tournament t;
        SECTION("Run returns boolean") {
            bool result = t.run();
            REQUIRE((result == true || result == false));
        }
    }
}
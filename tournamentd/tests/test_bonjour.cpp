#include <Catch2/catch.hpp>
#include "../bonjour.hpp"
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <vector>

TEST_CASE("Bonjour publisher creation and destruction", "[bonjour][basic]") {
    SECTION("Default constructor") {
        std::unique_ptr<bonjour_publisher> bp;
        REQUIRE_NOTHROW(bp = std::unique_ptr<bonjour_publisher>(new bonjour_publisher()));
    }

    SECTION("Destruction") {
        auto bp = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());
        REQUIRE_NOTHROW(bp.reset());
    }

    SECTION("Multiple instances") {
        auto bp1 = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());
        auto bp2 = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());
        REQUIRE(bp1 != nullptr);
        REQUIRE(bp2 != nullptr);
    }

    SECTION("RAII behavior") {
        {
            bonjour_publisher bp;
            // Should clean up automatically when going out of scope
        }
        REQUIRE(true); // If we get here, cleanup worked
    }
}

TEST_CASE("Bonjour service publishing", "[bonjour][publish]") {
    SECTION("Publish with valid parameters") {
        bonjour_publisher bp;

        // Test publishing with various valid parameters
        REQUIRE_NOTHROW(bp.publish("TestTournament", 25600));
        REQUIRE_NOTHROW(bp.publish("AnotherTournament", 25601));
        REQUIRE_NOTHROW(bp.publish("Tournament123", 30000));
    }

    SECTION("Publish with various service names") {
        bonjour_publisher bp;

        // Test different service name formats
        REQUIRE_NOTHROW(bp.publish("Simple", 25600));
        REQUIRE_NOTHROW(bp.publish("With Spaces", 25601));
        REQUIRE_NOTHROW(bp.publish("With-Dashes", 25602));
        REQUIRE_NOTHROW(bp.publish("With_Underscores", 25603));
        REQUIRE_NOTHROW(bp.publish("With.Dots", 25604));
        REQUIRE_NOTHROW(bp.publish("MixedCaseService", 25605));
        REQUIRE_NOTHROW(bp.publish("service123", 25606));

        // Special characters that might be problematic
        REQUIRE_NOTHROW(bp.publish("Service!@#", 25607));
        REQUIRE_NOTHROW(bp.publish("Service()", 25608));

        // Unicode characters
        REQUIRE_NOTHROW(bp.publish("Tournament🏆", 25609));
        REQUIRE_NOTHROW(bp.publish("锦标赛", 25610));
    }

    SECTION("Publish with various port numbers") {
        bonjour_publisher bp;

        // Test different port ranges
        REQUIRE_NOTHROW(bp.publish("LowPort", 1024));    // Low user port
        REQUIRE_NOTHROW(bp.publish("MidPort", 25600));   // Default tournament port
        REQUIRE_NOTHROW(bp.publish("HighPort", 65000));  // High port number

        // Edge cases for port numbers
        REQUIRE_NOTHROW(bp.publish("Port1", 1));         // Very low port
        REQUIRE_NOTHROW(bp.publish("MaxPort", 65535));   // Maximum port
    }

    SECTION("Publish edge cases") {
        bonjour_publisher bp;

        // Empty service name
        REQUIRE_NOTHROW(bp.publish("", 25600));

#ifdef __APPLE__
        // macOS allows up to 255 characters
        std::string max_length_name(255, 'A');
        REQUIRE_NOTHROW(bp.publish(max_length_name, 25600));

        std::string over_limit_name(256, 'B');
        REQUIRE_THROWS(bp.publish(over_limit_name, 25600));
#else
        // Linux/Avahi and other platforms: 63 characters maximum
        std::string max_length_name(63, 'A');
        REQUIRE_NOTHROW(bp.publish(max_length_name, 25600));

        std::string over_limit_name(64, 'B');
        REQUIRE_THROWS(bp.publish(over_limit_name, 25600));
#endif

        // Very long service name should throw on all platforms
        std::string very_long_name(1000, 'C');
        REQUIRE_THROWS(bp.publish(very_long_name, 25600));

        // Port 0 (might be used for dynamic port assignment)
        REQUIRE_NOTHROW(bp.publish("ZeroPort", 0));

        // Negative port (invalid, should handle gracefully)
        REQUIRE_NOTHROW(bp.publish("NegativePort", -1));

        // Port out of valid range
        REQUIRE_NOTHROW(bp.publish("InvalidPort", 70000));
    }
}

TEST_CASE("Bonjour service republishing", "[bonjour][republish]") {
    SECTION("Multiple publish calls with same service") {
        bonjour_publisher bp;

        // Publish same service multiple times (should update or handle gracefully)
        REQUIRE_NOTHROW(bp.publish("SameService", 25600));
        REQUIRE_NOTHROW(bp.publish("SameService", 25600)); // Same port
        REQUIRE_NOTHROW(bp.publish("SameService", 25601)); // Different port
        REQUIRE_NOTHROW(bp.publish("SameService", 25600)); // Back to original port
    }

    SECTION("Publish different services in sequence") {
        bonjour_publisher bp;

        // Publish different services one after another
        REQUIRE_NOTHROW(bp.publish("Service1", 25600));
        REQUIRE_NOTHROW(bp.publish("Service2", 25601));
        REQUIRE_NOTHROW(bp.publish("Service3", 25602));

        // Go back to first service
        REQUIRE_NOTHROW(bp.publish("Service1", 25603));
    }

    SECTION("Rapid publishing") {
        bonjour_publisher bp;

        // Rapidly publish many services (stress test)
        for (int i = 0; i < 10; ++i) {
            std::string service_name = "RapidService" + std::to_string(i);
            int port = 25600 + i;
            REQUIRE_NOTHROW(bp.publish(service_name, port));
        }
    }
}

TEST_CASE("Bonjour service lifecycle", "[bonjour][lifecycle]") {
    SECTION("Publish and destroy publisher") {
        {
            bonjour_publisher bp;
            bp.publish("LifecycleTest", 25600);

            // Small delay to ensure service has time to register
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // Publisher destroyed, service should be unpublished

        // Create new publisher with same service name
        {
            bonjour_publisher bp2;
            REQUIRE_NOTHROW(bp2.publish("LifecycleTest", 25600));
        }
    }

    SECTION("Multiple publishers with different services") {
        bonjour_publisher bp1;
        bonjour_publisher bp2;
        bonjour_publisher bp3;

        REQUIRE_NOTHROW(bp1.publish("Service1", 25600));
        REQUIRE_NOTHROW(bp2.publish("Service2", 25601));
        REQUIRE_NOTHROW(bp3.publish("Service3", 25602));

        // All should coexist without problems
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    SECTION("Publisher reuse") {
        bonjour_publisher bp;

        // Use same publisher for multiple different services over time
        REQUIRE_NOTHROW(bp.publish("FirstService", 25600));
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        REQUIRE_NOTHROW(bp.publish("SecondService", 25601));
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        REQUIRE_NOTHROW(bp.publish("ThirdService", 25602));
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

TEST_CASE("Bonjour error handling", "[bonjour][errors]") {
    SECTION("Network unavailability") {
        bonjour_publisher bp;

        // These should not crash even if Bonjour/Avahi is not available
        // or network interfaces are down
        REQUIRE_NOTHROW(bp.publish("NetworkTest", 25600));
        REQUIRE_NOTHROW(bp.publish("AnotherNetworkTest", 25601));
    }

    SECTION("System resource exhaustion simulation") {
        // Create many publishers to test resource handling
        std::vector<std::unique_ptr<bonjour_publisher>> publishers;

        for (int i = 0; i < 50; ++i) {
            auto bp = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());
            std::string service_name = "ResourceTest" + std::to_string(i);

            REQUIRE_NOTHROW(bp->publish(service_name, 25600 + i));
            publishers.push_back(std::move(bp));
        }

        // All publishers going out of scope should clean up properly
    }

    SECTION("Concurrent publishing") {
        // Test thread safety by publishing from multiple threads
        const int num_threads = 5;
        const int services_per_thread = 10;

        std::vector<std::thread> threads;
        std::vector<std::unique_ptr<bonjour_publisher>> publishers(num_threads);

        for (size_t t = 0; t < num_threads; ++t) {
            publishers[t] = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());

            threads.emplace_back([&publishers, t]() {
                for (int i = 0; i < services_per_thread; ++i) {
                    std::string service_name = "Thread" + std::to_string(t) + "Service" + std::to_string(i);
                    int port = 26000 + (int)t * 100 + i;

                    REQUIRE_NOTHROW(publishers[t]->publish(service_name, port));

                    // Small delay between publications
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            });
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }
    }
}

TEST_CASE("Bonjour platform compatibility", "[bonjour][platform]") {
    SECTION("Cross-platform service names") {
        bonjour_publisher bp;

        // Test service names that should work across different platforms
        REQUIRE_NOTHROW(bp.publish("CrossPlatform", 25600));
        REQUIRE_NOTHROW(bp.publish("cross-platform", 25601));
        REQUIRE_NOTHROW(bp.publish("CROSS_PLATFORM", 25602));

        // DNS-safe characters only
        REQUIRE_NOTHROW(bp.publish("dns-safe-123", 25603));
        REQUIRE_NOTHROW(bp.publish("DNSSafe123", 25604));
    }

    SECTION("Service discovery protocol compliance") {
        bonjour_publisher bp;

        // Test names that comply with DNS-SD standards
        REQUIRE_NOTHROW(bp.publish("valid-service", 25600));
        REQUIRE_NOTHROW(bp.publish("another.valid.service", 25601));
        REQUIRE_NOTHROW(bp.publish("123-numeric-prefix", 25602));

        // Test boundary cases for DNS labels
        std::string max_label(63, 'a'); // Maximum DNS label length
        REQUIRE_NOTHROW(bp.publish(max_label, 25603));
    }
}

TEST_CASE("Bonjour service information", "[bonjour][service_info]") {
    SECTION("Standard tournament service") {
        bonjour_publisher bp;

        // Publish service similar to what the actual tournament daemon would use
        REQUIRE_NOTHROW(bp.publish("Tournament Director", 25600));

        // Test with version information in name
        REQUIRE_NOTHROW(bp.publish("Tournament Director v2.0", 25601));

        // Test with host information
        REQUIRE_NOTHROW(bp.publish("Tournament Director - localhost", 25602));
    }

    SECTION("Multiple tournament instances") {
        std::vector<std::unique_ptr<bonjour_publisher>> publishers;

        // Simulate multiple tournament instances running
        for (int i = 1; i <= 3; ++i) {
            auto bp = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());
            std::string service_name = "Tournament " + std::to_string(i);
            int port = 25600 + i - 1;

            REQUIRE_NOTHROW(bp->publish(service_name, port));
            publishers.push_back(std::move(bp));
        }

        // Small delay to ensure all services are published
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

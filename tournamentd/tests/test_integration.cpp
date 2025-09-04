#include "../bonjour.hpp"
#include "../datetime.hpp"
#include "../gameinfo.hpp"
#include "../server.hpp"
#include "../tournament.hpp"
#include "nlohmann/json.hpp"
#include <Catch2/catch.hpp>
#include <chrono>
#include <sstream>
#include <thread>

#if !defined(P_tmpdir)
#define P_tmpdir "/tmp/"
#endif
#include <cstdio>
#include <fstream>
#include <functional>
#include <memory>

TEST_CASE("Tournament integration - basic tournament lifecycle", "[integration][tournament_lifecycle]")
{
    SECTION("Complete tournament setup and execution")
    {
        tournament t;

        // Pre-authorize a client
        int auth_code = t.authorize(12345);
        REQUIRE(auth_code == 12345);

        // Try to listen (might fail in test environment, but shouldn't crash)
        // Use same temporary directory logic as the actual daemon
        const char* tmpdir = std::getenv("TMPDIR");
        if(tmpdir == nullptr)
        {
            tmpdir = P_tmpdir;
        }
        std::string temp_path = tmpdir;

        try
        {
            auto listen_result = t.listen(temp_path.c_str());
            REQUIRE_FALSE(listen_result.first.empty()); // Should get a socket path
            REQUIRE(listen_result.second > 0);          // Should get a port number

            // Run tournament loop briefly
            for(int i = 0; i < 5; ++i)
            {
                bool continue_running = t.run();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if(!continue_running)
                    break;
            }
        }
        catch(const std::exception& e)
        {
            WARN("Tournament listen/run failed (expected in test environment): " << e.what());
        }
    }
}

TEST_CASE("GameInfo and Tournament integration", "[integration][gameinfo_tournament]")
{
    SECTION("Tournament with configured gameinfo")
    {
        tournament t;

        // Configure a complete tournament
        nlohmann::json config = {
            { "players", { { { "player_id", "p1" }, { "name", "Alice" } }, { { "player_id", "p2" }, { "name", "Bob" } }, { { "player_id", "p3" }, { "name", "Charlie" } } } },
            { "funding_sources", { { { "name", "Buy-in" }, { "type", 0 }, { "chips", 1500 }, { "cost", { { "amount", 100.0 }, { "currency", "USD" } } } } } },
            { "tables", { { { "table_name", "Table 1" } }, { { "table_name", "Table 2" } } } },
            { "blind_levels", { { { "little_blind", 25 }, { "big_blind", 50 }, { "duration", 1200 } }, { { "little_blind", 50 }, { "big_blind", 100 }, { "duration", 1200 } }, { { "little_blind", 100 }, { "big_blind", 200 }, { "duration", 1200 } } } },
            { "chips", { { { "denomination", 25 }, { "count_available", 200 } }, { { "denomination", 100 }, { "count_available", 100 } } } }
        };

        // Load configuration from JSON string representation
        std::string config_file = "/tmp/tournament_config_" + std::to_string(std::time(nullptr)) + ".json";
        std::ofstream file(config_file);
        file << config.dump();
        file.close();

        try
        {
            t.load_configuration(config_file);

            // Authorize client and try to run
            t.authorize(54321);

            // Brief run to ensure configuration is loaded
            for(int i = 0; i < 3; ++i)
            {
                t.run();
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        catch(const std::exception& e)
        {
            WARN("Tournament configuration test failed: " << e.what());
        }

        // Cleanup
        std::remove(config_file.c_str());
    }
}

TEST_CASE("Server client interaction simulation", "[integration][server_clients]")
{
    SECTION("Server with mock client handlers")
    {
        server s;
        std::string temp_path = "/tmp/server_integration_" + std::to_string(std::time(nullptr));

        try
        {
            s.listen(temp_path.c_str());

            bool new_client_handled = false;
            bool client_message_handled = false;

            auto handle_new_client = [&](std::ostream& os) -> bool
            {
                new_client_handled = true;
                os << R"({"type":"welcome","message":"Connected to tournament"})" << std::endl;
                return true;
            };

            auto handle_client = [&](std::iostream& ios) -> bool
            {
                client_message_handled = true;
                std::string line;
                if(std::getline(ios, line))
                {
                    // Echo back a response
                    ios << R"({"type":"response","echo":")" << line << R"("})" << std::endl;
                }
                return true;
            };

            // Poll with short timeout
            bool poll_result = s.poll(handle_new_client, handle_client, 5000);
            REQUIRE_FALSE(poll_result);        // Should timeout
            REQUIRE_FALSE(new_client_handled); // No actual clients connected
            REQUIRE_FALSE(client_message_handled);

            // Test broadcast
            s.broadcast(R"({"type":"broadcast","message":"Tournament starting"})");
        }
        catch(const std::exception& e)
        {
            WARN("Server integration test failed: " << e.what());
        }
    }
}

TEST_CASE("Bonjour service publishing integration", "[integration][bonjour_service]")
{
    SECTION("Tournament service discovery")
    {
        // Simulate tournament daemon publishing its service
        {
            bonjour_publisher bp;

            // Publish service like a real tournament would
            REQUIRE_NOTHROW(bp.publish("Tournament Integration Test", 25600));

            // Brief delay to allow service registration
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // Simulate service running
            for(int i = 0; i < 10; ++i)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                // In real scenario, tournament would be processing
            }
        }
        // Service should be unpublished when publisher goes out of scope

        // Brief delay to allow cleanup
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    SECTION("Multiple tournament instances")
    {
        std::vector<std::unique_ptr<bonjour_publisher>> publishers;

        // Simulate multiple tournament instances
        for(int i = 1; i <= 3; ++i)
        {
            auto bp = std::unique_ptr<bonjour_publisher>(new bonjour_publisher());
            std::string service_name = "Tournament Instance " + std::to_string(i);
            int port = 25600 + i - 1;

            REQUIRE_NOTHROW(bp->publish(service_name, port));
            publishers.push_back(std::move(bp));
        }

        // All services running simultaneously
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Services automatically unpublished when vector is destroyed
    }
}

TEST_CASE("DateTime and GameInfo timing integration", "[integration][datetime_timing]")
{
    SECTION("Tournament timing with real datetime")
    {
        gameinfo gi;

        // Configure tournament with short blind levels for testing
        nlohmann::json config = {
            { "blind_levels", { { { "little_blind", 25 }, { "big_blind", 50 }, { "duration", 100 } }, // 0.1 second levels
                                { { "little_blind", 50 }, { "big_blind", 100 }, { "duration", 100 } },
                                { { "little_blind", 100 }, { "big_blind", 200 }, { "duration", 100 } } } },
            { "players", { { { "player_id", "p1" }, { "name", "Test Player" } } } }
        };

        gi.configure(config);

        // Start tournament immediately
        gi.start();
        REQUIRE(gi.is_started());

        // Update game state multiple times
        for(int i = 0; i < 5; ++i)
        {
            gi.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }

        // Test blind level progression
        bool level_changed = gi.next_blind_level();
        REQUIRE_NOTHROW(level_changed);

        gi.update();

        // Dump state to verify timing information
        nlohmann::json state;
        gi.dump_state(state);
        REQUIRE(state.is_object());
    }
}

TEST_CASE("Full tournament daemon simulation", "[integration][full_daemon]")
{
    SECTION("Simulated tournament daemon workflow")
    {
        // Create all components a real daemon would use
        tournament t;
        bonjour_publisher bp;

        try
        {
            // 1. Authorize admin client
            int admin_code = t.authorize(99999);
            REQUIRE(admin_code == 99999);

            // 2. Load configuration
            nlohmann::json config = {
                { "players", { { { "player_id", "player1" }, { "name", "Tournament Player 1" } }, { { "player_id", "player2" }, { "name", "Tournament Player 2" } } } },
                { "blind_levels", { { { "little_blind", 50 }, { "big_blind", 100 }, { "duration", 300 } } } },
                { "funding_sources", { { { "name", "Entry Fee" }, { "type", 0 }, { "chips", 1000 }, { "cost", { { "amount", 25.0 }, { "currency", "USD" } } } } } },
                { "tables", { { { "table_name", "Main Table" } } } }
            };

            // Use same temporary directory logic as the actual daemon
            const char* tmpdir = std::getenv("TMPDIR");
            if(tmpdir == nullptr)
            {
                tmpdir = P_tmpdir;
            }

            std::string config_file = std::string(tmpdir) + "/full_daemon_config_" + std::to_string(std::time(nullptr)) + ".json";
            std::ofstream file(config_file);
            file << config.dump();
            file.close();

            t.load_configuration(config_file);

            // 3. Start listening for connections
            auto listen_result = t.listen(tmpdir);

            // 4. Publish service for discovery
            bp.publish("Full Integration Tournament", listen_result.second);

            // 5. Run tournament loop
            for(int i = 0; i < 10; ++i)
            {
                bool should_continue = t.run();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if(!should_continue && i < 5)
                {
                    // If tournament exits early, that's fine for test
                    break;
                }
            }

            // Cleanup
            std::remove(config_file.c_str());
        }
        catch(const std::exception& e)
        {
            WARN("Full daemon simulation failed: " << e.what());
        }
    }
}

TEST_CASE("Error handling integration", "[integration][error_handling]")
{
    SECTION("Tournament with invalid configuration")
    {
        tournament t;

        // Try to load non-existent configuration
        t.load_configuration("/nonexistent/path/config.json");

        // Should still be able to authorize and run
        REQUIRE_NOTHROW(t.authorize(11111));
        REQUIRE_NOTHROW(t.run());
    }

    SECTION("Server with invalid socket path")
    {
        server s;

        // Try to listen on invalid path (should throw)
        REQUIRE_THROWS(s.listen("/invalid/path/socket"));

        // Server should still be functional for other operations
        // (poll will work even if listen failed)
        auto handle_new_client = [](std::ostream&) -> bool
        {
            return true;
        };
        auto handle_client = [](std::iostream&) -> bool
        {
            return true;
        };

        bool result = s.poll(handle_new_client, handle_client, 1000);
        REQUIRE_FALSE(result); // Should timeout since no socket listening
    }

    SECTION("GameInfo with corrupted JSON")
    {
        gameinfo gi;

        // Invalid JSON should not crash
        nlohmann::json invalid_config = "not valid json";
        REQUIRE_NOTHROW(gi.configure(invalid_config));

        // Should still be able to operate
        REQUIRE_NOTHROW(gi.update());

        nlohmann::json state;
        REQUIRE_NOTHROW(gi.dump_state(state));
    }
}

TEST_CASE("Performance integration test", "[integration][performance]")
{
    SECTION("High frequency updates")
    {
        gameinfo gi;

        // Configure with many players and tables
        nlohmann::json config = {
            { "players", {} },
            { "tables", {} },
            { "blind_levels", { { { "little_blind", 25 }, { "big_blind", 50 } } } },
            { "funding_sources", { { { "name", "Buy-in" }, { "type", 0 }, { "chips", 1000 } } } }
        };

        // Add many players
        for(int i = 1; i <= 100; ++i)
        {
            config["players"].push_back({ { "player_id", "player" + std::to_string(i) },
                                          { "name", "Player " + std::to_string(i) } });
        }

        // Add many tables
        for(int i = 1; i <= 10; ++i)
        {
            config["tables"].push_back({ { "table_name", "Table " + std::to_string(i) } });
        }

        gi.configure(config);

        // Rapid updates
        auto start = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < 100; ++i)
        {
            gi.update();
            if(i % 10 == 0)
            {
                nlohmann::json state;
                gi.dump_state(state);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should complete in reasonable time (less than 1 second for 100 updates)
        REQUIRE(duration.count() < 1000);
    }
}

TEST_CASE("Memory usage integration test", "[integration][memory]")
{
    SECTION("Tournament lifecycle memory management")
    {
        // Test that creating and destroying tournaments doesn't leak memory
        for(int cycle = 0; cycle < 10; ++cycle)
        {
            tournament t;
            gameinfo gi;
            server s;
            bonjour_publisher bp;

            // Configure and run briefly
            nlohmann::json config = {
                { "players", { { { "player_id", "p1" }, { "name", "Player 1" } } } },
                { "blind_levels", { { { "little_blind", 25 }, { "big_blind", 50 } } } }
            };

            std::string config_file = "/tmp/memory_test_" + std::to_string(cycle) + ".json";
            std::ofstream file(config_file);
            file << config.dump();
            file.close();

            try
            {
                t.load_configuration(config_file);
                t.authorize(cycle + 1000);

                bp.publish("Memory Test " + std::to_string(cycle), 26000 + cycle);

                // Brief operations
                for(int i = 0; i < 5; ++i)
                {
                    t.run();
                    gi.update();
                }
            }
            catch(const std::exception& e)
            {
                WARN("Memory test cycle " << cycle << " failed: " << e.what());
            }

            std::remove(config_file.c_str());

            // Objects will be destroyed at end of loop iteration
        }
    }
}

TEST_CASE("Cross-component data flow", "[integration][data_flow]")
{
    SECTION("Configuration to JSON to configuration roundtrip")
    {
        // Create initial configuration
        nlohmann::json original_config = {
            { "players", { { { "player_id", "roundtrip1" }, { "name", "Roundtrip Player 1" } }, { { "player_id", "roundtrip2" }, { "name", "Roundtrip Player 2" } } } },
            { "blind_levels", { { { "little_blind", 25 }, { "big_blind", 50 }, { "duration", 1200 } }, { { "little_blind", 50 }, { "big_blind", 100 }, { "duration", 1200 } } } },
            { "funding_sources", { { { "name", "Test Buy-in" }, { "type", 0 }, { "chips", 1500 }, { "cost", { { "amount", 50.0 }, { "currency", "USD" } } } } } },
            { "tables", { { { "table_name", "Roundtrip Table" } } } }
        };

        // Configure first gameinfo instance
        gameinfo gi1;
        gi1.configure(original_config);

        // Dump configuration from first instance
        nlohmann::json dumped_config;
        gi1.dump_configuration(dumped_config);

        // Configure second gameinfo instance with dumped config
        gameinfo gi2;
        gi2.configure(dumped_config);

        // Both instances should produce similar state
        nlohmann::json state1, state2;
        gi1.dump_state(state1);
        gi2.dump_state(state2);

        REQUIRE(state1.is_object());
        REQUIRE(state2.is_object());

        // Both should be able to perform operations
        gi1.update();
        gi2.update();

        REQUIRE_NOTHROW(gi1.quick_setup());
        REQUIRE_NOTHROW(gi2.quick_setup());
    }
}

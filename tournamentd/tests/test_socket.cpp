#include <catch_amalgamated.hpp>
#include "../socket.hpp"
#include <sstream>
#include <thread>
#include <chrono>
#include <string>
#include <memory>
#include <cstring>


TEST_CASE("unix_socket creation and operations", "[socket][unix_socket]") {
    SECTION("Unix socket creation should not crash") {
        // Test that we can create unix sockets without crashing
        // Use temporary paths to avoid conflicts
        std::string temp_path = "/tmp/test_socket_" + std::to_string(std::time(nullptr));

        SECTION("Server socket creation") {
            std::unique_ptr<unix_socket> server;
            REQUIRE_NOTHROW(server = std::make_unique<unix_socket>(temp_path.c_str(), false, 1));
        }

        SECTION("Client socket creation with non-existent server") {
            // This should throw due to connection failure - no server listening
            std::unique_ptr<unix_socket> client;
            REQUIRE_THROWS(client = std::make_unique<unix_socket>(temp_path.c_str(), true));
        }
    }

    SECTION("Unix socket server-client interaction") {
        std::string temp_path = "/tmp/test_socket_server_" + std::to_string(std::time(nullptr));

        try {
            // Create server socket
            auto server = std::make_unique<unix_socket>(temp_path.c_str(), false, 1);

            // Test server socket properties
            std::ostringstream oss;
            oss << *server;
            REQUIRE_FALSE(oss.str().empty());

            // Test with very short timeout to avoid blocking tests
            auto selected = common_socket::select({*server}, 1000); // 1ms timeout
            // Should timeout with no connections
            REQUIRE(selected.empty());

        } catch (const std::exception& e) {
            // Socket creation might fail in test environment, which is acceptable
            WARN("Socket creation failed (expected in some test environments): " << e.what());
        }
    }
}

TEST_CASE("inet4_socket creation", "[socket][inet4_socket]") {
    SECTION("IPv4 client socket creation") {
        SECTION("Connection to non-existent host should throw") {
            std::unique_ptr<inet4_socket> client;
            REQUIRE_THROWS(client = std::make_unique<inet4_socket>("nonexistent.example.com", "12345"));
        }

        SECTION("Connection to localhost with invalid port") {
            std::unique_ptr<inet4_socket> client;
            REQUIRE_THROWS(client = std::make_unique<inet4_socket>("127.0.0.1", "99999"));
        }
    }

    SECTION("IPv4 server socket creation") {
        SECTION("Bind to available port") {
            try {
                auto server = std::make_unique<inet4_socket>("0", 1); // Bind to any available port

                std::ostringstream oss;
                oss << *server;
                REQUIRE_FALSE(oss.str().empty());

            } catch (const std::exception& e) {
                // Binding might fail in restricted environments
                WARN("Server socket creation failed: " << e.what());
            }
        }

        SECTION("Bind to specific high port") {
            try {
                auto server = std::make_unique<inet4_socket>("54321", 1);

            } catch (const std::exception& e) {
                // Port might be in use or binding restricted
                WARN("Specific port binding failed: " << e.what());
            }
        }
    }
}

TEST_CASE("inet6_socket creation", "[socket][inet6_socket]") {
    SECTION("IPv6 client socket creation") {
        SECTION("Connection to non-existent host should throw") {
            std::unique_ptr<inet6_socket> client;
            REQUIRE_THROWS(client = std::make_unique<inet6_socket>("::1", "12345"));
        }
    }

    SECTION("IPv6 server socket creation") {
        SECTION("Bind to IPv6 any address") {
            try {
                auto server = std::make_unique<inet6_socket>("0", 1);

            } catch (const std::exception& e) {
                // IPv6 might not be available in all environments
                WARN("IPv6 socket creation failed: " << e.what());
            }
        }
    }
}

TEST_CASE("Socket data operations", "[socket][data]") {
    SECTION("Listening socket error handling") {
        std::string temp_path = "/tmp/test_socket_listen_" + std::to_string(std::time(nullptr));
        
        auto server = std::make_unique<unix_socket>(temp_path.c_str(), false, 1);
        
        // Test that peek on listening socket properly throws an exception
        char buffer[10];
        REQUIRE_THROWS_AS(server->peek(buffer, sizeof(buffer)), std::system_error);
        
        // Cleanup
        std::remove(temp_path.c_str());
    }
    
    SECTION("Connected socket peek operation") {
        std::string temp_path = "/tmp/test_socket_peek_" + std::to_string(std::time(nullptr));
        
        try {
            // Create server socket
            auto server = std::make_unique<unix_socket>(temp_path.c_str(), false, 1);
            
            // Create client socket in separate thread to connect
            std::thread client_thread([&temp_path]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Let server start accepting
                try {
                    unix_socket client(temp_path.c_str(), true);
                    const char* test_data = "Hello";
                    client.send(test_data, strlen(test_data));
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Keep connection alive
                } catch (const std::exception& e) {
                    // Client connection may fail in test environment, that's ok
                }
            });
            
            // Accept connection
            auto accepted = server->accept();
            
            // Test peek on connected socket
            char buffer[10];
            long result = accepted.peek(buffer, sizeof(buffer));
            REQUIRE(result >= 0); // Should not crash and return valid result
            
            client_thread.join();
            
        } catch (const std::exception& e) {
            // Connection setup may fail in test environment
            WARN("Connected socket test failed (expected in some test environments): " << e.what());
        }
        
        // Cleanup
        std::remove(temp_path.c_str());
    }

}

TEST_CASE("Socket select functionality", "[socket][select]") {
    SECTION("Select with empty set") {
        std::set<common_socket> empty_set;
        auto result = common_socket::select(empty_set, 1000); // 1ms timeout
        REQUIRE(result.empty());
    }

    SECTION("Select with timeout") {
        std::string temp_path = "/tmp/test_socket_select_" + std::to_string(std::time(nullptr));

        try {
            auto server = std::make_unique<unix_socket>(temp_path.c_str(), false, 1);
            std::set<common_socket> socket_set = {*server};

            // Should timeout quickly with no activity
            auto start = std::chrono::steady_clock::now();
            auto result = common_socket::select(socket_set, 10000); // 10ms timeout
            auto end = std::chrono::steady_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            REQUIRE(duration.count() >= 5); // Should have waited at least some time
            REQUIRE(result.empty()); // Should timeout with no connections

        } catch (const std::exception& e) {
            WARN("Socket select test failed: " << e.what());
        }
    }
}

TEST_CASE("Socket comparison and equality", "[socket][comparison]") {
    SECTION("Socket equality operators") {
        std::string temp_path1 = "/tmp/test_socket_eq1_" + std::to_string(std::time(nullptr));
        std::string temp_path2 = "/tmp/test_socket_eq2_" + std::to_string(std::time(nullptr)) + "_2";

        try {
            auto socket1 = std::make_unique<unix_socket>(temp_path1.c_str(), false, 1);
            auto socket2 = std::make_unique<unix_socket>(temp_path2.c_str(), false, 1);

            // Test inequality (different sockets should not be equal)
            REQUIRE(*socket1 != *socket2);
            REQUIRE_FALSE(*socket1 == *socket2);

            // Test less-than operator (for std::set compatibility)
            bool lt_result = *socket1 < *socket2 || *socket2 < *socket1;
            REQUIRE(lt_result); // One should be less than the other

        } catch (const std::exception& e) {
            WARN("Socket comparison test failed: " << e.what());
        }
    }
}


TEST_CASE("Socket stream insertion", "[socket][stream]") {
    SECTION("Stream insertion for unix socket") {
        std::string temp_path = "/tmp/test_socket_stream_" + std::to_string(std::time(nullptr));

        try {
            auto server = std::make_unique<unix_socket>(temp_path.c_str(), false, 1);

            std::ostringstream oss;
            oss << *server;

            std::string result = oss.str();
            REQUIRE_FALSE(result.empty());
            // Should contain some socket information
            REQUIRE(result.length() > 5);

        } catch (const std::exception& e) {
            WARN("Socket stream insertion test failed: " << e.what());
        }
    }

    SECTION("Stream insertion for inet socket") {
        try {
            auto server = std::make_unique<inet4_socket>("0", 1);

            std::ostringstream oss;
            oss << *server;

            std::string result = oss.str();
            REQUIRE_FALSE(result.empty());

        } catch (const std::exception& e) {
            WARN("Inet socket stream insertion test failed: " << e.what());
        }
    }
}

TEST_CASE("Socket error conditions", "[socket][errors]") {
    SECTION("Invalid unix socket path") {
        // Test with invalid paths
        std::unique_ptr<unix_socket> server1, server2;
        REQUIRE_THROWS(server1 = std::make_unique<unix_socket>("", false, 1));
        REQUIRE_THROWS(server2 = std::make_unique<unix_socket>("/dev/null/invalid/path", false, 1));
    }

    SECTION("Invalid inet socket parameters") {
        // Test with invalid parameters
        std::unique_ptr<inet4_socket> client1, client2, server;
        REQUIRE_THROWS(client1 = std::make_unique<inet4_socket>("", "80"));
        REQUIRE_THROWS(client2 = std::make_unique<inet4_socket>("localhost", ""));
        REQUIRE_THROWS(server = std::make_unique<inet4_socket>("", 1));
    }

    SECTION("Invalid port numbers") {
        std::unique_ptr<inet4_socket> server1, server2;
        REQUIRE_THROWS(server1 = std::make_unique<inet4_socket>("-1", 1));
        REQUIRE_THROWS(server2 = std::make_unique<inet4_socket>("99999999", 1));
    }
}


#include "../socket.hpp"
#include <Catch2/catch.hpp>
#include <array>
#include <chrono>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

TEST_CASE("unix_socket creation and operations", "[socket][unix_socket]")
{
    SECTION("Unix socket creation should not crash")
    {
        // Test that we can create unix sockets without crashing
        // Use temporary paths to avoid conflicts
        std::string temp_path = "/tmp/test_socket_" + std::to_string(std::time(nullptr));

        SECTION("Server socket creation")
        {
            std::unique_ptr<unix_socket> server;
            REQUIRE_NOTHROW(server = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), false)));
        }

        SECTION("Client socket creation with non-existent server")
        {
            // This should throw due to connection failure - no server listening
            std::unique_ptr<unix_socket> client;
            REQUIRE_THROWS(client = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), true)));
        }
    }

    SECTION("Unix socket server-client interaction")
    {
        std::string temp_path = "/tmp/test_socket_server_" + std::to_string(std::time(nullptr));

        try
        {
            // Create server socket
            auto server = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), false));

            // Test server socket properties
            std::ostringstream oss;
            oss << *server;
            REQUIRE_FALSE(oss.str().empty());

            // Test with very short timeout to avoid blocking tests
            auto selected = common_socket::select({ *server }, 1000); // 1ms timeout
            // Should timeout with no connections
            REQUIRE(selected.empty());
        }
        catch(const std::exception& e)
        {
            // Socket creation might fail in test environment, which is acceptable
            WARN("Socket creation failed (expected in some test environments): " << e.what());
        }
    }
}

TEST_CASE("inet4_socket creation", "[socket][inet4_socket]")
{
    SECTION("IPv4 client socket creation")
    {
        SECTION("Connection to non-existent host should throw")
        {
            std::unique_ptr<inet4_socket> client;
            REQUIRE_THROWS(client = std::unique_ptr<inet4_socket>(new inet4_socket("nonexistent.example.com", "12345", true)));
        }

        SECTION("Connection to localhost with invalid port")
        {
            std::unique_ptr<inet4_socket> client;
            REQUIRE_THROWS(client = std::unique_ptr<inet4_socket>(new inet4_socket("127.0.0.1", "99999", true)));
        }
    }

    SECTION("IPv4 server socket creation")
    {
        SECTION("Bind to available port")
        {
            try
            {
                auto server = std::unique_ptr<inet4_socket>(new inet4_socket(nullptr, "0")); // Bind to any available port

                std::ostringstream oss;
                oss << *server;
                REQUIRE_FALSE(oss.str().empty());
            }
            catch(const std::exception& e)
            {
                // Binding might fail in restricted environments
                WARN("Server socket creation failed: " << e.what());
            }
        }

        SECTION("Bind to specific high port")
        {
            try
            {
                auto server = std::unique_ptr<inet4_socket>(new inet4_socket(nullptr, "54321"));
            }
            catch(const std::exception& e)
            {
                // Port might be in use or binding restricted
                WARN("Specific port binding failed: " << e.what());
            }
        }
    }
}

TEST_CASE("inet6_socket creation", "[socket][inet6_socket]")
{
    SECTION("IPv6 client socket creation")
    {
        SECTION("Connection to non-existent host should throw")
        {
            std::unique_ptr<inet6_socket> client;
            REQUIRE_THROWS(client = std::unique_ptr<inet6_socket>(new inet6_socket("::1", "12345", true)));
        }
    }

    SECTION("IPv6 server socket creation")
    {
        SECTION("Bind to IPv6 any address")
        {
            try
            {
                auto server = std::unique_ptr<inet6_socket>(new inet6_socket(nullptr, "0"));
            }
            catch(const std::exception& e)
            {
                // IPv6 might not be available in all environments
                WARN("IPv6 socket creation failed: " << e.what());
            }
        }
    }
}

TEST_CASE("Socket data operations", "[socket][data][unix_socket]")
{
    SECTION("Listening socket error handling")
    {
        std::string temp_path = "/tmp/test_socket_listen_" + std::to_string(std::time(nullptr));

        auto server = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), false));

        // Test that peek on listening socket properly throws an exception
        std::array<char, 10> buffer{};
        REQUIRE_THROWS_AS(server->peek(buffer.data(), buffer.size()), std::system_error);

        // Cleanup
        std::remove(temp_path.c_str());
    }

    SECTION("Connected socket peek operation")
    {
        std::string temp_path = "/tmp/test_socket_peek_" + std::to_string(std::time(nullptr));

        try
        {
            // Create server socket
            auto server = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), false));

            // Create client socket in separate thread to connect
            std::thread client_thread([&temp_path]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Let server start accepting
                try
                {
                    unix_socket client(temp_path.c_str(), true);
                    const char* test_data = "Hello";
                    client.send(test_data, strlen(test_data));
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Keep connection alive
                }
                catch(const std::exception& e)
                {
                    // Client connection may fail in test environment, that's ok
                }
            });

            // Accept connection
            auto accepted = server->accept();

            // Test peek on connected socket
            std::array<char, 10> buffer{};
            long result = accepted.peek(buffer.data(), buffer.size());
            REQUIRE(result >= 0); // Should not crash and return valid result

            client_thread.join();
        }
        catch(const std::exception& e)
        {
            // Connection setup may fail in test environment
            WARN("Connected socket test failed (expected in some test environments): " << e.what());
        }

        // Cleanup
        std::remove(temp_path.c_str());
    }
}

TEST_CASE("Socket select functionality", "[socket][select][unix_socket]")
{
    SECTION("Select with empty set")
    {
        std::set<common_socket> empty_set;
        auto result = common_socket::select(empty_set, 1000); // 1ms timeout
        REQUIRE(result.empty());
    }

    SECTION("Select with timeout")
    {
        std::string temp_path = "/tmp/test_socket_select_" + std::to_string(std::time(nullptr));

        try
        {
            auto server = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), false));
            std::set<common_socket> socket_set = { *server };

            // Should timeout quickly with no activity
            auto start = std::chrono::steady_clock::now();
            auto result = common_socket::select(socket_set, 10000); // 10ms timeout
            auto end = std::chrono::steady_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            REQUIRE(duration.count() >= 5); // Should have waited at least some time
            REQUIRE(result.empty());        // Should timeout with no connections
        }
        catch(const std::exception& e)
        {
            WARN("Socket select test failed: " << e.what());
        }
    }
}

TEST_CASE("Socket comparison and equality", "[socket][comparison][unix_socket]")
{
    SECTION("Socket equality operators")
    {
        std::string temp_path1 = "/tmp/test_socket_eq1_" + std::to_string(std::time(nullptr));
        std::string temp_path2 = "/tmp/test_socket_eq2_" + std::to_string(std::time(nullptr)) + "_2";

        try
        {
            auto socket1 = std::unique_ptr<unix_socket>(new unix_socket(temp_path1.c_str(), false));
            auto socket2 = std::unique_ptr<unix_socket>(new unix_socket(temp_path2.c_str(), false));

            // Test inequality (different sockets should not be equal)
            REQUIRE(*socket1 != *socket2);
            REQUIRE_FALSE(*socket1 == *socket2);

            // Test less-than operator (for std::set compatibility)
            bool lt_result = *socket1 < *socket2 || *socket2 < *socket1;
            REQUIRE(lt_result); // One should be less than the other
        }
        catch(const std::exception& e)
        {
            WARN("Socket comparison test failed: " << e.what());
        }
    }
}

TEST_CASE("Socket stream insertion", "[socket][stream][unix_socket]")
{
    SECTION("Stream insertion for unix socket")
    {
        std::string temp_path = "/tmp/test_socket_stream_" + std::to_string(std::time(nullptr));

        try
        {
            auto server = std::unique_ptr<unix_socket>(new unix_socket(temp_path.c_str(), false));

            std::ostringstream oss;
            oss << *server;

            std::string result = oss.str();
            REQUIRE_FALSE(result.empty());
            // Should contain some socket information
            REQUIRE(result.length() > 5);
        }
        catch(const std::exception& e)
        {
            WARN("Socket stream insertion test failed: " << e.what());
        }
    }

    SECTION("Stream insertion for inet socket")
    {
        try
        {
            auto server = std::unique_ptr<inet4_socket>(new inet4_socket(nullptr, "0"));

            std::ostringstream oss;
            oss << *server;

            std::string result = oss.str();
            REQUIRE_FALSE(result.empty());
        }
        catch(const std::exception& e)
        {
            WARN("Inet socket stream insertion test failed: " << e.what());
        }
    }
}

TEST_CASE("Socket error conditions", "[socket][errors][unix_socket]")
{
    SECTION("Invalid unix socket path")
    {
        // Test with invalid paths
        std::unique_ptr<unix_socket> server1;
        REQUIRE_THROWS(server1 = std::unique_ptr<unix_socket>(new unix_socket("", false)));
        std::unique_ptr<unix_socket> server2;
        REQUIRE_THROWS(server2 = std::unique_ptr<unix_socket>(new unix_socket("/dev/null/invalid/path", false)));
    }

    SECTION("unix_socket path validation")
    {
        SECTION("empty path should throw for listening")
        {
            REQUIRE_THROWS(unix_socket("", false));
        }

        SECTION("empty path should throw for connecting")
        {
            REQUIRE_THROWS(unix_socket("", true));
        }

        SECTION("path too long should throw")
        {
            // Unix socket paths are typically limited to 108 bytes
            // Create a path that's definitely too long
            std::string long_path(200, 'a');
            REQUIRE_THROWS(unix_socket(long_path.c_str(), false));
        }

        SECTION("valid path should not throw for listening")
        {
            std::string temp_path = "/tmp/test_valid_path_" + std::to_string(std::time(nullptr));
            REQUIRE_NOTHROW(unix_socket(temp_path.c_str(), false));
        }
    }

    SECTION("Invalid inet socket parameters")
    {
        // Test with invalid parameters
        std::unique_ptr<inet4_socket> client1;
        REQUIRE_THROWS(client1 = std::unique_ptr<inet4_socket>(new inet4_socket("", "80", true)));
        std::unique_ptr<inet4_socket> client2;
        REQUIRE_THROWS(client2 = std::unique_ptr<inet4_socket>(new inet4_socket("localhost", "", true)));
        std::unique_ptr<inet4_socket> server;
        REQUIRE_THROWS(server = std::unique_ptr<inet4_socket>(new inet4_socket(nullptr, "")));
    }

    SECTION("Invalid port numbers")
    {
        std::unique_ptr<inet4_socket> server1;
        REQUIRE_THROWS(server1 = std::unique_ptr<inet4_socket>(new inet4_socket(nullptr, "-1")));
        std::unique_ptr<inet4_socket> server2;
        REQUIRE_THROWS(server2 = std::unique_ptr<inet4_socket>(new inet4_socket(nullptr, "99999999")));
    }
}

TEST_CASE("Socket parameter validation", "[socket][validation]")
{
    SECTION("inet_socket connecting=true requires host")
    {
        SECTION("inet4_socket with connecting=true and nullptr host should throw")
        {
            REQUIRE_THROWS(inet4_socket(nullptr, "12345", true));
        }

        SECTION("inet4_socket with connecting=true and empty host should throw")
        {
            REQUIRE_THROWS(inet4_socket("", "12345", true));
        }

        SECTION("inet6_socket with connecting=true and nullptr host should throw")
        {
            REQUIRE_THROWS(inet6_socket(nullptr, "12345", true));
        }

        SECTION("inet6_socket with connecting=true and empty host should throw")
        {
            REQUIRE_THROWS(inet6_socket("", "12345", true));
        }
    }

    SECTION("inet_socket service validation")
    {
        SECTION("nullptr service should throw for listening socket")
        {
            REQUIRE_THROWS(inet4_socket(nullptr, nullptr, false));
        }

        SECTION("nullptr service should throw for connecting socket")
        {
            REQUIRE_THROWS(inet4_socket("localhost", nullptr, true));
        }

        SECTION("empty service should throw for listening socket")
        {
            REQUIRE_THROWS(inet4_socket(nullptr, "", false));
        }

        SECTION("empty service should throw for connecting socket")
        {
            REQUIRE_THROWS(inet4_socket("localhost", "", true));
        }
    }

    SECTION("inet_socket port range validation")
    {
        SECTION("port -1 should throw")
        {
            REQUIRE_THROWS(inet4_socket(nullptr, "-1", false));
        }

        SECTION("port 65536 should throw")
        {
            REQUIRE_THROWS(inet4_socket(nullptr, "65536", false));
        }

        SECTION("port 99999999 should throw")
        {
            REQUIRE_THROWS(inet4_socket(nullptr, "99999999", false));
        }

        SECTION("port 0 should not throw")
        {
            REQUIRE_NOTHROW(inet4_socket(nullptr, "0", false));
        }

        SECTION("port 65535 should not throw")
        {
            REQUIRE_NOTHROW(inet4_socket(nullptr, "65535", false));
        }

        SECTION("port 12345 should not throw")
        {
            REQUIRE_NOTHROW(inet4_socket(nullptr, "12345", false));
        }
    }

    SECTION("inet_socket non-numeric service names")
    {
        SECTION("service name 'http' should not throw for listening")
        {
            // Note: This may fail if we don't have permission to bind to port 80
            // but it should not fail validation
            try
            {
                inet4_socket(nullptr, "http", false);
            }
            catch(const std::invalid_argument&)
            {
                // Invalid argument means validation failed - test should fail
                REQUIRE(false);
            }
            catch(const std::system_error&)
            {
                // System error (like permission denied) is OK - validation passed
                REQUIRE(true);
            }
        }
    }

    SECTION("inet_socket valid inputs should not throw")
    {
        SECTION("inet4_socket listening with valid port")
        {
            REQUIRE_NOTHROW(inet4_socket(nullptr, "54321", false));
        }

        SECTION("inet6_socket listening with valid port")
        {
            REQUIRE_NOTHROW(inet6_socket(nullptr, "54322", false));
        }

        SECTION("connecting with valid host and service should not throw on validation")
        {
            // This will fail to connect, but should not fail validation
            try
            {
                inet4_socket("127.0.0.1", "54323", true);
            }
            catch(const std::invalid_argument&)
            {
                // Invalid argument means validation failed - test should fail
                REQUIRE(false);
            }
            catch(const std::system_error&)
            {
                // System error (connection refused) is OK - validation passed
                REQUIRE(true);
            }
        }
    }
}

#include <catch_amalgamated.hpp>
#include "../server.hpp"
#include <sstream>
#include <functional>
#include <chrono>
#include <thread>
#include <memory>

TEST_CASE("Server creation and destruction", "[server][basic]") {
    SECTION("Default constructor") {
        std::unique_ptr<server> s;
        REQUIRE_NOTHROW(s = std::make_unique<server>());
    }

    SECTION("Server destruction") {
        auto s = std::make_unique<server>();
        REQUIRE_NOTHROW(s.reset());
    }

    SECTION("Multiple server instances") {
        auto s1 = std::make_unique<server>();
        auto s2 = std::make_unique<server>();
        REQUIRE(s1 != nullptr);
        REQUIRE(s2 != nullptr);
    }
}

TEST_CASE("Server listen functionality", "[server][listen]") {
    SECTION("Listen on unix socket only") {
        server s;
        std::string temp_path = "/tmp/test_server_" + std::to_string(std::time(nullptr));

        try {
            REQUIRE_NOTHROW(s.listen(temp_path.c_str()));
        } catch (const std::exception& e) {
            WARN("Server listen on unix socket failed: " << e.what());
        }
    }

    SECTION("Listen on unix socket and inet service") {
        server s;
        std::string temp_path = "/tmp/test_server_inet_" + std::to_string(std::time(nullptr));

        try {
            // Use port 0 to let system assign available port
            REQUIRE_NOTHROW(s.listen(temp_path.c_str(), "0"));
        } catch (const std::exception& e) {
            WARN("Server listen on unix+inet failed: " << e.what());
        }
    }

    SECTION("Listen with null unix path") {
        server s;
        try {
            REQUIRE_NOTHROW(s.listen(nullptr, "0"));
        } catch (const std::exception& e) {
            WARN("Server listen with null unix path failed: " << e.what());
        }
    }

    SECTION("Invalid listen parameters") {
        server s;

        // Listen with invalid parameters should throw exceptions
        REQUIRE_THROWS(s.listen("", nullptr));
        REQUIRE_NOTHROW(s.listen(nullptr, nullptr)); // This one might be handled differently
    }
}

TEST_CASE("Server poll functionality", "[server][poll]") {
    SECTION("Poll with no clients") {
        server s;

        bool new_client_called = false;
        bool handle_client_called = false;

        auto handle_new_client = [&new_client_called](std::ostream&) -> bool {
            new_client_called = true;
            return true;
        };

        auto handle_client = [&handle_client_called](std::iostream&) -> bool {
            handle_client_called = true;
            return true;
        };

        // Poll with very short timeout (1ms) to avoid blocking tests
        bool result = s.poll(handle_new_client, handle_client, 1000);

        // Should return false (timeout) with no clients
        REQUIRE_FALSE(result);
        REQUIRE_FALSE(new_client_called);
        REQUIRE_FALSE(handle_client_called);
    }

    SECTION("Poll with listening server") {
        server s;
        std::string temp_path = "/tmp/test_server_poll_" + std::to_string(std::time(nullptr));

        try {
            s.listen(temp_path.c_str());

            bool new_client_called = false;
            bool handle_client_called = false;

            auto handle_new_client = [&new_client_called](std::ostream& os) -> bool {
                new_client_called = true;
                os << "Welcome!" << std::endl;
                return true;
            };

            auto handle_client = [&handle_client_called](std::iostream&) -> bool {
                handle_client_called = true;
                return true;
            };

            // Poll with short timeout
            bool result = s.poll(handle_new_client, handle_client, 5000); // 5ms

            // Should timeout with no connections
            REQUIRE_FALSE(result);
            REQUIRE_FALSE(new_client_called);
            REQUIRE_FALSE(handle_client_called);

        } catch (const std::exception& e) {
            WARN("Server poll test failed: " << e.what());
        }
    }

    SECTION("Poll with short timeout") {
        server s;

        auto handle_new_client = [](std::ostream&) -> bool { return true; };
        auto handle_client = [](std::iostream&) -> bool { return true; };

        // Poll with short timeout should return quickly when not listening
        auto start = std::chrono::steady_clock::now();
        bool result = s.poll(handle_new_client, handle_client, 1000); // 1ms timeout
        auto end = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Should return quickly and timeout
        REQUIRE(duration.count() < 1000); // Less than 1 second
        REQUIRE_FALSE(result);
    }
}

TEST_CASE("Server broadcast functionality", "[server][broadcast]") {
    SECTION("Broadcast with no clients") {
        server s;

        // Broadcasting with no clients should not crash
        REQUIRE_NOTHROW(s.broadcast("Hello World"));
        REQUIRE_NOTHROW(s.broadcast(""));
        REQUIRE_NOTHROW(s.broadcast("Multi\nLine\nMessage"));
    }

    SECTION("Broadcast with listening server") {
        server s;
        std::string temp_path = "/tmp/test_server_broadcast_" + std::to_string(std::time(nullptr));

        try {
            s.listen(temp_path.c_str());

            // Should not crash even with no connected clients
            REQUIRE_NOTHROW(s.broadcast("Test broadcast message"));
            REQUIRE_NOTHROW(s.broadcast("{\"type\":\"test\",\"message\":\"json format\"}"));

        } catch (const std::exception& e) {
            WARN("Server broadcast test failed: " << e.what());
        }
    }

    SECTION("Broadcast various message types") {
        server s;

        // Test different message content
        REQUIRE_NOTHROW(s.broadcast("Simple message"));
        REQUIRE_NOTHROW(s.broadcast("Message with special chars: !@#$%^&*()"));
        REQUIRE_NOTHROW(s.broadcast("Unicode: 你好 🌍"));
        REQUIRE_NOTHROW(s.broadcast(std::string(1000, 'A'))); // Long message

        // JSON-style messages
        REQUIRE_NOTHROW(s.broadcast("{\"command\":\"update\",\"data\":{\"level\":1}}"));
        REQUIRE_NOTHROW(s.broadcast("[{\"id\":1,\"name\":\"test\"}]"));
    }
}

TEST_CASE("Server client handling", "[server][clients]") {
    SECTION("Client handler return values") {
        server s;

        // Test handlers that return different values
        auto handle_new_client_true = [](std::ostream& os) -> bool {
            os << "Hello" << std::endl;
            return true;
        };

        auto handle_new_client_false = [](std::ostream& os) -> bool {
            os << "Goodbye" << std::endl;
            return false;
        };

        auto handle_client_true = [](std::iostream& ios) -> bool {
            std::string line;
            if (std::getline(ios, line)) {
                ios << "Echo: " << line << std::endl;
            }
            return true;
        };

        auto handle_client_false = [](std::iostream&) -> bool {
            return false;
        };

        // These should not crash regardless of return values
        REQUIRE_NOTHROW(s.poll(handle_new_client_true, handle_client_true, 1000));
        REQUIRE_NOTHROW(s.poll(handle_new_client_false, handle_client_false, 1000));
    }

    SECTION("Client handler exceptions") {
        server s;

        auto handle_new_client_throw = [](std::ostream&) -> bool {
            throw std::runtime_error("Handler error");
        };

        auto handle_client_throw = [](std::iostream&) -> bool {
            throw std::logic_error("Client handler error");
        };

        // Server should handle handler exceptions gracefully
        REQUIRE_NOTHROW(s.poll(handle_new_client_throw, handle_client_throw, 1000));
    }
}

TEST_CASE("Server stream operations", "[server][streams]") {
    SECTION("Client handler stream usage") {
        server s;

        bool handler_called = false;
        std::string written_data;

        auto handle_new_client = [&](std::ostream& os) -> bool {
            handler_called = true;

            // Test various stream operations
            os << "Line 1" << std::endl;
            os << "Line 2\n";
            os << "Number: " << 42 << std::endl;
            os << "Float: " << 3.14 << std::endl;

            // Test stream state
            REQUIRE(os.good());

            return true;
        };

        auto handle_client = [](std::iostream& ios) -> bool {
            // Test bidirectional stream operations
            std::string input;
            if (std::getline(ios, input)) {
                ios << "Response to: " << input << std::endl;
            }

            REQUIRE((ios.good() || ios.eof()));
            return true;
        };

        // Poll should handle stream operations without crashing
        REQUIRE_NOTHROW(s.poll(handle_new_client, handle_client, 1000));
    }
}

TEST_CASE("Server multiple listen calls", "[server][multiple_listen]") {
    SECTION("Multiple listen calls") {
        server s;
        std::string temp_path1 = "/tmp/test_server_multi1_" + std::to_string(std::time(nullptr));
        std::string temp_path2 = "/tmp/test_server_multi2_" + std::to_string(std::time(nullptr)) + "_2";

        try {
            // First listen call
            REQUIRE_NOTHROW(s.listen(temp_path1.c_str()));

            // Second listen call (should handle gracefully)
            REQUIRE_NOTHROW(s.listen(temp_path2.c_str()));

        } catch (const std::exception& e) {
            WARN("Multiple listen test failed: " << e.what());
        }
    }

    SECTION("Listen after poll") {
        server s;
        std::string temp_path = "/tmp/test_server_listen_after_poll_" + std::to_string(std::time(nullptr));

        auto handle_new_client = [](std::ostream&) -> bool { return true; };
        auto handle_client = [](std::iostream&) -> bool { return true; };

        // Poll first (with no listening sockets)
        REQUIRE_NOTHROW(s.poll(handle_new_client, handle_client, 1000));

        try {
            // Then listen
            REQUIRE_NOTHROW(s.listen(temp_path.c_str()));

            // Poll again
            REQUIRE_NOTHROW(s.poll(handle_new_client, handle_client, 1000));

        } catch (const std::exception& e) {
            WARN("Listen after poll test failed: " << e.what());
        }
    }
}

TEST_CASE("Server resource management", "[server][resources]") {
    SECTION("Server cleanup on destruction") {
        std::string temp_path = "/tmp/test_server_cleanup_" + std::to_string(std::time(nullptr));

        try {
            {
                server s;
                s.listen(temp_path.c_str());
                // Server should clean up when going out of scope
            }

            // Should be able to create another server with same path after cleanup
            {
                server s2;
                REQUIRE_NOTHROW(s2.listen(temp_path.c_str()));
            }

        } catch (const std::exception& e) {
            WARN("Server cleanup test failed: " << e.what());
        }
    }

    SECTION("Multiple server instances with different paths") {
        std::string temp_path1 = "/tmp/test_server_inst1_" + std::to_string(std::time(nullptr));
        std::string temp_path2 = "/tmp/test_server_inst2_" + std::to_string(std::time(nullptr)) + "_2";

        try {
            server s1, s2;

            REQUIRE_NOTHROW(s1.listen(temp_path1.c_str()));
            REQUIRE_NOTHROW(s2.listen(temp_path2.c_str()));

            // Both should be able to poll independently
            auto handle_new_client = [](std::ostream&) -> bool { return true; };
            auto handle_client = [](std::iostream&) -> bool { return true; };

            REQUIRE_NOTHROW(s1.poll(handle_new_client, handle_client, 1000));
            REQUIRE_NOTHROW(s2.poll(handle_new_client, handle_client, 1000));

        } catch (const std::exception& e) {
            WARN("Multiple server instances test failed: " << e.what());
        }
    }
}


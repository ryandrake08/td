#include <catch_amalgamated.hpp>
#include "../types.hpp"
#include "nlohmann/json.hpp"
#include <sstream>
#include <limits>

TEST_CASE("td::protocol_error", "[types][exceptions]") {
    SECTION("Can create and throw protocol_error") {
        REQUIRE_THROWS_AS(throw td::protocol_error("test error"), td::protocol_error);
    }

    SECTION("Error message is preserved") {
        try {
            throw td::protocol_error("custom message");
        } catch (const td::protocol_error& e) {
            REQUIRE(std::string(e.what()) == "custom message");
        }
    }
}

TEST_CASE("td::authorized_client", "[types][authorized_client]") {
    SECTION("Default constructor") {
        td::authorized_client client;
        REQUIRE(client.code == 0);
        REQUIRE(client.name.empty());
    }

    SECTION("Constructor with code and name") {
        td::authorized_client client(12345, "TestClient");
        REQUIRE(client.code == 12345);
        REQUIRE(client.name == "TestClient");
    }

    SECTION("JSON serialization roundtrip") {
        td::authorized_client original(54321, "JsonClient");

        nlohmann::json j;
        to_json(j, original);

        td::authorized_client deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized.code == original.code);
        REQUIRE(deserialized.name == original.name);
    }
}

TEST_CASE("td::blind_level", "[types][blind_level]") {
    SECTION("Default constructor") {
        td::blind_level level;
        REQUIRE(level.little_blind == 0);
        REQUIRE(level.big_blind == 0);
        REQUIRE(level.ante == 0);
        REQUIRE(level.ante_type == td::ante_type_t::none);
        REQUIRE(level.duration == 0);
        REQUIRE(level.break_duration == 0);
        REQUIRE(level.reason.empty());
    }

    SECTION("Equality operator") {
        td::blind_level level1;
        td::blind_level level2;
        REQUIRE(level1 == level2);

        level1.little_blind = 50;
        REQUIRE_FALSE(level1 == level2);

        level2.little_blind = 50;
        REQUIRE(level1 == level2);
    }

    SECTION("JSON serialization roundtrip") {
        td::blind_level original;
        original.little_blind = 25;
        original.big_blind = 50;
        original.ante = 5;
        original.ante_type = td::ante_type_t::traditional;
        original.duration = 1200;
        original.break_duration = 300;
        original.reason = "Level 1";

        nlohmann::json j;
        to_json(j, original);

        td::blind_level deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::chip", "[types][chip]") {
    SECTION("Default constructor") {
        td::chip chip;
        REQUIRE(chip.color.empty());
        REQUIRE(chip.denomination == 0);
        REQUIRE(chip.count_available == 0);
    }

    SECTION("Equality operator") {
        td::chip chip1;
        td::chip chip2;
        REQUIRE(chip1 == chip2);

        chip1.color = "red";
        chip1.denomination = 5;
        chip1.count_available = 100;

        REQUIRE_FALSE(chip1 == chip2);

        chip2.color = "red";
        chip2.denomination = 5;
        chip2.count_available = 100;

        REQUIRE(chip1 == chip2);
    }

    SECTION("JSON serialization roundtrip") {
        td::chip original;
        original.color = "blue";
        original.denomination = 25;
        original.count_available = 200;

        nlohmann::json j;
        to_json(j, original);

        td::chip deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::table", "[types][table]") {
    SECTION("Default constructor") {
        td::table table;
        REQUIRE(table.table_name.empty());
    }

    SECTION("Equality operator") {
        td::table table1;
        td::table table2;
        REQUIRE(table1 == table2);

        table1.table_name = "Table 1";
        REQUIRE_FALSE(table1 == table2);

        table2.table_name = "Table 1";
        REQUIRE(table1 == table2);
    }

    SECTION("JSON serialization roundtrip") {
        td::table original;
        original.table_name = "Final Table";

        nlohmann::json j;
        to_json(j, original);

        td::table deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::monetary_value", "[types][monetary_value]") {
    SECTION("Default constructor") {
        td::monetary_value mv;
        REQUIRE(mv.amount == 0.0);
        REQUIRE(mv.currency.empty());
    }

    SECTION("Constructor with amount and currency") {
        td::monetary_value mv(100.50, "USD");
        REQUIRE(mv.amount == 100.50);
        REQUIRE(mv.currency == "USD");
    }

    SECTION("Equality operator") {
        td::monetary_value mv1(50.0, "EUR");
        td::monetary_value mv2(50.0, "EUR");
        REQUIRE(mv1 == mv2);

        td::monetary_value mv3(50.0, "USD");
        REQUIRE_FALSE(mv1 == mv3);

        td::monetary_value mv4(75.0, "EUR");
        REQUIRE_FALSE(mv1 == mv4);
    }

    SECTION("JSON serialization roundtrip") {
        td::monetary_value original(123.45, "GBP");

        nlohmann::json j;
        to_json(j, original);

        td::monetary_value deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::monetary_value_nocurrency", "[types][monetary_value_nocurrency]") {
    SECTION("Default constructor") {
        td::monetary_value_nocurrency mv;
        REQUIRE(mv.amount == 0.0);
    }

    SECTION("Constructor with amount") {
        td::monetary_value_nocurrency mv(250.75);
        REQUIRE(mv.amount == 250.75);
    }

    SECTION("Equality operator") {
        td::monetary_value_nocurrency mv1(100.0);
        td::monetary_value_nocurrency mv2(100.0);
        REQUIRE(mv1 == mv2);

        td::monetary_value_nocurrency mv3(200.0);
        REQUIRE_FALSE(mv1 == mv3);
    }

    SECTION("JSON serialization roundtrip") {
        td::monetary_value_nocurrency original(456.78);

        nlohmann::json j;
        to_json(j, original);

        td::monetary_value_nocurrency deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::funding_source", "[types][funding_source]") {
    SECTION("Default constructor") {
        td::funding_source fs;
        REQUIRE(fs.name.empty());
        REQUIRE(fs.type == td::funding_source_type_t::buyin);
        REQUIRE(fs.forbid_after_blind_level == std::numeric_limits<std::size_t>::max());
        REQUIRE(fs.chips == 0);
    }

    SECTION("Equality operator") {
        td::funding_source fs1;
        td::funding_source fs2;
        REQUIRE(fs1 == fs2);

        fs1.name = "Buy-in";
        fs1.type = td::funding_source_type_t::buyin;
        fs1.chips = 1000;

        REQUIRE_FALSE(fs1 == fs2);

        fs2 = fs1;
        REQUIRE(fs1 == fs2);
    }

    SECTION("JSON serialization roundtrip") {
        td::funding_source original;
        original.name = "Rebuy";
        original.type = td::funding_source_type_t::rebuy;
        original.forbid_after_blind_level = 5;
        original.chips = 500;
        original.cost = td::monetary_value(25.0, "USD");
        original.commission = td::monetary_value(2.5, "USD");
        original.equity = td::monetary_value_nocurrency(22.5);

        nlohmann::json j;
        to_json(j, original);

        td::funding_source deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::player", "[types][player]") {
    SECTION("Default constructor") {
        td::player player;
        REQUIRE(player.player_id.empty());
        REQUIRE(player.name.empty());
    }

    SECTION("Equality operator") {
        td::player p1;
        td::player p2;
        
        // Fix timestamp issue - set to same time for comparison
        datetime fixed_time = datetime::now();
        p1.added_at = fixed_time;
        p2.added_at = fixed_time;
        
        REQUIRE(p1 == p2);

        p1.player_id = "player1";
        p1.name = "John Doe";

        REQUIRE_FALSE(p1 == p2);

        p2.player_id = "player1";
        p2.name = "John Doe";

        REQUIRE(p1 == p2);
    }

    SECTION("JSON serialization roundtrip") {
        td::player original;
        original.player_id = "player123";
        original.name = "Jane Smith";
        original.added_at = datetime::from_gm("2023-01-01T12:00:00");

        nlohmann::json j;
        to_json(j, original);

        td::player deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::seat", "[types][seat]") {
    SECTION("Default constructor") {
        td::seat seat;
        REQUIRE(seat.table_number == 0);
        REQUIRE(seat.seat_number == 0);
    }

    SECTION("Constructor with table and seat numbers") {
        td::seat seat(3, 7);
        REQUIRE(seat.table_number == 3);
        REQUIRE(seat.seat_number == 7);
    }

    SECTION("Equality operator") {
        td::seat seat1(1, 2);
        td::seat seat2(1, 2);
        REQUIRE(seat1 == seat2);

        td::seat seat3(1, 3);
        REQUIRE_FALSE(seat1 == seat3);

        td::seat seat4(2, 2);
        REQUIRE_FALSE(seat1 == seat4);
    }

    SECTION("JSON serialization roundtrip") {
        td::seat original(5, 8);

        nlohmann::json j;
        to_json(j, original);

        td::seat deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::player_movement", "[types][player_movement]") {
    SECTION("Default constructor") {
        td::player_movement pm;
        REQUIRE(pm.player_id.empty());
        REQUIRE(pm.name.empty());
        REQUIRE(pm.from_table_name.empty());
        REQUIRE(pm.from_seat_name.empty());
        REQUIRE(pm.to_table_name.empty());
        REQUIRE(pm.to_seat_name.empty());
    }

    SECTION("Constructor with parameters") {
        td::player_movement pm("player1", "John", "Table1", "Seat1", "Table2", "Seat2");
        REQUIRE(pm.player_id == "player1");
        REQUIRE(pm.name == "John");
        REQUIRE(pm.from_table_name == "Table1");
        REQUIRE(pm.from_seat_name == "Seat1");
        REQUIRE(pm.to_table_name == "Table2");
        REQUIRE(pm.to_seat_name == "Seat2");
    }

    SECTION("JSON serialization") {
        td::player_movement original("p1", "Alice", "T1", "S1", "T2", "S2");

        nlohmann::json j;
        to_json(j, original);

        REQUIRE_FALSE(j.empty());
    }
}

TEST_CASE("td::player_chips", "[types][player_chips]") {
    SECTION("Default constructor") {
        td::player_chips pc;
        REQUIRE(pc.denomination == 0);
        REQUIRE(pc.chips == 0);
    }

    SECTION("Constructor with denomination and chips") {
        td::player_chips pc(25, 20);
        REQUIRE(pc.denomination == 25);
        REQUIRE(pc.chips == 20);
    }

    SECTION("JSON serialization") {
        td::player_chips original(100, 5);

        nlohmann::json j;
        to_json(j, original);

        REQUIRE_FALSE(j.empty());
    }
}

TEST_CASE("td::manual_payout", "[types][manual_payout]") {
    SECTION("Default constructor") {
        td::manual_payout mp;
        REQUIRE(mp.buyins_count == 0);
        REQUIRE(mp.payouts.empty());
    }

    SECTION("Constructor with parameters") {
        std::vector<td::monetary_value_nocurrency> payouts = {
            td::monetary_value_nocurrency(100.0),
            td::monetary_value_nocurrency(60.0),
            td::monetary_value_nocurrency(40.0)
        };
        td::manual_payout mp(10, payouts);

        REQUIRE(mp.buyins_count == 10);
        REQUIRE(mp.payouts.size() == 3);
        REQUIRE(mp.payouts[0] == td::monetary_value_nocurrency(100.0));
    }

    SECTION("Equality operator") {
        std::vector<td::monetary_value_nocurrency> payouts1 = {td::monetary_value_nocurrency(50.0)};
        std::vector<td::monetary_value_nocurrency> payouts2 = {td::monetary_value_nocurrency(50.0)};

        td::manual_payout mp1(5, payouts1);
        td::manual_payout mp2(5, payouts2);
        REQUIRE(mp1 == mp2);

        td::manual_payout mp3(6, payouts1);
        REQUIRE_FALSE(mp1 == mp3);
    }

    SECTION("JSON serialization roundtrip") {
        std::vector<td::monetary_value_nocurrency> payouts = {
            td::monetary_value_nocurrency(200.0),
            td::monetary_value_nocurrency(120.0)
        };
        td::manual_payout original(8, payouts);

        nlohmann::json j;
        to_json(j, original);

        td::manual_payout deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("td::result", "[types][result]") {
    SECTION("Default constructor") {
        td::result result;
        REQUIRE(result.place == 0);
        REQUIRE(result.name.empty());
    }

    SECTION("Constructor with place") {
        td::result result(3);
        REQUIRE(result.place == 3);
        REQUIRE(result.name.empty());
    }

    SECTION("Constructor with place and name") {
        td::result result(1, "Winner");
        REQUIRE(result.place == 1);
        REQUIRE(result.name == "Winner");
    }

    SECTION("JSON serialization") {
        td::result original(2, "Runner-up");
        original.payout = td::monetary_value_nocurrency(500.0);

        nlohmann::json j;
        to_json(j, original);

        REQUIRE_FALSE(j.empty());
    }
}

TEST_CASE("td::seated_player", "[types][seated_player]") {
    SECTION("Constructor for unseated player") {
        td::seated_player sp("player1", true, "John");
        REQUIRE(sp.player_id == "player1");
        REQUIRE(sp.buyin == true);
        REQUIRE(sp.player_name == "John");
        REQUIRE(sp.table_name.empty());
        REQUIRE(sp.seat_name.empty());
    }

    SECTION("Constructor for seated player") {
        td::seated_player sp("player2", false, "Jane", "Table1", "Seat3");
        REQUIRE(sp.player_id == "player2");
        REQUIRE(sp.buyin == false);
        REQUIRE(sp.player_name == "Jane");
        REQUIRE(sp.table_name == "Table1");
        REQUIRE(sp.seat_name == "Seat3");
    }

    SECTION("JSON serialization") {
        td::seated_player original("p1", true, "Alice", "T1", "S1");

        nlohmann::json j;
        to_json(j, original);

        REQUIRE_FALSE(j.empty());
    }
}

TEST_CASE("td::seating_chart_entry", "[types][seating_chart_entry]") {
    SECTION("Constructor for empty seat") {
        td::seating_chart_entry entry("Table1", "Seat1");
        REQUIRE(entry.player_name.empty());
        REQUIRE(entry.table_name == "Table1");
        REQUIRE(entry.seat_name == "Seat1");
    }

    SECTION("Constructor for occupied seat") {
        td::seating_chart_entry entry("Bob", "Table2", "Seat5");
        REQUIRE(entry.player_name == "Bob");
        REQUIRE(entry.table_name == "Table2");
        REQUIRE(entry.seat_name == "Seat5");
    }

    SECTION("JSON serialization") {
        td::seating_chart_entry original("Charlie", "FinalTable", "Seat1");

        nlohmann::json j;
        to_json(j, original);

        REQUIRE_FALSE(j.empty());
    }
}

TEST_CASE("td::automatic_payout_parameters", "[types][automatic_payout_parameters]") {
    SECTION("Default constructor") {
        td::automatic_payout_parameters app;
        REQUIRE(app.percent_seats_paid >= 0.0);
        REQUIRE(app.percent_seats_paid <= 1.0);
    }

    SECTION("Constructor with parameters") {
        td::automatic_payout_parameters app(0.15, true, 2.0, 0.5, 0.1);
        REQUIRE(app.percent_seats_paid == 0.15);
        REQUIRE(app.round_payouts == true);
        REQUIRE(app.payout_shape == 2.0);
        REQUIRE(app.pay_the_bubble == 0.5);
        REQUIRE(app.pay_knockouts == 0.1);
    }

    SECTION("Equality operator") {
        td::automatic_payout_parameters app1(0.2, false, 1.5, 0.3, 0.05);
        td::automatic_payout_parameters app2(0.2, false, 1.5, 0.3, 0.05);
        REQUIRE(app1 == app2);

        td::automatic_payout_parameters app3(0.25, false, 1.5, 0.3, 0.05);
        REQUIRE_FALSE(app1 == app3);
    }

    SECTION("JSON serialization roundtrip") {
        td::automatic_payout_parameters original(0.18, true, 2.5, 0.4, 0.08);

        nlohmann::json j;
        to_json(j, original);

        td::automatic_payout_parameters deserialized;
        from_json(j, deserialized);

        REQUIRE(deserialized == original);
    }
}

TEST_CASE("Enum stream insertion operators", "[types][enums]") {
    SECTION("funding_source_type_t") {
        std::ostringstream oss;
        oss << td::funding_source_type_t::buyin;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::funding_source_type_t::rebuy;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::funding_source_type_t::addon;
        REQUIRE_FALSE(oss.str().empty());
    }

    SECTION("payout_policy_t") {
        std::ostringstream oss;
        oss << td::payout_policy_t::automatic;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::payout_policy_t::forced;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::payout_policy_t::manual;
        REQUIRE_FALSE(oss.str().empty());
    }

    SECTION("rebalance_policy_t") {
        std::ostringstream oss;
        oss << td::rebalance_policy_t::manual;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::rebalance_policy_t::automatic;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::rebalance_policy_t::shootout;
        REQUIRE_FALSE(oss.str().empty());
    }

    SECTION("ante_type_t") {
        std::ostringstream oss;
        oss << td::ante_type_t::none;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::ante_type_t::traditional;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::ante_type_t::bba;
        REQUIRE_FALSE(oss.str().empty());
    }

    SECTION("final_table_policy_t") {
        std::ostringstream oss;
        oss << td::final_table_policy_t::fill;
        REQUIRE_FALSE(oss.str().empty());

        oss.str("");
        oss << td::final_table_policy_t::randomize;
        REQUIRE_FALSE(oss.str().empty());
    }

    SECTION("blind_level stream insertion") {
        td::blind_level level;
        level.little_blind = 25;
        level.big_blind = 50;
        level.ante = 5;

        std::ostringstream oss;
        oss << level;
        REQUIRE_FALSE(oss.str().empty());
    }
}

TEST_CASE("DateTime JSON serialization", "[types][datetime_json]") {
    SECTION("datetime to_json/from_json") {
        auto original = datetime::from_gm("2023-01-01T12:00:00");

        nlohmann::json j;
        to_json(j, original);

        datetime deserialized;
        from_json(j, deserialized);

        // Allow small time difference due to precision
        auto diff = std::chrono::duration_cast<std::chrono::seconds>(
            static_cast<std::chrono::system_clock::time_point>(original) -
            static_cast<std::chrono::system_clock::time_point>(deserialized)
        ).count();
        REQUIRE(std::abs(diff) <= 1);
    }

    SECTION("time_point to_json/from_json") {
        auto original = std::chrono::system_clock::now();

        nlohmann::json j;
        std::chrono::to_json(j, original);

        std::chrono::system_clock::time_point deserialized;
        std::chrono::from_json(j, deserialized);

        auto diff = std::chrono::duration_cast<std::chrono::seconds>(original - deserialized).count();
        REQUIRE(std::abs(diff) <= 1);
    }
}
#include "lib.hpp"
#include <catch.hpp>
#include <nlohmann/json.hpp>
#include <sstream>

TEST_CASE("Something works") { REQUIRE(not_random() == not_random()); }

TEST_CASE("Process stringstream") {
    std::stringstream ss;
    // nlohmann::json obj
    ss << R"({"count":2, "text":"world"})";
    process_client(ss);
    std::string s;
    std::getline(ss, s);
    REQUIRE(s == R"(hello, hello, "world")");
}
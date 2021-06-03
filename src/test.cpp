#include "lib.hpp"
#include <catch.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sstream>

using boost::asio::ip::tcp;

TEST_CASE("Something works") { REQUIRE(not_random() == not_random()); }
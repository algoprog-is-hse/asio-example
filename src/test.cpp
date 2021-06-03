#include "lib.hpp"
#include <catch.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <sstream>

using boost::asio::ip::tcp;

TEST_CASE("Something works") { REQUIRE(not_random() == not_random()); }

TEST_CASE("Process stringstream") {
    std::stringstream ss;
    nlohmann::json obj{{"count", 2}, {"text", "world"}};
    ss << obj << std::flush;
    // ss << R"({"count":2, "text":"world"})";
    process_client(ss);
    std::string s;
    std::getline(ss, s);
    REQUIRE(s == R"(hello, hello, world)");
}

TEST_CASE("Network") {
    boost::asio::io_context io_context;
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 0));
    tcp::socket socket(io_context);
    acceptor.async_accept(socket, [&](boost::system::error_code ec) {
        REQUIRE(!ec);
        std::make_shared<session>(io_context, std::move(socket))->go();
    });
    std::thread th{[&] {
        tcp::iostream stream(acceptor.local_endpoint());
        nlohmann::json obj{{"count", 2}, {"text", "world"}};
        stream << obj << std::flush;
        std::string s;
        std::getline(stream, s);
        REQUIRE(s == R"(hello, hello, world)");
        stream << nlohmann::json{{"exit", 1}} << std::flush;
    }};
    io_context.run();
    th.join();
}
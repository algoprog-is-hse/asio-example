#include "lib.hpp"
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <stdexcept>

int not_random() { return 3; }

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

struct abort_exception : std::domain_error {
    abort_exception() : std::domain_error{"Exit requested"} {}
};

void process_client(std::iostream &stream) {
    nlohmann::json obj;
    stream >> obj;
    if (obj.is_object() && obj.contains("count")) {
        char str[] = "hello, ";
        int n = obj["count"].get<int>();
        for (int i = 0; i < n; ++i)
            stream << str;
    }
    if (obj.is_object() && obj.contains("text"))
        stream << obj["text"].get<std::string>() << "\n";
    if (obj.is_object() && obj.contains("exit"))
        throw abort_exception{};
}

void session::go() {
    auto self(shared_from_this());
    boost::asio::spawn(strand,
                       std::bind(&session::loop, self, std::placeholders::_1));

    // boost::asio::spawn(strand, delegate(self, &session::timer_callback));
}

void session::loop(boost::asio::yield_context yield) {
    beast::error_code ec;

    // This buffer is required to persist across reads
    beast::flat_buffer buffer;

    for (;;) {
        // Set the timeout.
        stream.expires_after(std::chrono::seconds(30));

        // Read a request
        http::request<http::string_body> req;
        http::async_read(stream, buffer, req, yield[ec]);
        if (ec == http::error::end_of_stream)
            break;
        if (ec) {
            std::cerr << "read: " << ec.message() << "\n";
            return;
        }

        // Send the response
        if (req.method() == http::verb::get && req.target() == "/hello") {
            http::response<http::string_body> res;
            res.body() = "<h1>Hello</h1>";
            http::async_write(stream, res, yield[ec]);
        } else {
            http::response<http::string_body> res;
            res.body() = fmt::format("<h1>Not Found: '<span>{}</span>'</h1>", req.target());
            res.reason("Not found");
            res.result(http::status::not_found);
            http::async_write(stream, res, yield[ec]);
        }
    }

    // Send a TCP shutdown
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    // At this point the connection is closed gracefully
}

session::session(boost::asio::io_context &io_context,
                 boost::asio::ip::tcp::socket t_socket)
    : stream(std::move(t_socket)), strand(io_context.get_executor()) {}

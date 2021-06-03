#include "lib.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>

int not_random() { return 3; }

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
    timer.expires_from_now(std::chrono::seconds(10));
    th = std::thread([this, self] {
        try {
            std::locale loc{""};
            for (;;) { // while (true)
                timer.expires_from_now(std::chrono::seconds(10));
                process_client(stream);
            }
        } catch (std::exception &e) {
            stream.close();
            timer.cancel();
        }
    });
    th.detach();

    // boost::asio::spawn(strand, delegate(self, &session::timer_callback));
}

void session::timer_callback(boost::asio::yield_context yield) {
    while (stream.socket().is_open()) {
        boost::system::error_code ignored_ec;
        timer.async_wait(yield[ignored_ec]);
        auto now = decltype(timer)::time_point::clock::now();
        if (timer.expiry() <= now)
            stream.close();
    }
}

session::session(boost::asio::io_context &io_context,
                 boost::asio::ip::tcp::socket t_socket)
    : stream(std::move(t_socket)), timer(io_context),
      strand(io_context.get_executor()) {}

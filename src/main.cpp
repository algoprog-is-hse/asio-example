//
// echo_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "lib.hpp"
#include <boost/asio.hpp>
#include <boost/asio/basic_socket_iostream.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/thread_pool.hpp>
#include <charconv>
#include <iostream>
#include <locale>
#include <memory>
#include <string_view>
#include <utility>

using boost::asio::ip::tcp;
// TODO: add tests
// TODO: add HTTP

template <typename Class, typename Function>
auto delegate(std::shared_ptr<Class> ptr, Function fun) {
    // return [ptr = std::move(ptr), fun]() {
    return [ ptr, fun ]<typename... Args>(Args && ... arg) {
        return (ptr.get()->*fun)(std::forward<Args>(arg)...);
    };
}

int main(int argc, char *argv[]) {
    try {
        unsigned short port = 1234;
        if (argc > 1)
            port = static_cast<unsigned short>(std::atoi(argv[1]));
        // if (argc != 2) {
        //     std::cerr << "Usage: echo_server <port>\n";
        //     return 1;
        // }

        boost::asio::io_context io_context;

        boost::asio::spawn(io_context, [&](boost::asio::yield_context yield) {
            tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));

            while (true) {
                boost::system::error_code ec;
                tcp::socket socket(io_context);
                acceptor.async_accept(socket, yield[ec]);
                if (!ec) {
                    std::make_shared<session>(io_context, std::move(socket))
                        ->go();
                } else {
                    std::cerr << ec << "\n";
                }
            }
        });

        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

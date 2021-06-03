#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <memory>
#include <thread>

extern int not_random();
extern void process_client(std::iostream &stream);

class session : public std::enable_shared_from_this<session> {
    // Curiosly recurring template pattern (CRTP)
  public:
    explicit session(boost::asio::io_context &io_context,
                     boost::asio::ip::tcp::socket socket);

    /// Run session in a separate thread
    void go();
  private:
    void loop(boost::asio::yield_context yield);
    boost::beast::tcp_stream stream;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
};

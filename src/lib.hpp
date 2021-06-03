#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>
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
    void timer_callback(boost::asio::yield_context yield);
    std::thread th;
    boost::asio::ip::tcp::iostream stream;
    boost::asio::steady_timer timer;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;
};

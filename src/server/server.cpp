//
// server.cpp
// ~~~~~~~~~~
//
// Original Work Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Modified Work Copyright 2018 Cody Feltch
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include "../../include/connection.hpp" // Must come before boost/serialization headers.
#include <boost/serialization/vector.hpp>
#include "../../include/eye_message.hpp"
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>


namespace codechallenge
{

/// Serves eye messages to any client that connects to it.
class server
{
public:
    /// Constructor opens the acceptor and starts waiting for the first incoming
    /// connection.
    server(boost::asio::io_service& io_service)
        : acceptor_(io_service,
                    boost::asio::local::stream_protocol::endpoint("/tmp/code_challenge/streams")) {
        this->io_service = &io_service;

        // Start an accept operation for a new connection.
        connection_ptr new_conn(new connection(acceptor_.get_io_service()));
        acceptor_.async_accept(new_conn->socket(),
                               boost::bind(&server::handle_accept, this,
                                           boost::asio::placeholders::error, new_conn));

    }

    /// Handle completion of a accept operation.
    void handle_accept(const boost::system::error_code& e, connection_ptr conn) {
        if (!e) {
            std::cout << "Client Connected!" << std::endl;

            // Setup eye-data publishing timer.
            boost::thread t([this, conn]() {
                boost::asio::io_service io;
                boost::asio::deadline_timer t(io, boost::posix_time::milliseconds(10));
                t.async_wait(boost::bind(&server::send_eye_message, this, boost::asio::placeholders::error, &t, conn));
                io.run();
            });
        }

        // Start an accept operation for a new connection.
        connection_ptr new_conn(new connection(acceptor_.get_io_service()));
        acceptor_.async_accept(new_conn->socket(),
                               boost::bind(&server::handle_accept, this,
                                           boost::asio::placeholders::error, new_conn));
    }

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code& e, boost::asio::deadline_timer* t, connection_ptr conn) {
        // Just assume that a write error is caused by the client socket closing
        if(e) {
            // Stop the io service sending messages to the client, allow the connection finish
            t->get_io_service().stop();
            std::cout << "Client Disconnected" << std::endl;
        }
    }

    /// Send one or more eye_messages to client connection
    void send_eye_message(const boost::system::error_code& e,
                          boost::asio::deadline_timer* t, connection_ptr conn) {

        // Create a vector of eye messages
        std::vector<eye_message> stocks_;
        for (int i=0; i<server::sample_chunk_length; i++) {
            eye_message msg;
            msg.time_seconds = time(0);
            msg.time_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            msg.id = rand()%2;
            msg.confidence = rand()%2;
            msg.normalized_pos_x = (rand()%1000)/1000.0;
            msg.normalized_pos_y = (rand()%1000)/1000.0;
            msg.pupil_diameter = rand()%100;
            stocks_.push_back(msg);
        }

        // Write messages to the stream
        conn->async_write(stocks_,
                          boost::bind(&server::handle_write, this,
                                      boost::asio::placeholders::error, t, conn));

        // Schedule next message, or finish
        if(!should_stop && !e) {
            t->expires_at(t->expires_at() + boost::posix_time::milliseconds(1000));
            t->async_wait(boost::bind(&server::send_eye_message, this, boost::asio::placeholders::error, t, conn));
        } else {
            t->get_io_service().stop();
            return;
        }

    }

    /// Run the io service on a seperate thread
    void start() {
        should_stop = false;
        server_thread = new boost::thread([this]() {
            io_service->run();
        });
    }

    /// Stop the thread running the io service
    void stop() {
        should_stop = true;
        io_service->stop();
        server_thread->join();
    }

private:
    /// The acceptor object used to accept incoming socket connections.
    boost::asio::local::stream_protocol::acceptor acceptor_;

    /// IO Service
    boost::asio::io_service* io_service;

    /// Num samples to send in each chunk
    const static int sample_chunk_length = 1;

    /// Run Thread
    boost::thread* server_thread;

    /// Flag for service thread execution
    std::atomic_bool should_stop {false};
};

} // namespace codechallenge

int main(int argc, char* argv[])
{
    try {

        // Handle command line arguments.
        if (argc != 1) {
            std::cout << "Ignoring additional input arguments." << std::endl;
        }

        // Remove and recreate socket directory, just in case
        boost::filesystem::remove_all("/tmp/code_challenge/");
        boost::filesystem::create_directories("/tmp/code_challenge/");

        // Setup Server
        boost::asio::io_service io_service;
        codechallenge::server server(io_service);

        // Run until input
        server.start();
        std::cout << "Running..." << std::endl;
        system("read -p 'Press <Enter> to stop\n' var");
        server.stop();
        std::cout << "Server Stopped" << std::endl;

        // Cleanup tmp directory
        boost::filesystem::create_directories("/tmp/code_challenge/");

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

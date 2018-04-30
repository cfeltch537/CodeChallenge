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
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>
#include "../../include/connection.hpp" // Must come before boost/serialization headers.
#include <boost/serialization/vector.hpp>
#include "../../include/eye_message.hpp"
#include <stdlib.h>
#include <time.h>

namespace codechallenge
{

/// Serves stock quote information to any client that connects to it.
class server
{
public:
    /// Constructor opens the acceptor and starts waiting for the first incoming
    /// connection.
    server(boost::asio::io_service& io_service, unsigned short port)
        : acceptor_(io_service,
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        // Create the data to be sent to each client.
        ulong current_id = 0;

        for (int i=0; i<10; i++) {
            eye_message s;
            s.seq_number = current_id;
            s.time_seconds = rand()%1000;
            s.time_millis = rand()%1000;
            s.id = rand()%2;
            s.confidence = rand()%2;
            s.normalized_pos_x = (rand()%1000)/1000.0;
            s.normalized_pos_y = (rand()%1000)/1000.0;
            s.pupil_diameter = rand()%100;
            stocks_.push_back(s);
            current_id++;
        }

        // Start an accept operation for a new connection.
        connection_ptr new_conn(new connection(acceptor_.get_io_service()));
        acceptor_.async_accept(new_conn->socket(),
                               boost::bind(&server::handle_accept, this,
                                           boost::asio::placeholders::error, new_conn));
    }

    /// Handle completion of a accept operation.
    void handle_accept(const boost::system::error_code& e, connection_ptr conn) {
        if (!e) {
            // Successfully accepted a new connection. Send the list of stocks to the
            // client. The connection::async_write() function will automatically
            // serialize the data structure for us.
            conn->async_write(stocks_,
                              boost::bind(&server::handle_write, this,
                                          boost::asio::placeholders::error, conn));
        }

        // Start an accept operation for a new connection.
        connection_ptr new_conn(new connection(acceptor_.get_io_service()));
        acceptor_.async_accept(new_conn->socket(),
                               boost::bind(&server::handle_accept, this,
                                           boost::asio::placeholders::error, new_conn));
    }

    /// Handle completion of a write operation.
    void handle_write(const boost::system::error_code& e, connection_ptr conn) {
        // Nothing to do. The socket will be closed automatically when the last
        // reference to the connection object goes away.
    }

private:
    /// The acceptor object used to accept incoming socket connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The data to be sent to each client.
    std::vector<eye_message> stocks_;
};

} // namespace codechallenge

int main(int argc, char* argv[])
{
    try {
        unsigned short port;

        // Check command line arguments.
        if (argc == 1) {
            std::cout << "No port specified, using Port: 12345" << std::endl;
            port = 12345;
        } else if (argc == 2) {
            port = boost::lexical_cast<unsigned short>(argv[1]);
            std::cout << "Using Port: " << port << std::endl;
        } else {
            std::cerr << "Usage: server, server <port>" << std::endl;
            return 1;
        }

        // Setup Server
        boost::asio::io_service io_service;
        codechallenge::server server(io_service, port);
        std::cout << "Running..." << std::endl;
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

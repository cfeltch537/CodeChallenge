//
// client.cpp
// ~~~~~~~~~
//
// Original Work Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Modified Work Copyright 2018 Cody Feltch
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <vector>
#include "../../include/connection.hpp" // Must come before boost/serialization headers.
#include <boost/serialization/vector.hpp>
#include "../../include/eye_message.hpp"
#include <iostream>
#include <fstream>

namespace codechallenge
{

/// Downloads stock quote information from a server.
class client
{
public:
    /// Constructor starts the asynchronous connect operation.
    client(boost::asio::io_service& io_service,
           const std::string& host, const std::string& service)
        : connection_(io_service) {
        this->io_service = &io_service;

        boost::asio::local::stream_protocol::endpoint ep("/tmp/code_challenge/streams");

        // Open file for data logging
        open_file();

        connection_.socket().async_connect(ep, boost::bind(&client::handle_connect, this,
                                               boost::asio::placeholders::error));
    }

    /// Deconstructor
    ~client() {
        // Close data log file
        close_file();
    }

    /// Handle completion of a connect operation.
    void handle_connect(const boost::system::error_code& e) {
        if (!e) {
            // Successfully established connection. Start operation to read the list
            // of stocks. The connection::async_read() function will automatically
            // decode the data that is read from the underlying socket.
            connection_.async_read(stocks_,
                                   boost::bind(&client::handle_read, this,
                                               boost::asio::placeholders::error));
        } else {
            // An error occurred. Log it and return. Since we are not starting a new
            // operation the io_service will run out of work to do and the client will
            // exit.
            std::cerr << e.message() << std::endl;
        }
    }

    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code& e) {
        if (!e) {

            // Print out the eye data that was received.
            for (std::size_t i = 0; i < stocks_.size(); ++i) {
                message_count++;
                std::cout<< "\n" << "Count: " << message_count << ", ";
                std::cout << "Time(Sec): " << stocks_[i].time_seconds << ", ";
                std::cout << "Time(Nanos): " << stocks_[i].time_nanos << ", ";
                std::cout << "ID: " << stocks_[i].id << ", ";
                std::cout << "Confidence: " << stocks_[i].confidence << ", ";
                std::cout << "NormalizedPosX: "  << stocks_[i].normalized_pos_x << ", ";
                std::cout << "NormalizedPosY: " << stocks_[i].normalized_pos_y << ", ";
                std::cout << "PupilDiameter: " << stocks_[i].pupil_diameter << ", ";

                // Write sample data to file
                write_sample_to_file(stocks_[i]);

            }
            // Listen for additional data
            connection_.async_read(stocks_,
                                   boost::bind(&client::handle_read, this,
                                               boost::asio::placeholders::error));
        } else {
            // An error occurred.
            std::cerr << e.message() << std::endl;
        }
    }

    /// Open a file, named with timestamp, for logging eye data
    void open_file() {
        // Get Time for Filename
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // Get Filename
        char filename[1000];
        boost::filesystem::create_directories("./saved_data");
        strftime(filename, sizeof(filename),"./saved_data/eyedata_%Y%m%d%H%M%S.csv", timeinfo);

        // Open File
        myfile.open (filename);

        // Write Header
        myfile << "Timestamp(Seconds)" << ",";
        myfile << "Timestamp(Nanoseconds)" << ",";
        myfile << "ID" << ",";
        myfile << "Confidence" << ",";
        myfile << "NormalizedPosX" << ",";
        myfile << "NormalizedPosY" << ",";
        myfile << "PupilDiameter";
        myfile << "\n";
    }

    /// Close data log file
    void close_file() {
        myfile.close();
    }

    /// Write an eye message's data to the log file
    void write_sample_to_file(eye_message s) {
        myfile << s.time_seconds << ",";
        myfile << s.time_nanos << ",";
        myfile << s.id << ",";
        myfile << s.confidence << ",";
        myfile << s.normalized_pos_x << ",";
        myfile << s.normalized_pos_y << ",";
        myfile << s.pupil_diameter;
        myfile << "\n";
    }

    /// Run the io service on a seperate thread
    void start() {
        should_stop = false;
        client_thread = new boost::thread([this]() {
            io_service->run();
        });
    }

    /// Stop the thread running the io service
    void stop() {
        should_stop = true;
        io_service->stop();
        boost::system::error_code e;
        connection_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, e);
        connection_.socket().close(e);
        client_thread->join();
    }

private:
    /// The connection to the server.
    connection connection_;

    /// The data received from the server.
    std::vector<eye_message> stocks_;

    /// CSV File For Saved Data
    std::ofstream myfile;

    /// IO Service
    boost::asio::io_service* io_service;

    /// Run Thread
    boost::thread* client_thread;

    /// Flag for client thread execution
    std::atomic_bool should_stop {false};

    /// Counter for number of received messages
    int message_count = 0;

};

} // namespace codechallenge

int main(int argc, char* argv[])
{
    try {

        std::string host = "localhost";
        std::string port = "12345";

        // Check command line arguments.
        if(argc == 2) {
            std::cout << "No Host IP Given - Using default." << std::endl;
            port = argv[1];
        } else if (argc == 3) {
            host = argv[1];
            port = argv[2];
        } else {
            std::cerr << "Usage: client <host> <port>, client <port>, client" << std::endl;
            return 1;
        }
        std::cout << "Host: " << host << ", Port: " << port << "..." << std::endl;

        // Setup Client
        boost::asio::io_service io_service;
        codechallenge::client client(io_service, host, port);

        // Run until input
        client.start();
        std::cout << "Running..." << std::endl;
        system("read -p 'Press <Enter> to stop' var");
        client.stop();
        std::cout << "Client Stopped" << std::endl;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

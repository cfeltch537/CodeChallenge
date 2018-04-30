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
        // Resolve the host name into an IP address.
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(host, service);
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
            resolver.resolve(query);

        // Start an asynchronous connect operation.
        boost::asio::async_connect(connection_.socket(), endpoint_iterator,
                                   boost::bind(&client::handle_connect, this,
                                               boost::asio::placeholders::error));
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
            open_file();

            // Print out the data that was received.
            for (std::size_t i = 0; i < stocks_.size(); ++i) {
                std::cout << "EyeData number " << i << "\n";
                std::cout << "  sequence_num: " << stocks_[i].seq_number << "\n";
                std::cout << "  time_seconds: " << stocks_[i].time_seconds << "\n";
                std::cout << "  time_millis: " << stocks_[i].time_millis << "\n";
                std::cout << "  id: " << stocks_[i].id << "\n";
                std::cout << "  confidence: " << stocks_[i].confidence << "\n";
                std::cout << "  normalized_pos_x: " << stocks_[i].normalized_pos_x << "\n";
                std::cout << "  normalized_pos_y: " << stocks_[i].normalized_pos_y << "\n";
                std::cout << "  pupil_diameter: " << stocks_[i].pupil_diameter << "\n";

                write_sample_to_file(stocks_[i]);

            }
            close_file();

        } else {
            // An error occurred.
            std::cerr << e.message() << std::endl;
        }

        // Since we are not starting a new operation the io_service will run out of
        // work to do and the client will exit.
    }

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

    void close_file() {
        myfile.close();
    }

    void write_sample_to_file(eye_message s) {
        myfile << s.time_seconds << ",";
        myfile << s.time_millis << ",";
        myfile << s.id << ",";
        myfile << s.confidence << ",";
        myfile << s.normalized_pos_x << ",";
        myfile << s.normalized_pos_y << ",";
        myfile << s.pupil_diameter;
        myfile << "\n";
    }

private:
    /// The connection to the server.
    connection connection_;

    /// The data received from the server.
    std::vector<eye_message> stocks_;

    // CSV File For Saved Data
    std::ofstream myfile;

};

} // namespace codechallenge

int main(int argc, char* argv[])
{
    try {

        std::string host = "localhost";
        std::string port = "12345";

        // Check command line arguments.
        if(argc == 1) {
            std::cout << "No Host or Port Given." << std::endl;
        } else if(argc == 1) {
            std::cout << "No Port Given." << std::endl;
            host = argv[1];
        } else if (argc == 3) {
            host = argv[1];
            port = argv[2];
        } else {
            std::cerr << "Usage: dfsfclient <host> <port>" << std::endl;
            return 1;
        }

        std::cout << "Running, Host: " << host << ", Port: " << port << "..." << std::endl;

        boost::asio::io_service io_service;
        codechallenge::client client(io_service, host, port);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

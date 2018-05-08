# CodeChallenge (EyeData Client-Server)

My Solution to the Eye Data Code Challenge.

My solution consists of two processes, a server and a client. The server, capable of handling multiple clients, periodically (~100Hz) sends randomly generated EyeData to its clients over UNIX domain sockets. This project is developed in C++, using Boost ASIO.

## Getting Started

These instructions will get you a walk you through setting up your development environment, then building and running this project.

## Dependencies

### Built With

* [CMake 3.5.1](https://cmake.org/download/)
* [Codelight](https://codelite.org/)
* [Boost 1.58](https://www.boost.org/)
* [GCC (g++)](Uhttps://gcc.gnu.org)

### Built On
* Ubuntu 16.04.4 LTS

### Prerequisites

What things you need to install the software and how to install them

#### Build tools

```
sudo apt-get install g++ codelite cmake
```

#### Boost
Boost installation instructions:
```
https://www.boost.org/doc/libs/1_58_0/more/getting_started/unix-variants.html
```
OR... through package manager (version may vary)

```
sudo apt-get install libboost-all-development
```

### Building

Run build script

```
./build.sh
```

Open workspace in codelite

```
codelite CodeChallenge.workspace &
```

Build project in Codelite (g++)

```
Build -> Build Project
```

In the /bin folder, should be a 'client' and 'server' executable
## Running the processes

The client and server both run without any additional input parameters.
Both run until interrupted by the ENTER key press.

### Run Server

```
./bin/server
```

Example Output

```
./server
Running...
Press <Enter> to stop
Client Connected!
Client Disconnected
Server Stopped
```

### Run Client

```
./bin/client
```

Example Output

```
./client
Running...
Press <Enter> to stop
Count: 1, Time(Sec): 1525753885, Time(Nanos): 700283012, ID: 1, Confidence: 0, NormalizedPosX: 0.777, NormalizedPosY: 0.915, PupilDiameter: 93,
Count: 2, Time(Sec): 1525753885, Time(Nanos): 711014350, ID: 1, Confidence: 0, NormalizedPosX: 0.492, NormalizedPosY: 0.649, PupilDiameter: 21,
Client Stopped
```

## ToDo

* Implement Unit Testing
* Implement a true Publisher-Subscriber system


## Challenge Questions

### Your solution’s non-functional requirements:
1. You should design your code with modular interfaces that make it easy to move to
different messaging and transport libraries if needed.

  **The asynchronous message handling of boost::asio is baked into this solution pretty deeply, but the 'base_connection' class in 'connection.hpp' can be implemented to make use of any message format or transport library.**

2. Your solution should focus on low latency whenever possible without sacrificing modular
design constraints listed in 1.

  **The main points of latency for this type of application would likely be message serialization/deserialization, transmission, and logging. The transmission medium is a low latency Unix Domain Socket. The serialization is done through the boost::archive. File logging could probably be made more efficient by implementing a buffering queue of incoming messages and a producer/consumer pattern and logging in chunks.**

3. Your solution must be appropriately documented and tested.

  **This README is meant to serve as documentation. I manually tested the final implementation but have not yet had a chance to implement any unit tests.**


### Things you should consider in constructing your solution:
1. Design trade-offs such as why a particular message or transport library was selected,
advantages/disadvantages, effects on latency and throughput, scalability, long term
library support, etc.

  **I chose to develop this project in C++, using the boost::asio library for asynchronous connection and message handling. [Latency] All data communication is handled in boost's use of the very lightweight unix domain sockets. [Throughput] The message serialization/deserialization is handled using the boost::archive library. [Scalability] The current implementation can handle multiple clients (but each client will receive different data). The program could easily be modified to keep a list of clients, and send the same data to each. [Long Term Support] Boost isn't going anywhere and all of the libraries used have a user-base.**

2. Changes needed to accommodate acknowledgement of the process’ identity and how to
ensure appropriate handling of messages from unidentified processes.

  **This can be implemented as part of the message header.**

3. How the data is logged and how you would query the information in post-hoc analyses.

  **The data is logged to CSV files. Anything can read CSV files, either for use or for populating a database.**

## Authors

* **Cody Feltch** - *Initial work* - [cfeltch537](https://github.com/cfeltch537)


## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* BOOST ASIO Examples: https://www.boost.org/doc/libs/1_53_0/doc/html/boost_asio/examples.html
* The INTERNET!!!

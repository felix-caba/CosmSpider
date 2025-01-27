#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include "Ports.h"

class PortScanner {
public:
    PortScanner(const std::string& ipAddress) 
        : ip(ipAddress), portServiceMap(PORT_SERVICE_MAP) {}

    void scan() {
        std::vector<std::thread> threads;
        for (const auto& port_entry : portServiceMap) {
            threads.emplace_back(&PortScanner::scanPort, this, port_entry.first);
        }

        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    std::unordered_map<int, std::string> getOpenPorts() const {
        return open_ports;
    }

    int getClosedPorts() const {
        return closedPorts;
    }

private:
    std::string ip;
    const std::unordered_map<int, std::string>& portServiceMap;
    std::unordered_map<int, std::string> open_ports;
    std::mutex port_mutex;
    int closedPorts = 0;

    void scanPort(int port) {
        try {
            boost::asio::io_context io_context;
            boost::asio::ip::tcp::socket socket(io_context);
            boost::asio::ip::tcp::endpoint endpoint(
                boost::asio::ip::make_address(ip), port
            );

            // Set a timeout for the connection attempt
            boost::asio::deadline_timer timer(io_context);
            timer.expires_from_now(boost::posix_time::seconds(1)); // 1-second timeout

            bool connected = false;
            boost::system::error_code error;

            // Asynchronous connect with timeout
            socket.async_connect(endpoint, [&](const boost::system::error_code& ec) {
                if (!ec) {
                    connected = true;
                }
                error = ec;
                timer.cancel();
            });

            timer.async_wait([&](const boost::system::error_code& ec) {
                if (!ec) { // Timer expired, cancel the connection
                    socket.close();
                }
            });

            io_context.run(); // Run the io_context to handle async operations

            if (connected) {
                std::cout << "Port " << port << " is open of ip " << ip << std::endl;
                std::lock_guard<std::mutex> lock(port_mutex);
                auto service = portServiceMap.find(port);
                if (service != portServiceMap.end()) {
                    open_ports[port] = service->second;
                } else {
                    open_ports[port] = "Unknown";
                }
            } else {
                std::lock_guard<std::mutex> lock(port_mutex);
                closedPorts++;
            }
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(port_mutex);
            closedPorts++;
        }
    }
};
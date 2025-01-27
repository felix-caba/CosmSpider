
#define SCANNER_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "ports.h"
#include <iostream>

// declaro
class PortScanner
{
public:
    PortScanner(const std::string &ipAddress) : ip(ipAddress), portServiceMap(PORT_SERVICE_MAP), io_context(), resolver(io_context)
    {
    }

    void scan()
    {
    
        // Hilo para ejecutar el io_context
        std::thread io_thread([this]()
                              { io_context.run(); });

        std::vector<std::thread> threads;
        for (const auto &port : PORT_SERVICE_MAP)
        {
            threads.emplace_back(&PortScanner::scanPort, this, port.first);
        }

        for (auto &t : threads)
        {
            t.join();
        }

        io_context.stop(); // Detener el io_context
        io_thread.join();  // Esperar a que termine el hilo del io_context
    }

    std::unordered_map<int, std::string> getOpenPorts() const
    {
        return open_ports;
    }

    int getClosedPorts() const
    {
        return closedPorts;
    }

private:
    // Atributos
    std::string ip;
    const std::unordered_map<int, std::string> &portServiceMap;
    std::unordered_map<int, std::string> open_ports;
    std::string operating_system;
    std::mutex port_mutex;
    int closedPorts = 0;

    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver;

    void scanPort(int port)
    {
        try
        {
            boost::asio::ip::tcp::socket socket(io_context);
            boost::asio::ip::tcp::endpoint endpoint(
                boost::asio::ip::make_address(ip), port);

          
            boost::asio::deadline_timer timer(io_context);
            timer.expires_from_now(boost::posix_time::seconds(10));

            // Operación asíncrona para cancelar la conexión si hay timeout
            bool connected = false;
            timer.async_wait([&socket](const boost::system::error_code &ec)
                             {
                                 if (!ec)
                                     socket.cancel(); // Cancela la conexión si el timer expira
                             });

            // Conexión asíncrona
            boost::system::error_code error;
            socket.async_connect(endpoint, [&](const boost::system::error_code &ec)
                                 {
                                     if (!ec)
                                         connected = true;
                                     timer.cancel(); // Cancela el timer si la conexión se completa
                                 });

            // Ejecutar el io_context para procesar operaciones asíncronas
            io_context.run();

            if (connected)
            {


                std::cout << "Port " << port << " is open" << std::endl;


                std::lock_guard<std::mutex> lock(port_mutex);
                auto service = PORT_SERVICE_MAP.find(port);
                if (service != PORT_SERVICE_MAP.end())
                {
                    open_ports[port] = service->second;
                }
                else
                {
                    open_ports[port] = "Unknown";
                }
            }
            else
            {
                closedPorts++;
            }
        }
        catch (std::exception &e)
        {
            closedPorts++;
        }
    }
};
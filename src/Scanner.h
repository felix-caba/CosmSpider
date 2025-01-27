
#define SCANNER_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "ports.h"

// declaro
class PortScanner
{
public:
    PortScanner(const std::string &ipAddress) : ip(ipAddress), portServiceMap(PORT_SERVICE_MAP), io_context(), resolver(io_context)
    {
    }

    void scan()
    {
        std::vector<std::thread> threads;
        for (const auto &port : PORT_SERVICE_MAP)
        {
            threads.emplace_back(&PortScanner::scanPort, this, port.first);
        }

        for (auto &t : threads)
        {
            t.join();
        }
    }

    std::unordered_map<int, std::string> getOpenPorts() const
    {
        return open_ports;
    }

    int getClosedPorts() const
    {
        return closedPorts;
    }

    std::string getOperatingSystemX() const
    {
        return operating_system;
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

    std::string getOperatingSystem()
    {
    }

    void scanPort(int port)
    {

        try
        {

            boost::asio::ip::tcp::socket socket(io_context);
            boost::asio::ip::tcp::endpoint endpoint(
                boost::asio::ip::make_address(ip), port);

            boost::system::error_code error;
            socket.connect(endpoint, error);

            if (!error)
            {
                // bloquea la edicion del puerto mutex, asegura que sea
                // desbloqueado al terminar la edicion

                // el mutex hace que solo lo debajo de el se bloquee, hasta
                // que salga del scope, osea, ya terminado
                std::lock_guard<std::mutex> lock(port_mutex);

                // findea en el map el puerto que he escaneado

                auto service = PORT_SERVICE_MAP.find(port);

                // port service map es basicamente dentro del iterador,
                // lo que significa que ha llegado al final. si es end, es que
                // no lo ha encontrado, si no es end, es que ha salido guay
                // y le dice que coja el segundo valor que es la valor asociado a la key
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
            ++closedPorts;
        }
    }
};
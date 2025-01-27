
#define SCANNER_H

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "ports.h"


class PortScanner
{

public:
    PortScanner(const std::string &ipAddress) : ip(ipAddress), 
    portServiceMap(PORT_SERVICE_MAP), io_context(), resolver(io_context)
    {

       


    }

private:
    std::string ip;
    const std::unordered_map<int, std::string> &portServiceMap;
    std::unordered_map<int, std::string> open_ports;
    std::string operating_system;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver;
   
};


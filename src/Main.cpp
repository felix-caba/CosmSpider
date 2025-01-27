#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstring>
#include <fstream>
#include "Scanner.h"
#include "Ports.h"

std::vector<std::string> source_ips;
std::unordered_map<std::string, std::vector<std::string>> dest_file;

int main(int argc, char **argv)
{
    int rank, n_ranks;
    int ips_per_rank;

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE)
    {
        std::cerr << "MPI no soporta multihilo!" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

    // For each line on the file source_ips, add the IP to the source_ips vector and also add one nrank

    if (rank == 0)
    {
        std::ifstream file("./source_ips.txt");
        if (file.is_open())
        {
            std::string line;
            while (getline(file, line))
            {
                source_ips.push_back(line);
            }
            file.close();
        }
    }

    int num_ips = 0;
    if (rank == 0)
    {
        num_ips = source_ips.size();
    }
    MPI_Bcast(&num_ips, 1, MPI_INT, 0, MPI_COMM_WORLD); // Todos reciben num_ips

    // Redimensionar en todos los procesos
    source_ips.resize(num_ips);

    std::cout << "Number of IPs: " << num_ips << std::endl;

    if (rank != 0)
    {
        source_ips.resize(num_ips);
    }

    for (int i = 0; i < num_ips; i++)
    {

        int len = 0;

        // If the rank is 0, get the length of the IP
        if (rank == 0)
        {
            len = source_ips[i].size();
        }

        // Sends Length to all the Processes
        MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Cretes a Buffer of Length + 1 to store the IPs
        char *buffer = new char[len + 1];

        // Root sends the IP to all the Processes
        if (rank == 0)
        {
            strncpy(buffer, source_ips[i].c_str(), len);
        }

        MPI_Bcast(buffer, len, MPI_CHAR, 0, MPI_COMM_WORLD);
        buffer[len] = '\0';

        if (rank != 0)
        {
            source_ips[i] = buffer;
        }

        delete[] buffer;
    }

    ips_per_rank = floor(num_ips / n_ranks);

    // Process each IP with a rank

    int start = rank * ips_per_rank;
    int end = (rank == n_ranks - 1) ? num_ips : (rank + 1) * ips_per_rank;

    for (int i = start; i < end; i++)
    {

        std::cout << "Scanning IP: " << source_ips[i] << " with rank: " << rank << std::endl;

        PortScanner scanner(source_ips[i]);
        scanner.scan();
        std::unordered_map<int, std::string> open_ports = scanner.getOpenPorts();
        int closed_ports = scanner.getClosedPorts();

        std::string ip = source_ips[i];

        std::string open_ports_str = "";

        for (const auto &port : open_ports)
        {
            open_ports_str += std::to_string(port.first) + " ";
        }

        std::string closed_ports_str = std::to_string(closed_ports);

        std::string result = open_ports_str + " " + closed_ports_str + " ";
        

        dest_file[ip] = {result};
    }

    if (rank != 0)
    {
        std::string data_to_send;
        for (const auto &entry : dest_file)
        {
            data_to_send += entry.first + ":" + entry.second[0] + "\n";
        }

        int data_size = data_to_send.size();
        MPI_Send(&data_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(data_to_send.c_str(), data_size, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        std::ofstream outfile("./results.txt");
        // Write the results of rank 0 root
        for (const auto &entry : dest_file)
        {
            outfile << entry.first << ":" << entry.second[0] << "\n";
        }

        // Other ranks
        for (int other_rank = 1; other_rank < n_ranks; other_rank++)
        {
            int data_size;
            MPI_Recv(&data_size, 1, MPI_INT, other_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            char *buffer = new char[data_size + 1];
            MPI_Recv(buffer, data_size, MPI_CHAR, other_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            buffer[data_size] = '\0';

            outfile << buffer;
            delete[] buffer;
        }
        outfile.close();
    }
    MPI_Finalize();
    return 0;
}

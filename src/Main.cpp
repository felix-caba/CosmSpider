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

std::vector<std::string> source_ips;
std::unordered_map<std::string, std::vector<std::string>> dest_file;

int main(int argc, char **argv)
{
    int rank, n_ranks;
    int ips_per_rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

    // For each line on the file source_ips, add the IP to the source_ips vector and also add one nrank

    if (rank == 0)
    {
        std::ifstream file("../source_ips.txt");
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

    int num_ips = source_ips.size();

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

    for (int i = rank * ips_per_rank; i < (rank * ips_per_rank) + ips_per_rank; i++)
    {
   
    }



}

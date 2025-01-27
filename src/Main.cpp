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
    MPI_Init(&argc, &argv);

    // For each line on the file source_ips, add the IP to the source_ips vector and also add one nrank

     if (rank == 0) {
        std::ifstream file("../source_ips.txt");
        if (file.is_open()) {
            std::string line;
            while (getline(file, line)) {
                source_ips.push_back(line);
            }
            file.close();
        }
    }



    



    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);




}

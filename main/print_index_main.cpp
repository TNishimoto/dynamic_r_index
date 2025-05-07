#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/dynamic_r_index.hpp"
#include "libdivsufsort/sa.hpp"

int main(int argc, char *argv[])
{
std::cout << "\033[41m";
#ifdef RELEASE_BUILD
    std::cout << "Running in Release mode";
#elif defined(DEBUG_BUILD)

    std::cout << "Running in Debug mode";
#else
    std::cout << "Running in Unknown mode";
#endif
std::cout << "\e[m" << std::endl;
    cmdline::parser p;

    p.add<std::string>("input_file", 'i', "input index name", true);


    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file");

    uint64_t mark = stool::IO::load_first_64bits(input_file_path);

    std::chrono::system_clock::time_point st1, st2;
    st1 = std::chrono::system_clock::now();
    // uint is_r_index = 1;

    std::string index_name = "";

    if (mark == stool::dynamic_r_index::DynamicFMIndex::LOAD_KEY)
    {
        index_name = "Dynamic FM-index";
        stool::dynamic_r_index::DynamicFMIndex dfmi;
        {
            std::ifstream ifs;
            ifs.open(input_file_path, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Error: Could not open file for reading." << std::endl;
                throw std::runtime_error("File open error");
            }
            auto tmp = stool::dynamic_r_index::DynamicFMIndex::build_from_data(ifs);
            dfmi.swap(tmp);
        }
        dfmi.print_light_statistics();
    }
    else if (mark == stool::dynamic_r_index::DynamicRIndex::LOAD_KEY)
    {
        index_name = "Dynamic r-index";

        stool::dynamic_r_index::DynamicRIndex drfmi;
        {
            std::ifstream ifs;
            ifs.open(input_file_path, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Error: Could not open file for reading." << std::endl;
                throw std::runtime_error("File open error");
            }
            auto tmp = stool::dynamic_r_index::DynamicRIndex::build_from_data(ifs);
            drfmi.swap(tmp);
        }
        drfmi.print_light_statistics();
    }
    else
    {
        throw std::runtime_error("Invalid input path");
    }

    st2 = std::chrono::system_clock::now();

}

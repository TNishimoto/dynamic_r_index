#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/dynamic_r_index.hpp"
#include "include/fm_index_performance_test.hpp"
#include "include/r_index_performance_test.hpp"

#include "libdivsufsort/sa.hpp"


int main(int argc, char *argv[])
{
#ifdef DEBUG
    std::cout << "\033[41m";
    std::cout << "DEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif
#ifdef SLOWDEBUG
    std::cout << "\033[41m";
    std::cout << "SLOWDEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif

    cmdline::parser p;

    p.add<std::string>("input_file", 'i', "input index name", true);
    p.add<uint64_t>("seed", 's', "seed", false, 0);

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file");
uint64_t seed = p.get<uint64_t>("seed");

    uint64_t mark = stool::IO::load_first_64bits(input_file_path);

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
        stool::dynamic_r_index_test::PerformanceTest::performance_test(dfmi, seed);

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
        stool::r_index_test::PerformanceTest::performance_test(drfmi, seed);

    }
    else
    {
        throw std::runtime_error("Invalid input path");
    }


    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Index File: \t\t\t\t\t" << input_file_path << std::endl;
    std::cout << "Index Type: \t\t\t\t\t" << index_name << std::endl;


    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

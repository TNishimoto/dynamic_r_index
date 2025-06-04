#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
//#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"
#include "libdivsufsort/sa.hpp"

#include <filesystem>

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

    p.add<std::string>("input_file", 'i', "input file name", true);

    p.parse_check(argc, argv);
    std::string filename = p.get<std::string>("input_file");

    std::ifstream tmpInputStream;
    tmpInputStream.open(filename, std::ios::binary);
    std::vector<uint8_t> text;

    stool::IO::load(tmpInputStream, text);
    text.push_back(0);
    std::cout << text.size() << std::endl;

    std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text);
    std::vector<uint64_t> lcpArray = stool::construct_LCP_array(text, sa);

    uint64_t lcp_total = 0;
    uint64_t lcp_max = 0;
    
    for(uint64_t i = 0; i<lcpArray.size();i++){
        lcp_total += lcpArray[i];
        lcp_max = std::max(lcp_max, lcpArray[i]);


        if(i % 1000000 == 1){
            std::cout << "[" <<i << "/" << lcpArray.size() << "]" << " Total = " << lcp_total << ", Average = " << (lcp_total / i) << ", max = " << lcp_max << std::endl;
        }

    }

    std::cout << std::endl;

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "File: \t \t  " << filename << std::endl;
    std::cout << "n: \t \t "<< lcpArray.size() << std::endl;

    std::cout << "The sum of LCP values: \t "<< lcp_total << std::endl;
    std::cout << "The max of LCP values: \t "<< lcp_max << std::endl;
    std::cout << "The average of LCP values: \t "<< (lcp_total / lcpArray.size()) << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;


}

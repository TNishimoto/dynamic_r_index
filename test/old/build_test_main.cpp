#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"

#include "../include/old_implementations/btree_include.hpp"
#include "include/dynamic_r_index_test.hpp"
//

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

    // p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<uint>("mode", 'm', "mode", true);
    p.add<std::string>("input_path", 'i', "input_path", false);

    p.add<uint>("detailed_check", 'u', "detailed_check", false, 0);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");
    std::string input_path = p.get<std::string>("input_path");
    bool detailed_check = p.get<uint>("detailed_check") == 0 ? false : true;

    std::vector<std::pair<std::string, uint64_t>> time_profiler;
    stool::r_index::DynamicRIndex drfmi;
    drfmi.build_from_bwt_file(input_path, 252, &time_profiler);
}

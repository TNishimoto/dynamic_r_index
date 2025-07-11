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
#include "include/btree.hpp"
#include "include/permutation/pom_tree.hpp"
#include "include/permutation/permutation_order_maintenance.hpp"
#include "include/permutation/naive_pom.hpp"
#include "include/dynamic_fm_index/naive_dynamic_string.hpp"
#include "include/dynamic_fm_index/dynamic_fm_index.hpp"

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

    std::vector<uint64_t> sa = stool::construct_suffix_array(text);
    std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa);

    /*
    stool::drfmi::DynamicFMIndex dfmi;
    dfmi.initialize(8, 0);
    std::cout << "Building" << std::endl;
    dfmi.build(bwt, sa);
    std::cout << "Expeperiment" << std::endl;

    uint8_t fst_char = text[0];    
    uint64_t x = 0;
    for(uint64_t i = 1; i<bwt.size();i++){
        uint64_t p = dfmi.insert_char(i, fst_char);
        x += p;
        dfmi.delete_char(i);

        if(i % 10000 == 1){
            std::cout << "[" <<i << "/" << bwt.size() << "]" << " Total = " << x << ", Average = " << (x / i) << std::endl;

        }

    }
    std::cout << "Total: " << x << std::endl;
    std::cout << "Average: " << (x/ bwt.size()) << std::endl;
    */




    /*
     std::string str = "ababbbababa";
     std::vector<uint8_t> text = to_vector(str);


     stool::drfmi::DynamicBWT dbwt;
     dbwt.wavelet_tree.push_many(255, str);
     for(uint64_t i = 0;i < dbwt.wavelet_tree.size();i++){
         std::cout << (char)dbwt.wavelet_tree[i];

     }
     std::cout << std::endl;
     */

    // insert_test(80, 4);
}

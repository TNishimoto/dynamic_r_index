#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
#include "../include/old_implementations/btree_include.hpp"
#include "include/dynamic_fm_index/naive_dynamic_string.hpp"
#include "include/dynamic_fm_index/dynamic_fm_index.hpp"

#include <filesystem>

std::vector<uint8_t> get_chars(std::vector<uint8_t> &text)
{
    std::unordered_set<uint8_t> char_set;
    for (uint64_t x = 0; x < text.size() - 1; x++)
    {
        char_set.insert(text[x]);
    }
    std::vector<uint8_t> chars;
    for (auto c : char_set)
    {
        chars.push_back(c);
    }
    return chars;
}

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
    p.add<int>("mode", 'm', "mode", true);
    p.add<int>("number_of_trials", 'n', "number_of_trials", false, 10000);

    p.parse_check(argc, argv);
    std::string filename = p.get<std::string>("input_file");
    int mode = p.get<int>("mode");
    uint64_t numberOfTrials = p.get<int>("number_of_trials");

    std::ifstream tmpInputStream;
    tmpInputStream.open(filename, std::ios::binary);
    std::vector<uint8_t> text;

    stool::IO::load(tmpInputStream, text);
    text.push_back(0);
    int64_t n = text.size();

    std::vector<uint64_t> sa = stool::construct_suffix_array(text);
    std::vector<uint8_t> bwt = stool::construct_BWT(text, sa);
    std::vector<uint8_t> chars = get_chars(text);

    stool::r_index::DynamicFMIndex dfmi;
    dfmi.initialize(bwt, 8);
    std::cout << "Building" << std::endl;
    dfmi.build(bwt, sa);
    std::cout << "Experiment" << std::endl;

    // uint64_t try_limit = 10000;

    int64_t per = text.size() / numberOfTrials;

    uint64_t insert_move_count = 0;
    uint64_t delete_move_count = 0;

    std::mt19937_64 mt64(n);
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, chars.size() - 1);

    for (int64_t x = n - 1; x >= 1; x -= 10)
    {
        uint8_t character = chars[get_rand_uni_int(mt64)];
        std::cout << "insert: " << x << "/" << (char)character << std::endl;
        uint64_t p = dfmi.insert_char(x, character);
        uint64_t insert_shift = p;
        insert_move_count += insert_shift;

        std::cout << "delete: " << x << std::endl;
        uint64_t q = dfmi.delete_char(x);
        uint64_t delete_shift = q;

        delete_move_count += delete_shift;
    }


    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << filename << std::endl;
    std::cout << "Text length: \t" << n << std::endl;
    std::cout << "Mode: \t" << (mode == 0 ? "seq_pos" : "rand_pos") << std::endl;

    std::cout << "Insertion move count (total) \t \t" << insert_move_count << std::endl;
    std::cout << "Deletion move count (total) \t \t" << delete_move_count << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

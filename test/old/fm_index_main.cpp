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
#include "include/dynamic_fm_index/naive_dynamic_string.hpp"
#include "include/dynamic_fm_index/dynamic_fm_index.hpp"
#include "include/time_debug.hpp"

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
    std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa);
    std::vector<uint8_t> chars = get_chars(text);

    stool::drfmi::DynamicFMIndex dfmi;
    dfmi.initialize(bwt, 16);
    std::cout << "Building" << std::endl;
    dfmi.build(bwt, sa);
    std::cout << "Experiment" << std::endl;

    // uint64_t try_limit = 10000;

    std::chrono::system_clock::time_point insert_start, insert_end, delete_end;
    int64_t per = std::max((int64_t)1, (int64_t)(text.size() / numberOfTrials));

    uint64_t total_insertion_time = 0;
    uint64_t insertion_max_time = 0;
    uint64_t insertion_num = 0;

    uint64_t total_deletion_time = 0;
    uint64_t deletion_max_time = 0;

    uint64_t insert_move_count = 0;
    uint64_t insert_move_max_count = 0;

    uint64_t delete_move_count = 0;
    uint64_t delete_move_max_count = 0;

    uint64_t tmp_total_insertion_time = 0;
    uint64_t tmp_insertion_max_time = 0;
    uint64_t tmp_insert_move_count = 0;
    uint64_t tmp_insert_move_max_count = 0;

    std::mt19937_64 mt64(n);
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, chars.size() - 1);

    std::vector<uint64_t> insert_pos_vec;
    if (mode == 0)
    {
        for (int64_t x = n - 1; x >= 1; x -= per)
        {
            insert_pos_vec.push_back(x);
        }
    }
    else
    {
        std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, n - 2);
        for (uint64_t i = 0; i < numberOfTrials; i++)
        {
            insert_pos_vec.push_back(get_rand_uni_pos(mt64));
        }
    }

    std::cout << "Check BWT" << std::endl;
    std::vector<uint8_t> testBWT1 = dfmi.get_bwt();

    stool::equal_check(bwt, testBWT1);

    stool::TimeDebug::clear();

    for (uint64_t x : insert_pos_vec)
    {

        insert_start = std::chrono::system_clock::now();
        uint8_t character = chars[get_rand_uni_int(mt64)];
        uint64_t p = dfmi.insert_char(x, character);
        uint64_t insert_shift = p > 0 ? (p - 1) : p;
        insert_move_count += insert_shift;
        insert_move_max_count = std::max(insert_move_max_count, insert_shift);
        insert_end = std::chrono::system_clock::now();
        uint64_t insert_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(insert_end - insert_start).count();


        uint64_t q = dfmi.delete_char(x);
        uint64_t delete_shift = q > 0 ? (q - 1) : q;

        delete_move_count += delete_shift;
        delete_move_max_count = std::max(delete_move_max_count, delete_shift);

        delete_end = std::chrono::system_clock::now();
        uint64_t delete_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(delete_end - insert_end).count();


        total_insertion_time += insert_elapsed;
        total_deletion_time += delete_elapsed;
        insertion_max_time = std::max(insertion_max_time, (uint64_t)insert_elapsed);
        deletion_max_time = std::max(deletion_max_time, (uint64_t)delete_elapsed);



        tmp_total_insertion_time += insert_elapsed;
        tmp_insertion_max_time = std::max(tmp_insertion_max_time, (uint64_t)insert_elapsed);
        tmp_insert_move_count += insert_shift;
        tmp_insert_move_max_count = std::max(tmp_insert_move_max_count, insert_shift);

        insertion_num++;

        if (insertion_num % 100 == 0)
        {
            std::cout << std::endl;
            std::cout << "Process Insertion: " << (numberOfTrials - insertion_num) << " ---------------------" << std::endl;
            std::cout << "Insertion time (average): \t \t" << (tmp_total_insertion_time / (100 * 1000)) << "[ms]" << std::endl;
            std::cout << "Insertion time (max): \t \t \t" << tmp_insertion_max_time / 1000 << "[ms]" << std::endl;
            std::cout << "Insertion move count (total): \t \t" << tmp_insert_move_count << std::endl;
            std::cout << "Insertion move count (average): \t" << (tmp_insert_move_count / 100) << std::endl;
            std::cout << "Insertion move count (max): \t \t" << tmp_insert_move_max_count << std::endl;

            tmp_total_insertion_time = 0;
            tmp_insertion_max_time = 0;
            tmp_insert_move_count = 0;
            tmp_insert_move_max_count = 0;
        }
        else
        {
            std::cout << "+" << std::flush;
        }
    }

    std::cout << "Check BWT" << std::endl;
    std::vector<uint8_t> testBWT2 = dfmi.get_bwt();
    stool::equal_check(bwt, testBWT2);

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << filename << std::endl;
    std::cout << "Text length: \t" << n << std::endl;
    std::cout << "Mode: \t" << (mode == 0 ? "seq_pos" : "rand_pos") << std::endl;

    // time_print("Indexing time: \t \t", "chars", index_elapsed, n);
    std::cout << "Insertion time (total) \t \t" << (total_insertion_time / 1000) << "[ms]" << std::endl;
    std::cout << "Insertion time (max) \t \t" << (insertion_max_time / 1000) << "[ms]" << std::endl;
    std::cout << "Insertion time (average) \t" << (total_insertion_time / (insertion_num * 1000)) << "[ms]" << std::endl;
    std::cout << "Deletion time (total) \t \t" << (total_deletion_time / 1000) << "[ms]" << std::endl;
    std::cout << "Deletion time (max) \t \t" << (deletion_max_time / 1000) << "[ms]" << std::endl;
    std::cout << "Deletion time (average) \t" << (total_deletion_time / (insertion_num * 1000)) << "[ms]" << std::endl;
    std::cout << std::endl;
    std::cout << "Insertion move count (total) \t \t" << insert_move_count << std::endl;
    std::cout << "Insertion move count (max) \t \t" << insert_move_max_count << std::endl;
    std::cout << "Insertion move count (average) \t \t" << (insert_move_count / insertion_num) << std::endl;
    std::cout << "Deletion move count (total) \t \t" << delete_move_count << std::endl;
    std::cout << "Deletion move count (max) \t \t" << delete_move_max_count << std::endl;
    std::cout << "Deletion move count (average) \t \t" << (delete_move_count / insertion_num) << std::endl;

    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;

#ifdef TIME_DEBUG
    std::cout << "WT update time  \t" << (stool::wavelet_tree_update_time / 1000) << std::endl;
    std::cout << "WT update count  \t" << stool::wavelet_tree_update_count << std::endl;
    std::cout << "ISA update time  \t" << (stool::isa_update_time / 1000) << std::endl;
    std::cout << "ISA update count  \t" << stool::isa_update_count << std::endl;
    std::cout << "C update time  \t \t" << (stool::C_update_time / 1000) << std::endl;
    std::cout << "C update count  \t" << stool::C_update_count << std::endl;
    std::cout << "LF time  \t \t" << (stool::LF_time / 1000) << std::endl;
    std::cout << "LF count  \t \t" << stool::LF_count << std::endl;
    std::cout << "pom access time  \t" << (stool::pom_access_time / 1000) << std::endl;
    std::cout << "pom access count  \t" << stool::pom_access_count << std::endl;

#endif
}

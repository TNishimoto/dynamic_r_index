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

#include "../include/time_debug.hpp"
#include "../include/dynamic_r_index/dynamic_r_index.hpp"
#include "../include/dynamic_r_index/sampling_sa_builder.hpp"

#include <filesystem>

std::pair<uint64_t, uint64_t> insert_and_delete_string(stool::r_index::DynamicRIndex &drfmi, std::mt19937_64 &mt64, uint64_t pattern_length, std::vector<std::pair<std::string, uint64_t>> &time_profiler)
{
    uint64_t text_size = drfmi.get_dynamic_rlbwt().text_size();
    assert(pattern_length + 1 < text_size);
    std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - pattern_length - 1);
    uint64_t pos1 = get_rand_uni_pos(mt64);
    uint64_t pos2 = get_rand_uni_pos(mt64);

    std::chrono::system_clock::time_point st1, st2, st3, st4;

    st1 = std::chrono::system_clock::now();
    std::vector<uint8_t> substr = drfmi.access_substring_of_text(pos1, pattern_length);
    st2 = std::chrono::system_clock::now();
    uint64_t move_count1 = drfmi.insert_string(pos2, substr);
    st3 = std::chrono::system_clock::now();
    uint64_t move_count2 = drfmi.delete_string(pos2, pattern_length);
    st4 = std::chrono::system_clock::now();

    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();
    uint64_t time3 = std::chrono::duration_cast<std::chrono::nanoseconds>(st4 - st3).count();
    time_profiler.push_back(std::pair<std::string, uint64_t>("access", time1));
    time_profiler.push_back(std::pair<std::string, uint64_t>("insert", time2));
    time_profiler.push_back(std::pair<std::string, uint64_t>("delete", time3));

    return std::pair<uint64_t, uint64_t>(move_count1, move_count2);
}

void mode1(std::string file_path, uint64_t numberOfTrials, uint64_t pattern_length, uint64_t degree, uint64_t seed)
{

    std::vector<std::pair<std::string, uint64_t>> time_profiler;
    std::chrono::system_clock::time_point st1, st2, st3;
    stool::r_index::DynamicRIndex drfmi;
    stool::print_memory_usage();

    {

        st2 = std::chrono::system_clock::now();
        drfmi.build_from_bwt_file(file_path, degree, &time_profiler);
        st3 = std::chrono::system_clock::now();
    }

    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    stool::print_memory_usage();
    std::mt19937_64 mt64(seed);
    uint64_t move_count_for_insertion = 0;
    uint64_t move_count_for_deletion = 0;

    uint64_t max_access_time = 0;
    uint64_t max_insertion_time = 0;
    uint64_t max_deletion_time = 0;
    uint64_t total_access_time = 0;
    uint64_t total_insertion_time = 0;
    uint64_t total_deletion_time = 0;

    const stool::r_index::DynamicRLBWT& dbwt = drfmi.get_dynamic_rlbwt();


    for (uint64_t i = 0; i < numberOfTrials; i++)
    {
        std::vector<std::pair<std::string, uint64_t>> time_profiler_for_test;

        std::cout << "-" << std::flush;
        std::pair<uint64_t, uint64_t> counter = insert_and_delete_string(drfmi, mt64, pattern_length, time_profiler_for_test);
        move_count_for_insertion += counter.first;
        move_count_for_deletion += counter.second;

        uint64_t access_time = time_profiler_for_test[0].second;
        uint64_t insertion_time = time_profiler_for_test[1].second;
        uint64_t deletion_time = time_profiler_for_test[2].second;
        max_access_time = std::max(max_access_time, access_time);
        max_insertion_time = std::max(max_insertion_time, insertion_time);
        max_deletion_time = std::max(max_deletion_time, deletion_time);
        total_access_time += access_time;
        total_insertion_time += insertion_time;
        total_deletion_time += deletion_time;
    }
    std::cout << std::endl;

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Input File:         " << file_path << std::endl;
    std::cout << "Text Length:        " << dbwt.text_size() << std::endl;
    std::cout << "Number of BWT Runs: " << dbwt.run_count() << std::endl;
    std::cout << "Degree:             " << degree << std::endl;
    std::cout << "Seed:               " << seed << std::endl;

    // std::cout << "Loading Time for BWT:  " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / drfmi.dbwt.text_size()) << "[ns/char])" << std::endl;
    std::cout << "Construction Time of R-index:  " << (time2 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / dbwt.text_size()) << "[ns/char])" << std::endl;

    for (auto &it : time_profiler)
    {
        std::cout << it.first << ": " << (it.second / (1000 * 1000)) << "[ms] (Avg: " << (it.second / dbwt.text_size()) << "[ns/char])" << std::endl;
    }

    std::cout << "Total Access time: " << (total_access_time / (1000 * 1000)) << "[ms] (Avg: " << (total_access_time / numberOfTrials) << "[ns/per]), Max: " << max_access_time << "[ms]" << std::endl;
    std::cout << "Total Insertion time: " << (total_insertion_time / (1000 * 1000)) << "[ms] (Avg: " << (total_insertion_time / numberOfTrials) << "[ns/per]), Max = " << max_insertion_time << "[ns]" << std::endl;
    std::cout << "Total Deletion time: " << (total_deletion_time / (1000 * 1000)) << "[ms] (Avg: " << (total_deletion_time / numberOfTrials) << "[ns/per]), Max: " << max_deletion_time << "[ns]" << std::endl;

    std::cout << "Move Count (Insertion): " << move_count_for_insertion << " (Avg: " << (move_count_for_insertion / numberOfTrials) << ")" << std::endl;
    std::cout << "Move Count (Deletion): " << move_count_for_deletion << " (Avg: " << (move_count_for_deletion / numberOfTrials) << ")" << std::endl;
    drfmi.print_memory_usage();
    stool::print_memory_usage();

    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;

}
void mode2(std::string file_path, uint64_t degree)
{
    std::vector<std::pair<std::string, uint64_t>> time_profiler;
    std::chrono::system_clock::time_point st2, st3;
    stool::r_index::DynamicRIndex drfmi;

    {

        st2 = std::chrono::system_clock::now();
        drfmi.build_from_bwt_file(file_path, degree, &time_profiler);
        st3 = std::chrono::system_clock::now();
    }

    uint64_t time = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();

    const stool::r_index::DynamicRLBWT& dbwt = drfmi.get_dynamic_rlbwt();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Input File:         " << file_path << std::endl;
    std::cout << "Text Length:        " << dbwt.text_size() << std::endl;
    std::cout << "Degree:             " << degree << std::endl;

    std::cout << "Construction Time of R-index:  " << (time / (1000 * 1000)) << "[ms] (Avg: " << (time / dbwt.text_size()) << "[ns/char])" << std::endl;
    for (auto &it : time_profiler)
    {
        std::cout << it.first << ": " << (it.second / (1000 * 1000)) << "[ms] (Avg: " << (it.second / dbwt.text_size()) << "[ns/char])" << std::endl;
    }
    drfmi.print_memory_usage();
    stool::print_memory_usage();

    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

void mode3(std::string file_path, [[maybe_unused]] uint64_t degree)
{
    std::vector<uint8_t> bwt; 
    stool::IO::load(file_path, bwt);
    std::vector<uint8_t> chars = stool::StringFunctions::get_alphabet(bwt);

        stool::r_index::DynamicRIndex drfmi;
        drfmi.initialize(chars, false);
        drfmi.build_from_bwt_file2(file_path, 126);
        std::cout << "END" << std::endl;


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
    p.add<int>("pattern_length", 'p', "pattern_length", false, 100);
    p.add<int>("number_of_trials", 'n', "number_of_trials", false, 100);
    p.add<int>("degree", 'd', "degree", false, 64);
    p.add<int>("seed", 's', "seed", false, 0);

    p.parse_check(argc, argv);
    std::string file_path = p.get<std::string>("input_file");
    uint64_t numberOfTrials = p.get<int>("number_of_trials");
    uint64_t pattern_length = p.get<int>("pattern_length");
    uint64_t seed = p.get<int>("seed");
    uint64_t degree = p.get<int>("degree");
    uint64_t mode = p.get<int>("mode");

    if (mode == 1)
    {
        mode1(file_path, numberOfTrials, pattern_length, degree, seed);
    }
    else if(mode == 2)
    {
        mode2(file_path, degree);
    }if (mode == 3)
    {
        mode3(file_path, degree);
    }

    // int mode = p.get<int>("mode");
}

#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/dynamic_r_index/rlbwt_builder.hpp"

#include "stool/include/memory.hpp"

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
    p.add<std::string>("output_file", 'o', "output file name", true, "");

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file");
    std::string output_file_path = p.get<std::string>("output_file");

    if (output_file_path.size() == 0)
    {
        output_file_path = input_file_path + ".bwt";
    }

    std::chrono::system_clock::time_point st1, st2, st3;
    st1 = std::chrono::system_clock::now();
    stool::dynamic_r_index::DynamicRLBWT rlbwt = stool::dynamic_r_index::RLBWTBuilder::online_build_for_reversed_text(input_file_path, '$');
    st2 = std::chrono::system_clock::now();


    rlbwt.write_BWT(output_file_path);
    st3 = std::chrono::system_clock::now();

    uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();
    uint64_t time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(st3 - st2).count();    

    /*
    if(true){
        rlbwt.verify();
        std::cout << "Text: " << rlbwt.get_text_str() << std::endl;
    }
    */
    


    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Input File:         " << input_file_path << std::endl;
    std::cout << "Output File:        " << output_file_path << std::endl;
    std::cout << "Text Length:        " << rlbwt.text_size() << std::endl;
    std::cout << "Number of BWT Runs: " << rlbwt.run_count() << std::endl;    
    std::cout << "Construction Time:  " << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / rlbwt.text_size()) << "[ns/char])" << std::endl;
    std::cout << "Writing Time:       " << (time2 / (1000 * 1000)) << "[ms]" << std::endl;
    #ifdef TIME_DEBUG
    std::cout << "IncrementRun:       " << stool::increment_run_count << std::endl;
    std::cout << "IncrementRun Time:  " << (stool::increment_run_time / (1000 * 1000)) << "[ms] (Avg: " << (stool::increment_run_time / stool::increment_run_count) << "[ns/op])"  << std::endl;
    #endif
    rlbwt.print_memory_usage();
    stool::print_memory_usage();

    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;

}

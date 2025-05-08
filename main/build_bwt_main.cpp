#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/dynamic_r_index/rlbwt_builder.hpp"


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

    p.add<std::string>("input_text_path", 'i', "The file path to a text", true);
    p.add<std::string>("output_bwt_path", 'o', "The path to the file where the BWT will be written", false, "");
    p.add<std::string>("null_terminated_string", 'c', "The special character indicating the end of text", false, "\\0");
    p.add<uint>("detailed_message_flag", 'e', "The value is 1 if detailed messages are printed, and 0 otherwise", false, 0);

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_text_path");
    std::string output_file_path = p.get<std::string>("output_bwt_path");
    std::string tmp_null_terminated_string = p.get<std::string>("null_terminated_string");
    uint8_t null_terminated_string = stool::DebugPrinter::get_first_character(tmp_null_terminated_string);

    //uint8_t null_terminated_string = p.get<std::string>("null_terminated_string")[0];
    uint detailed_message_flag = p.get<uint>("detailed_message_flag");

    if (output_file_path.size() == 0)
    {
        output_file_path = input_file_path + ".bwt";
    }

    std::chrono::system_clock::time_point st1, st2, st3;

    std::vector<uint8_t> text;
    stool::IO::load(input_file_path, text);

    //bool null_terminated_string_check = true;
    for(uint i = 0; i < text.size(); i++){
        if(text[i] == null_terminated_string){
            throw std::runtime_error("The input text must not contain the null terminated string: " + stool::DebugPrinter::to_visible_string(null_terminated_string));
        }
        if(text[i] < null_terminated_string){
            throw std::runtime_error("The null terminated string must be smallest among all characters in the input text: " + stool::DebugPrinter::to_visible_string(null_terminated_string));
        }
    }
    

    st1 = std::chrono::system_clock::now();
    stool::dynamic_r_index::DynamicRLBWT rlbwt = stool::dynamic_r_index::RLBWTBuilder::build(text, null_terminated_string);
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
    std::cout << "Input File: \t\t" << input_file_path << std::endl;
    std::cout << "Text Length: \t\t" << rlbwt.text_size() << std::endl;
    if(text.size() < 1000){
       std::string view_text = stool::DebugPrinter::to_string(text);
       view_text.push_back(null_terminated_string);

        std::cout << "Text: \t\t\t" << stool::DebugPrinter::to_visible_string(view_text) << std::endl;
    }else{
        std::cout << "Text: \t\t\t" << "[Omitted]" << std::endl;
    }
    std::cout << "Null terminated string: " << stool::DebugPrinter::to_visible_string(null_terminated_string) << std::endl;


    std::cout << "Output File: \t\t" << output_file_path << std::endl;
    if(text.size() < 1000){
        std::vector<uint8_t> bwt;
        stool::IO::load(output_file_path, bwt);
        std::string bwt_str = stool::DebugPrinter::to_string(bwt);

        std::cout << "BWT: \t\t\t" << stool::DebugPrinter::to_visible_string(bwt_str) << std::endl;
    }else{
        std::cout << "BWT: \t\t\t" << "[Omitted]" << std::endl;
    }
    std::cout << "Number of BWT Runs: \t" << rlbwt.run_count() << std::endl;    


    std::cout << "Construction Time: \t" << (time1 / (1000 * 1000)) << "[ms] (Avg: " << (time1 / rlbwt.text_size()) << "[ns/char])" << std::endl;
    std::cout << "Writing Time: \t\t" << (time2 / (1000 * 1000)) << "[ms]" << std::endl;
    #ifdef TIME_DEBUG
    std::cout << "IncrementRun:       " << stool::increment_run_count << std::endl;
    std::cout << "IncrementRun Time:  " << (stool::increment_run_time / (1000 * 1000)) << "[ms] (Avg: " << (stool::increment_run_time / stool::increment_run_count) << "[ns/op])"  << std::endl;
    #endif
    if(detailed_message_flag == 1){
        rlbwt.print_memory_usage();
    }
    stool::print_memory_usage();

    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;

}

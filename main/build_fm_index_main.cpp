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

    p.add<std::string>("input_file_path", 'i', "The file path to either a text or a BWT", true);
    p.add<std::string>("output_file_path", 'o', "The path to the file where the dynamic FM-index will be written", false, "");
    p.add<std::string>("null_terminated_string", 'c', "The special character indicating the end of text", false, "\\0");
    p.add<uint>("is_bwt", 'u', "This value is 1 if the input file is a BWT, and 0 otherwise", false, 0);
    p.add<uint>("sampling_interval", 's', "The sampling interval for the suffix array", false, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL);

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file_path");
    std::string output_file_path = p.get<std::string>("output_file_path");
    uint text_type = p.get<uint>("is_bwt");
    uint sampling_interval = p.get<uint>("sampling_interval");
    std::string tmp_null_terminated_string = p.get<std::string>("null_terminated_string");
    uint8_t null_terminated_string = stool::DebugPrinter::get_first_character(tmp_null_terminated_string);

    uint is_bwt = 1;

    if (output_file_path.size() == 0)
    {
        output_file_path = input_file_path + "." + std::to_string(sampling_interval) + ".dfmi";
    }
    std::ofstream os;
    os.open(output_file_path, std::ios::binary);
    if (!os)
    {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        throw std::runtime_error("File open error");
    }

    // uint is_r_index = 1;

    std::chrono::system_clock::time_point st1, st2;
    st1 = std::chrono::system_clock::now();

    stool::dynamic_r_index::DynamicFMIndex dfmi;
    if (text_type == is_bwt)
    {
        std::vector<uint8_t> bwt;
        stool::IO::load_text(input_file_path, bwt);
        std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(bwt);
        stool::dynamic_r_index::DynamicFMIndex tmp_dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet, sampling_interval, stool::Message::SHOW_MESSAGE);
        dfmi.swap(tmp_dfmi);
    }
    else
    {
        std::vector<uint8_t> text;
        stool::IO::load_text(input_file_path, text, true, null_terminated_string);
        std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(text);

        std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::SHOW_MESSAGE);
        std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::SHOW_MESSAGE);
        std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::SHOW_MESSAGE);
        stool::dynamic_r_index::DynamicFMIndex tmp_dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet, isa, sampling_interval, stool::Message::SHOW_MESSAGE);
        dfmi.swap(tmp_dfmi);
    }

    //dfmi.print_memory_usage();
    //dfmi.print_statistics();

    stool::dynamic_r_index::DynamicFMIndex::save(dfmi, os);
    os.close();

    st2 = std::chrono::system_clock::now();
    uint64_t text_size = dfmi.size();


    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Input File: \t\t\t\t\t" << input_file_path << std::endl;
    std::cout << "Output File: \t\t\t\t\t" << output_file_path << std::endl;
    std::cout << "The type of the input file: \t\t\t" << (text_type == is_bwt ? "bwt" : "text") << std::endl;
    dfmi.print_light_statistics();

    if (text_size > 0)
    {
        uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
        uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
        uint64_t per_time = text_size > 0 ? (((double)ms_time / (double)text_size) * 1000000) : ms_time;
        std::cout << "Total time: \t\t\t\t\t" << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
    }
    stool::print_memory_usage();
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;

    dfmi.clear(stool::Message::SHOW_MESSAGE);
}

/**
 * @file build_r_index_main.cpp
 * @brief Build dynamic r-index from text or BWT file
 * 
 * This program constructs a dynamic r-index data structure from either
 * a raw text file or a pre-computed BWT file. The resulting index is
 * saved to a binary file (.dri format).
 * 
 * Usage:
 *   ./build_r_index -i input.txt -o output.dri
 *   ./build_r_index -i input.bwt -o output.dri -u 1
 * 
 * @author TNishimoto
 * @date 2025
 * @license MIT
 */

#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/lib.hpp"
#include "../include/all.hpp"
#include "libdivsufsort/sa.hpp"

/**
 * @brief Extract the first character from a string, handling escape sequences
 * @param text The input string
 * @param default_character Default character to return if string is empty
 * @return The first character value (handles \n, \0, etc. as escape sequences)
 */
uint8_t get_first_character(const std::string &text, uint8_t default_character = '\0'){
    if(text.size() == 0){
        return default_character;
    }
    if(text[0] == '\\' && text.size() > 1){
        auto tmp = text.substr(1);
        return std::stoi(tmp);
    }else{
        return text[0];
    }

}

/**
 * @brief Main function to build dynamic r-index
 * @param argc Number of command-line arguments
 * @param argv Command-line arguments
 * @return Exit code (0 on success)
 */
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

    p.add<std::string>("input_file_path", 'i', "Input file path (text or BWT)", true);
    p.add<std::string>("output_index_path", 'o', "Output index file path (.dri)", false, "");
    p.add<std::string>("null_terminated_string", 'c', "End-of-text character", false, "\\0");
    p.add<uint>("is_bwt", 'u', "Set to 1 if input is BWT", false, 0);

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file_path");
    std::string output_file_path = p.get<std::string>("output_index_path");
    std::string tmp_null_terminated_string = p.get<std::string>("null_terminated_string");
    uint8_t null_terminated_string = get_first_character(tmp_null_terminated_string);
    uint text_type = p.get<uint>("is_bwt");

    uint IS_BWT = 1;

    if (output_file_path.size() == 0)
    {
        output_file_path = input_file_path + ".drfmi";
    }
    std::ofstream os;
    os.open(output_file_path, std::ios::binary);
    if (!os)
    {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        throw std::runtime_error("File open error");
    }


    std::chrono::system_clock::time_point st1, st2;
    st1 = std::chrono::system_clock::now();
    // uint is_r_index = 1;
    stool::dynamic_r_index::DynamicRIndex drfmi;
    if (text_type == IS_BWT)
    {
        stool::dynamic_r_index::DynamicRIndex tmp_drfmi = stool::dynamic_r_index::DynamicRIndex::build_from_BWT_file(input_file_path, stool::Message::SHOW_MESSAGE);
        drfmi.swap(tmp_drfmi);
    }
    else
    {
        std::vector<uint8_t> text;
        stool::FileReader::load_vector_with_end_marker_if_no_end_marker(input_file_path, text, null_terminated_string);

        std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(text);

        std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::SHOW_MESSAGE);
        std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::SHOW_MESSAGE);
        std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::SHOW_MESSAGE);
        stool::dynamic_r_index::DynamicRIndex tmp_drfmi = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet, stool::Message::SHOW_MESSAGE);

        drfmi.swap(tmp_drfmi);
    }

    //drfmi.print_memory_usage();
    //drfmi.print_statistics();

    stool::dynamic_r_index::DynamicRIndex::store_to_file(drfmi, os);
    os.close();

    st2 = std::chrono::system_clock::now();
    uint64_t text_size = drfmi.size();

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Input File: \t\t\t\t\t" << input_file_path << std::endl;
    std::cout << "Output File: \t\t\t\t\t" << output_file_path << std::endl;
    std::cout << "The type of the input file: \t\t\t" << (text_type == IS_BWT ? "bwt" : "text") << std::endl;
    drfmi.print_light_statistics();
    if (text_size > 0)
    {
        uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
        uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
        uint64_t per_time = text_size > 0 ? (((double)ms_time / (double)text_size) * 1000000) : ms_time;
        std::cout << "Total time: \t\t\t\t\t" << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
    }
    stool::Memory::print_memory_usage();
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;

    drfmi.clear();

}

#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/drfmi.hpp"
#include "libdivsufsort/sa.hpp"

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
    p.add<std::string>("output_file", 'o', "output file name", false, "");
    // p.add<uint>("index_type", 'm', "dynamic r-index or dynamic fm index", true, 1);
    p.add<uint>("text_type", 'u', "input type", false, 0);
    p.add<bool>("lightweight", 'w', "lightweight", false, true);
    p.add<uint>("sampling_interval", 's', "sampling_interval", false, stool::fm_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL);

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file");
    std::string output_file_path = p.get<std::string>("output_file");
    // bool isLightWeight = p.get<bool>("lightweight");
    //  uint index_type = p.get<uint>("index_type");
    uint text_type = p.get<uint>("text_type");
    uint sampling_interval = p.get<uint>("sampling_interval");

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

    stool::fm_index::DynamicFMIndex dfmi;
    if (text_type == is_bwt)
    {
        std::vector<uint8_t> bwt;
        stool::IO::load_text(input_file_path, bwt);
        std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(bwt);
        stool::fm_index::DynamicFMIndex tmp_dfmi = stool::fm_index::DynamicFMIndex::build(bwt, alphabet, sampling_interval, stool::Message::SHOW_MESSAGE);
        dfmi.swap(tmp_dfmi);
    }
    else
    {
        std::vector<uint8_t> text;
        stool::IO::load_text(input_file_path, text, true);
        std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(text);

        std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::SHOW_MESSAGE);
        std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::SHOW_MESSAGE);
        std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::SHOW_MESSAGE);
        stool::fm_index::DynamicFMIndex tmp_dfmi = stool::fm_index::DynamicFMIndex::build(bwt, alphabet, isa, sampling_interval, stool::Message::SHOW_MESSAGE);
        dfmi.swap(tmp_dfmi);
    }

    //dfmi.print_memory_usage();
    //dfmi.print_statistics();

    stool::fm_index::DynamicFMIndex::save(dfmi, os);
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

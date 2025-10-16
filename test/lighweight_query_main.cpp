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

    p.add<std::string>("text", 'i', "text", true);
    p.add<std::string>("index_type", 't', "index_type", true);
    p.add<std::string>("query_type", 'q', "query file", true);
    p.add<std::string>("pattern", 'p', "pattern", false, "");
    p.add<uint64_t>("position", 'j', "position", false, 0);

    p.parse_check(argc, argv);
    std::string text = p.get<std::string>("text");
    char index_type = p.get<std::string>("index_type")[0];
    char query_type = p.get<std::string>("query_type")[0];
    std::string pattern = p.get<std::string>("pattern");
    uint64_t position = p.get<uint64_t>("position");

    std::vector<uint8_t> text_vec;
    for (auto c : text)
    {
        text_vec.push_back(c);
    }
    std::vector<uint8_t> pattern_vec;
    for (auto c : pattern)
    {
        pattern_vec.push_back(c);
    }

    std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text_vec, stool::Message::NO_MESSAGE);
    std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
    std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text_vec, sa, stool::Message::NO_MESSAGE);

    std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(text_vec);

    if (index_type == 'f')
    {
        std::cout << "FM-index" << std::endl;
        std::cout << "pattern: " << pattern << std::endl;
        std::cout << "position: " << position << std::endl;
        std::cout << "query_type: " << query_type << std::endl;
        std::cout << "text: " << text << std::endl;

        stool::dynamic_r_index::DynamicFMIndex dyn_index = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet, 8, stool::Message::NO_MESSAGE);
        if (query_type == 'I')
        {
                stool::dynamic_r_index::FMIndexEditHistory output_history;

            if (pattern.size() == 1)
            {
                dyn_index.insert_char(position, pattern[0], output_history);
            }
            else if (pattern.size() > 1)
            {
                dyn_index.insert_string(position, pattern_vec, output_history);
            }
            output_history.print();

            std::string text_str =  dyn_index.get_text_str();
            std::cout << "text_str: " << text_str << std::endl;
        }
        else if (query_type == 'D')
        {
        }
    }
}

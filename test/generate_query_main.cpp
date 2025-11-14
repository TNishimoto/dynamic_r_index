#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/all.hpp"
#include "libdivsufsort/sa.hpp"

bool check_run(const std::vector<uint8_t> &text, uint64_t i, uint64_t len){
    bool b = true;
    for(uint64_t x = 1; x < len;x++){
        if(text[i + x] != text[i + x -1]){
            b = false;
        }
    }
    return b;
}

void generate_random_update_queries(const std::vector<uint8_t> &text, std::ofstream &os, uint64_t pattern_length, uint64_t number_of_patterns, int mode, std::ofstream *additional_os, uint64_t seed)
{
    std::mt19937_64 mt64(seed);
    std::vector<uint8_t> alphabet = stool::StringFunctions::get_alphabet(text);
    // stool::DebugPrinter::print_characters(alphabet);
    uint64_t text_size = text.size();

    if (additional_os != nullptr && (mode == 3 || mode == 4))
    {
        (*additional_os) << "# number=" << number_of_patterns << " length=" << pattern_length << " file=/home/dummy/txt forbidden " << std::endl;
    }

    for (uint8_t c : alphabet)
    {
        if (c == 0)
        {
            throw std::runtime_error("This input file contains character NUL(0)");
        }
        else if (c == 1)
        {
            throw std::runtime_error("This input file contains character SOH(1)");
        }
    }

    std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size);
    std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, alphabet.size() - 1);
    std::uniform_int_distribution<uint64_t> get_rand_uni_substr(0, text_size - pattern_length);

    for (uint64_t i = 0; i < number_of_patterns; i++)
    {
        std::cout << "+" << std::flush;
        if (mode == 0)
        {
            std::string pattern;
            uint64_t insertion_pos = get_rand_uni_pos(mt64);
            uint64_t substr_pos = get_rand_uni_substr(mt64);

            for (uint64_t x = 0; x < pattern_length; x++)
            {
                uint8_t c = text[substr_pos + x];
                if (c == '\n')
                {
                    c = 0;
                }
                else if (c == '\t')
                {
                    c = 1;
                }
                pattern.push_back(c);
            }
            os << "INSERT\t" << insertion_pos << "\t" << pattern << std::endl;
            os << "DELETE\t" << insertion_pos << "\t" << pattern_length << std::endl;
        }
        else if (mode == 1)
        {
            std::string pattern;
            uint64_t insertion_pos = get_rand_uni_pos(mt64);

            for (uint64_t x = 0; x < pattern_length; x++)
            {
                uint8_t c = alphabet[get_rand_uni_char(mt64)];
                if (c == '\n')
                {
                    c = 0;
                }
                else if (c == '\t')
                {
                    c = 1;
                }
                pattern.push_back(c);
            }
            os << "INSERT\t" << insertion_pos << "\t" << pattern << std::endl;
            os << "DELETE\t" << insertion_pos << "\t" << pattern_length << std::endl;
        }
        else if (mode == 2)
        {
            std::string pattern;
            uint64_t substr_pos = get_rand_uni_substr(mt64);

            for (uint64_t x = 0; x < pattern_length; x++)
            {
                uint8_t c = text[substr_pos + x];
                if (c == '\n')
                {
                    c = 0;
                }
                else if (c == '\t')
                {
                    c = 1;
                }
                pattern.push_back(c);
            }
            os << "DELETE\t" << substr_pos << "\t" << pattern_length << std::endl;
            os << "INSERT\t" << substr_pos << "\t" << pattern << std::endl;
        }
        else if (mode == 3)
        {
            std::string pattern;
            std::string pattern2;

            // uint64_t insertion_pos = get_rand_uni_pos(mt64);
            uint64_t substr_pos = get_rand_uni_substr(mt64);

            for (uint64_t x = 0; x < pattern_length; x++)
            {
                uint8_t c = text[substr_pos + x];
                pattern2.push_back(c);
                if (c == '\n')
                {
                    c = 0;
                }
                else if (c == '\t')
                {
                    c = 1;
                }
                pattern.push_back(c);
            }
            os << "COUNT\t"  << pattern << std::endl;
            if (additional_os != nullptr)
            {
                (*additional_os) << pattern2;
            }
        }
        else if (mode == 4)
        {
            std::string pattern;
            std::string pattern2;
            // uint64_t insertion_pos = get_rand_uni_pos(mt64);
            uint64_t substr_pos = get_rand_uni_substr(mt64);

            uint64_t counter = 0;
            while (check_run(text, substr_pos, pattern_length))
            {
                substr_pos = get_rand_uni_substr(mt64);
                counter++;
                if(counter > 100){
                    throw std::runtime_error("Error: generate_random_update_queries");
                }
            }
            

            for (uint64_t x = 0; x < pattern_length; x++)
            {
                uint8_t c = text[substr_pos + x];
                pattern2.push_back(c);
                if (c == '\n')
                {
                    c = 0;
                }
                else if (c == '\t')
                {
                    c = 1;
                }
                pattern.push_back(c);
            }
            os << "LOCATE\t"  << pattern << std::endl;
            if (additional_os != nullptr)
            {
                (*additional_os) << pattern2;
            }
        }
    }
    std::cout << std::endl;
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

    p.add<std::string>("input_file", 'i', "input index name", true);
    p.add<std::string>("output_file", 'o', "output index name", false, "");
    p.add<std::string>("additional_output_path", 'a', "additional output path", false, "");

    // p.add<uint>("index_type", 'm', "dynamic r-index or dynamic fm index", true, 1);
    p.add<uint64_t>("pattern_length", 'm', "pattern_length", false, 1);
    p.add<uint64_t>("number_of_patterns", 'k', "number_of_patterns", false, 100);
    p.add<uint>("mode", 'w', "mode", true);

    p.add<uint64_t>("seed", 's', "seed", false, 0);

    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file");
    std::string output_file_path = p.get<std::string>("output_file");
    std::string additional_output_path = p.get<std::string>("additional_output_path");

    uint64_t pattern_length = p.get<uint64_t>("pattern_length");
    uint64_t number_of_patterns = p.get<uint64_t>("number_of_patterns");
    uint64_t seed = p.get<uint64_t>("seed");
    uint mode = p.get<uint>("mode");

    if (mode > 4)
    {
        throw std::runtime_error("mode must be smaller than 5");
    }

    // bool isLightWeight = p.get<bool>("lightweight");
    //  uint index_type = p.get<uint>("index_type");

    std::vector<uint8_t> text;
    stool::FileReader::load_vector(input_file_path, text);
    /*
    if (output_file_path.size() == 0)
    {
        output_file_path = input_file_path + ".update.tsv";
    }
    */

    {
        std::ofstream additional_os;
        bool b = additional_output_path.size() > 0;
        std::ofstream* _additional_os = nullptr;
        if (b)
        {
            additional_os.open(additional_output_path);
            _additional_os = &additional_os; 
        }

        if (output_file_path.size() > 0)
        {
            std::ofstream os;
            os.open(output_file_path);
            if (!os)
            {
                std::cerr << "Error: Could not open file for writing." << std::endl;
                throw std::runtime_error("File open error");
            }

            

            generate_random_update_queries(text, os, pattern_length, number_of_patterns, mode, _additional_os, seed);
        }
    }

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Input File: " << input_file_path << std::endl;
    std::cout << "Output File: " << output_file_path << std::endl;
    std::cout << "The length of each pattern: " << pattern_length << std::endl;
    std::cout << "The nummber of patterns: " << (number_of_patterns * 2) << std::endl;
    if (mode == 0)
    {
        std::cout << "Type: " << "Random Insertion of Random Substrings" << std::endl;
    }
    else if (mode == 1)
    {
        std::cout << "Type: " << "Random Insertion of Random Patterns" << std::endl;
    }
    else if (mode == 2)
    {
        std::cout << "Type: " << "Random Deletion of Random Substrings" << std::endl;
    }
    else if (mode == 3)
    {
        std::cout << "Type: " << "Random Count Queries" << std::endl;
    }
    else if (mode == 4)
    {
        std::cout << "Type: " << "Random Locate Queries" << std::endl;
    }

    std::cout << "Seed: " << seed << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

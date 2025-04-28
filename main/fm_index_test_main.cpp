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

//#include "../include/old_implementations/btree_include.hpp"
#include "../test/dynamic_fm_index_test.hpp"
#include "../test/dynamic_r_index_test.hpp"
//

#include <filesystem>

void fm_index_test(uint64_t text_size, uint64_t mode, bool detailed_check, uint64_t seed)
{
    uint64_t trial_num = 10;
    if (mode == 1)
    {
        std::cout << "character_insertion_and_deletion_test" << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < trial_num; i++)
            {
                std::cout << alphabet_type << std::flush;

                stool::fm_index_test::DynamicFMIndexTest::character_insertion_and_deletion_test(text_size, alphabet_type, detailed_check, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 2)
    {
        std::cout << "string_insertion_and_deletion_test" << std::endl;

        uint64_t length_of_insertion_string = 2;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
            {
                for (uint64_t i = 0; i < trial_num; i++)
                {
                    std::cout << alphabet_type << std::flush;

                    stool::fm_index_test::DynamicFMIndexTest::string_insertion_and_deletion_test(text_size, length_of_insertion_string, alphabet_type, seed++);
                }
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if(mode == 3)
    {
        std::cout << "backward_search_test" << std::endl;

        uint64_t seed = 0;
        uint64_t pattern_size = 100;
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < trial_num; i++)
            {
                std::cout << alphabet_type << std::flush;

                stool::fm_index_test::DynamicFMIndexTest::backward_search_test(text_size, pattern_size, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if(mode == 4)
    {
        std::cout << "sampled_isa_test" << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < trial_num; i++)
            {
                std::cout << alphabet_type << std::flush;

                stool::fm_index_test::DynamicFMIndexTest::sampled_isa_test(text_size, alphabet_type, detailed_check, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if(mode == 5)
    {
        std::cout << "save_and_load_test" << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < trial_num; i++)
            {
                std::cout << alphabet_type << std::flush;

                stool::fm_index_test::DynamicFMIndexTest::save_and_load_test(text_size, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    
    else{
        for(uint64_t i = 1;i <= 5; i++){
            fm_index_test(text_size, i, detailed_check, seed);
        }
    }

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

    // p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<uint>("mode", 'm', "mode", false, 0);
    p.add<uint>("seed", 's', "seed", false, 0);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");
    uint64_t seed = p.get<uint>("seed");

    fm_index_test(150, mode, false, seed);
}

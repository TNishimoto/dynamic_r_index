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

// #include "../include/old_implementations/btree_include.hpp"
#include "include/dynamic_fm_index_test.hpp"
#include "include/dynamic_r_index_test.hpp"
#include "stool/test/sources/template/dynamic_string_test.hpp"


//

#include <filesystem>

using namespace stool::fm_index_test;

void fm_index_test(uint64_t text_size, uint64_t mode, bool detailed_check, uint64_t seed)
{
    std::function<std::vector<uint64_t>(const std::vector<uint8_t>&)> sa_builder_function = [](const std::vector<uint8_t>& text)
    {
        //auto sa = stool::StringFunctions::construct_naive_suffix_array(text);
        auto sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
        return sa;
    };

    std::function<bool(stool::dynamic_r_index::DynamicFMIndex &, stool::dynamic_r_index::DynamicFMIndex &)> equal_check_function = [](stool::dynamic_r_index::DynamicFMIndex& text1, stool::dynamic_r_index::DynamicFMIndex& text2)
    {
        auto bwt1 = text1.get_bwt();
        auto sa1 = text1.get_sa();
        auto isa1 = text1.get_isa();
        auto bwt2 = text2.get_bwt();
        auto sa2 = text2.get_sa();
        auto isa2 = text2.get_isa();
        stool::EqualChecker::equal_check(bwt1, bwt2, "BWT");
        stool::EqualChecker::equal_check(sa1, sa2, "SA");
        stool::EqualChecker::equal_check(isa1, isa2, "ISA");

        return true;
    };



    uint64_t number_of_trials_insert = 100;
    uint64_t number_of_trials = 10;


    uint64_t trial_num = 10;
    if (mode == 1)
    {
        //stool::StringTest::access_character_test<stool::dynamic_r_index::DynamicFMIndex>(5000, 100, 10, true, seed);
        //detailed_check = true;
        stool::DynamicStringTest::insert_character_test<stool::dynamic_r_index::DynamicFMIndex>(text_size, number_of_trials_insert, number_of_trials, true, detailed_check, seed);
        stool::DynamicStringTest::remove_character_test<stool::dynamic_r_index::DynamicFMIndex>(text_size, number_of_trials_insert, number_of_trials, true, detailed_check, seed);

    }
    else if (mode == 2)
    {
        stool::DynamicStringTest::insert_string_test<stool::dynamic_r_index::DynamicFMIndex>(text_size, number_of_trials_insert, number_of_trials, 100, true, detailed_check, seed);
        stool::DynamicStringTest::delete_string_test<stool::dynamic_r_index::DynamicFMIndex>(text_size, number_of_trials_insert, number_of_trials, 100, true, detailed_check, seed);
    }
    else if (mode == 3)
    {
        stool::StringTest::locate_query_test<stool::dynamic_r_index::DynamicFMIndex>(text_size, 0, 100, 10, sa_builder_function, true, seed);

    }
    else if (mode == 4)
    {
        std::cout << "sampled_isa_test" << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < trial_num; i++)
            {
                std::cout << alphabet_type << std::flush;

                DynamicFMIndexTest::sampled_isa_test(text_size, alphabet_type, detailed_check, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 5)
    {
        std::string filepath = "dfmi.bits";
        stool::StringTest::save_and_load_test<stool::dynamic_r_index::DynamicFMIndex>(text_size, trial_num, equal_check_function, true, filepath, seed);


        /*
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < trial_num; i++)
            {
                std::cout << alphabet_type << std::flush;

                DynamicFMIndexTest::save_and_load_test(text_size, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
        */
    }
    else if (mode == 6)
    {
        std::cout << "LF test" << std::endl;
        uint64_t length_of_insertion_string = 2;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
            {
                uint64_t length_of_text = 10;

                while (length_of_text < text_size)
                {
                    for (uint64_t i = 0; i < trial_num; i++)
                    {
                        std::cout << alphabet_type << std::flush;

                        DynamicFMIndexTest::string_deletion_for_LF_test(length_of_text, length_of_insertion_string, alphabet_type, seed++);
                    }

                    length_of_text *= 2;
                }
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if (mode == 7)
    {
        stool::DynamicStringTest::random_test<stool::dynamic_r_index::DynamicFMIndex, stool::NaiveDynamicString, false, true>(text_size, 0, 100, 10, true, detailed_check, seed);
    }
    else
    {
        for (uint64_t i = 1; i <= 6; i++)
        {
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

    fm_index_test(5000, mode, false, seed);
}

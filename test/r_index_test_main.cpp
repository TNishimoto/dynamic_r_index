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
#include "include/dynamic_r_index_test.hpp"
//

#include <filesystem>

void main_sub(int mode, bool detailed_check, uint64_t seed)
{
    uint64_t max_alphabet_type = stool::UInt8VectorGenerator::get_max_alphabet_type();

    if (mode == 1)
    {
        std::cout << "Execute DynamicRIndexTest::random_character_insertion_and_deletion_test." << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                stool::r_index_test::DynamicRIndexTest::random_character_insertion_and_deletion_test(100, alphabet_type, seed++, detailed_check);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 2)
    {
        std::cout << "Execute DynamicRIndexTest::inverse_phi_test_without_updates." << std::endl;

        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                stool::r_index_test::DynamicRIndexTest::inverse_phi_test_without_updates(320, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 3)
    {
        std::cout << "Execute DynamicRIndexTest::F_test." << std::endl;

        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                stool::r_index_test::DynamicRIndexTest::F_test(320, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 4)
    {
        std::cout << "Execute DynamicRIndexTest::detailed_insertion_test." << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                stool::r_index_test::DynamicRIndexTest::detailed_insertion_test(320, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 5)
    {
        std::cout << "Execute DynamicRIndexTest::detailed_deletion_test." << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                stool::r_index_test::DynamicRIndexTest::detailed_deletion_test(320, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 6)
    {
        std::cout << "Execute DynamicRIndexTest::detailed_string_insertion_test." << std::endl;
        uint64_t length_of_insertion_string = 2;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
            {

                for (uint64_t i = 0; i < 10; i++)
                {
                    std::cout << alphabet_type << std::flush;
                    stool::r_index_test::DynamicRIndexTest::detailed_string_insertion_test(320, length_of_insertion_string, alphabet_type, seed++);
                }
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if (mode == 7)
    {
        std::cout << "Execute DynamicRIndexTest::detailed_string_deletion_test." << std::endl;
        uint64_t length_of_insertion_string = 1;
        for (uint64_t x = 0; x < 4; x++)
        {
            for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
            {

                for (uint64_t i = 0; i < 10; i++)
                {
                    std::cout << alphabet_type << std::flush;
                    stool::r_index_test::DynamicRIndexTest::detailed_string_deletion_test(320, length_of_insertion_string, alphabet_type, seed++);
                }
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if (mode == 8)
    {
        std::cout << "Execute RLBWTBuilder::build." << std::endl;
        uint64_t length = 8;
        std::vector<uint8_t> chars;
        chars.push_back('A');
        chars.push_back('C');
        chars.push_back('G');
        chars.push_back('T');

        uint64_t loop = 8;
        for (uint64_t x = 0; x < loop; x++)
        {
            std::cout << "(" << x << "/" << loop << ") " << std::flush;
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(length, chars, i);
                text.push_back('$');

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);
                text.pop_back();
                stool::dynamic_r_index::DynamicRLBWT rlbwt = stool::dynamic_r_index::RLBWTBuilder::build(text);

                stool::equal_check("BWT", bwt, rlbwt.get_bwt());
            }
            std::cout << std::endl;

            length *= 2;
        }
    }
    else if (mode == 9)
    {
        std::cout << "Execute DynamicRIndexTest::backward_search_test." << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {

            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;
                stool::r_index_test::DynamicRIndexTest::backward_search_test(320, 10, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 10)
    {
        std::cout << "save_and_load_test" << std::endl;
        for (uint64_t alphabet_type = 0; alphabet_type <= stool::UInt8VectorGenerator::get_max_alphabet_type(); alphabet_type++)
        {
            for (uint64_t i = 0; i < 30; i++)
            {
                std::cout << alphabet_type << std::flush;

                stool::r_index_test::DynamicRIndexTest::save_and_load_test(320, alphabet_type, seed++);
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 11)
    {
        /*
        for (uint64_t alphabet_type = 0; alphabet_type <= 0; alphabet_type++)
        {
            std::cout << alphabet_type << std::flush;
            uint64_t text_size = 200;

            std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
            std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

            uint64_t end_marker = '$';
            std::vector<uint8_t> alphabet_with_end_marker = stool::dynamic_r_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, end_marker);
            text.push_back(end_marker);

            std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
            std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
            std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

            stool::DebugPrinter::print_characters(text, "Text");

            stool::dynamic_r_index::DynamicRIndex drfmi1 = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);
            stool::dynamic_r_index::DynamicRIndex drfmi2 = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

            std::mt19937_64 mt64(seed);
            stool::r_index_test::DynamicRIndexTest::bwt_check(drfmi1, drfmi2);

            for (uint64_t i = 0; i < 30; i++)
            {
                stool::r_index_test::DynamicRIndexTest::bwt_test_random_insertion2(drfmi1, drfmi2, chars, mt64, true);
            }
        }        
        std::cout << std::endl;
        */

    }

    else if (mode == 0)
    {
        for (uint64_t i = 1; i <= 10; i++)
        {
            main_sub(i, detailed_check, seed);
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

    p.add<uint>("detailed_check", 'u', "detailed_check", false, 0);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");
    uint64_t seed = p.get<uint>("seed");

    bool detailed_check = p.get<uint>("detailed_check") == 0 ? false : true;

    main_sub(mode, detailed_check, seed);
}

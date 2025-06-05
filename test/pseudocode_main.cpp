#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
#include "include/dynamic_r_index_snapshot_for_insertion.hpp"
//

#include <filesystem>

void main_sub(int mode, bool detailed_check, uint64_t seed)
{
    uint64_t max_alphabet_type = 2;

    if (mode == 1)
    {
        std::cout << "Execute DynamicRIndexTest::detailed_string_deletion_test." << std::endl;
        uint64_t length_of_insertion_string = 4;

        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {
            uint64_t text_size = 10;

            while (text_size < 320)
            {
                length_of_insertion_string = 4;

                while (length_of_insertion_string < 32)
                {
                    for (uint64_t i = 0; i < 10; i++)
                    {
                        std::cout << alphabet_type << std::flush;
                        stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::insertion_test(text_size, length_of_insertion_string, alphabet_type, seed++);
                    }

                    length_of_insertion_string *= 2;
                }

                text_size *= 2;
            }
        }
        std::cout << std::endl;
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

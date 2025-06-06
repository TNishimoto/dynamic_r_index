#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include "stool/include/stool.hpp"
#include "include/dynamic_r_index_snapshot_for_insertion.hpp"
#include "include/dynamic_r_index_snapshot_for_deletion.hpp"
//

#include <filesystem>

void main_sub(int mode, bool detailed_check, uint64_t seed)
{
    uint64_t max_alphabet_type = 2;

    if (mode == 1)
    {
        std::cout << "Dynamic LF test for Insertion" << std::endl;
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
    else if (mode == 2)
    {
        std::cout << "Dynamic LF test for Deletion" << std::endl;
        uint64_t length_of_deletion_string = 4;

        for (uint64_t alphabet_type = 0; alphabet_type <= max_alphabet_type; alphabet_type++)
        {
            uint64_t text_size = 10;

            while (text_size < 320)
            {
                length_of_deletion_string = 4;

                while (length_of_deletion_string < 32 && length_of_deletion_string < text_size)
                {
                    for (uint64_t i = 0; i < 10; i++)
                    {
                        std::cout << alphabet_type << std::flush;
                        stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion::deletion_test(text_size, length_of_deletion_string, alphabet_type, seed++);
                    }

                    length_of_deletion_string *= 2;
                }

                text_size *= 2;
            }
        }
        std::cout << std::endl;
    }
    else if (mode == 0)
    {
        for (uint64_t i = 1; i <= 2; i++)
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
    p.add<std::string>("text", 't', "text", false, "");

    p.add<uint>("position", 'p', "position", false, 0);
    p.add<uint>("length", 'l', "length", false, 0);
    p.add<uint>("seed", 's', "seed", false, 0);

    p.add<uint>("detailed_check", 'u', "detailed_check", false, 0);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");
    uint64_t seed = p.get<uint>("seed");
    std::string text = p.get<std::string>("text");
    uint64_t position = p.get<uint>("position");
    uint64_t length = p.get<uint>("length");

    bool detailed_check = p.get<uint>("detailed_check") == 0 ? false : true;

    if (mode == 2)
    {

        stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion::deletion_test(text, position, length, true);
    }
    else
    {
        main_sub(mode, detailed_check, seed);
    }
}

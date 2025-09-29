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
    p.add<std::string>("text", 't', "text", false, "bbabba$");

    p.add<uint>("position", 'p', "position", false, 3);
    p.add<uint>("length", 'l', "length", false, 1);


    p.parse_check(argc, argv);
    std::string text = p.get<std::string>("text");
    uint64_t position = p.get<uint>("position");
    uint64_t length = p.get<uint>("length");


    std::string command;

    while(true){
        std::cout << "text \t\t\t = " << text << std::endl;
        std::cout << "position \t\t = " << stool::dynamic_r_index::NaiveOperations::value_with_index(position, true) << std::endl;
        std::cout << "deletion_length \t = " << length << std::endl;
        std::cout << "Command? (Press h to display the list of commands.)" << std::endl;
        

        std::cin >> command;
        if(command == "exit"){
            break;
        }
        else if(command == "snapshot"){

            std::vector<stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion> snap_shots = stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion::get_all_snap_shots(text, position, length);
            for (uint64_t i = 0; i < snap_shots.size(); i++)
            {
                snap_shots[i].print_conceptual_matrix(true);


                stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion *prev = i == 0 ? nullptr : &snap_shots[i - 1];

                if (i < snap_shots.size() - 1)
                {
                    snap_shots[i].verify_RLE_update(prev);
                    snap_shots[i].verify_dynamic_LF(snap_shots[i + 1]);
                    snap_shots[i].verify_SA_update(prev);
                }

            }


            //stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::insertion_demo(text, position, inserted_string, true);
        }
        else if(command == "test"){
            uint64_t alphabet_type = 0;
            uint64_t number_of_trials = 100;
            uint64_t text_size = 100;
            uint64_t insertion_length = 10;
            //uint64_t text_size = 10;
            //uint64_t insertion_length = 3;

            uint64_t seed = 0;
            stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion::deletion_test(text_size, insertion_length, alphabet_type, number_of_trials, seed);

        }
        else if(command == "h" || command == "help"){
            std::cout << "--------------[Command list]------------------" << std::endl;
            std::cout << "snapshot \t\t = Display the snapshot of the insertion." << std::endl;
            std::cout << "exit \t\t\t = Exit the program." << std::endl;
            std::cout << "--------------------------------" << std::endl;
            std::cout << std::endl;
        }
            
    }

    /*
    if (mode == 2)
    {

        stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion::deletion_test(text, position, length, true);
    }
    else
    {
        main_sub(mode, detailed_check, seed);
    }
    */

}

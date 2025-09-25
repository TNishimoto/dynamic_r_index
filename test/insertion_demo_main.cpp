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


int main(int argc, char *argv[])
{
    cmdline::parser p;

    // p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<std::string>("text", 't', "text", false, "bbabba$");
    p.add<int>("position", 'j', "position", false, 6);
    p.add<std::string>("inserted_string", 'p', "inserted string", false, "b");

    p.parse_check(argc, argv);
    std::string text = p.get<std::string>("text");
    uint64_t position = p.get<int>("position") - 1;
    std::string inserted_string = p.get<std::string>("inserted_string");

    std::string command;

    while(true){
        std::cout << "text \t\t\t = " << text << std::endl;
        std::cout << "position \t\t = " << stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::value_with_index(position, true) << std::endl;
        std::cout << "inserted_string \t = " << inserted_string << std::endl;
        std::cout << "Command? (Press h to display the list of commands.)" << std::endl;
        

        std::cin >> command;
        if(command == "exit"){
            break;
        }
        else if(command == "snapshot"){

            std::vector<stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion> snap_shots = stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::get_all_snap_shots(text, position, inserted_string);
            for (uint64_t i = 0; i < snap_shots.size(); i++)
            {
                snap_shots[i].print_information(true);

                stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion *prev = i == 0 ? nullptr : &snap_shots[i - 1];
                stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion *next = i + 1 < snap_shots.size() ? &snap_shots[i + 1] : nullptr;
                if(i < snap_shots.size() - 1){
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
            uint64_t seed = 0;
            stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::insertion_test(text_size, insertion_length, alphabet_type, number_of_trials, seed);
        }
        else if(command == "h" || command == "help"){
            std::cout << "--------------[Command list]------------------" << std::endl;
            std::cout << "snapshot \t\t = Display the snapshot of the insertion." << std::endl;
            std::cout << "exit \t\t\t = Exit the program." << std::endl;
            std::cout << "--------------------------------" << std::endl;
            std::cout << std::endl;
        }
            
    }

}

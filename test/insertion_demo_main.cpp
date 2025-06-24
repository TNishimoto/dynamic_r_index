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

    std::cout << "text \t\t\t = " << text << std::endl;
    std::cout << "position \t\t = " << stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::value_with_index(position, true) << std::endl;
    std::cout << "inserted_string \t = " << inserted_string << std::endl;

    stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion::insertion_demo(text, position, inserted_string, true);
}

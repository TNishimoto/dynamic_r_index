#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "../include/dynamic_r_index.hpp"
#include "libdivsufsort/sa.hpp"


/*
void print_NONE_query_result(const QueryResults &result, int message_paragraph = 0){
    std::vector<uint64_t> query_count_vector = result.get_query_count_vector();
    std::cout << "The number of NONE queries: \t\t\t\t\t" << query_count_vector[stool::QueryType::NONE] << std::endl;
}
*/



int main(int argc, char *argv[])
{
std::cout << "\033[41m";
#ifdef RELEASE_BUILD
    std::cout << "Running in Release mode";
#elif defined(DEBUG_BUILD)

    std::cout << "Running in Debug mode";
#else
    std::cout << "Running in Unknown mode";
#endif
std::cout << "\e[m" << std::endl;

    cmdline::parser p;

    p.add<std::string>("input_index_path", 'i', "The file path to the dynamic r-index or the dynamic FM-index", true);
    p.add<std::string>("command_path", 'q', "The tsv file path to commands", true);
    p.add<std::string>("output_log_path", 'w', "The file path to the log file", true);
    p.add<std::string>("output_index_path", 'o', "The path to the file where the updated index will be written", false, "");
    p.add<std::string>("alternative_tab_key", 't', "The alternative tab key for the command file", false, "");
    p.add<std::string>("alternative_line_break_key", 'n', "The alternative line break key for the command file", false, "");

    //p.add<bool>("detail_flag", 'b', "detail_flag", true);


    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_index_path");
    std::string query_file_path = p.get<std::string>("command_path");
    std::string log_file_path = p.get<std::string>("output_log_path");
    std::string output_file_path = p.get<std::string>("output_index_path");
    std::string alternative_tab_key = p.get<std::string>("alternative_tab_key");
    std::string alternative_line_break_key = p.get<std::string>("alternative_line_break_key");
    //bool detail_flag = p.get<bool>("detail_flag");

    // bool isLightWeight = p.get<bool>("lightweight");
    //  uint index_type = p.get<uint>("index_type");
    /*
    if (output_file_path.size() == 0)
    {
        output_file_path = input_file_path + ".upd";
    }
    */

    if (log_file_path.size() == 0)
    {
        log_file_path = input_file_path + ".query.log";
    }

    std::ofstream os;
    bool os_exist = output_file_path.size() > 0;
    if (os_exist)
    {
        os.open(output_file_path, std::ios::binary);
        if (!os)
        {
            std::cerr << "Error: Could not open file for writing. path: " << output_file_path << std::endl;
            throw std::runtime_error("File open error");
        }
    }

    std::ifstream query_ifs(query_file_path);

    std::ofstream log_os;
    log_os.open(log_file_path);
    if (!log_os)
    {
        std::cerr << "Error: Could not open file for writing. path: " << log_file_path << std::endl;
        throw std::runtime_error("File open error");
    }

    uint64_t mark = stool::IO::load_first_64bits(input_file_path);

    uint64_t hash1 = 0;
    uint64_t hash2 = 0;

    std::chrono::system_clock::time_point st1, st2;
    st1 = std::chrono::system_clock::now();
    // uint is_r_index = 1;

    stool::dynamic_r_index::QueryResults result;
    std::string index_name = "";

    if (mark == stool::dynamic_r_index::DynamicFMIndex::LOAD_KEY)
    {
        index_name = "Dynamic FM-index";
        stool::dynamic_r_index::DynamicFMIndex dfmi;
        {
            std::ifstream ifs;
            ifs.open(input_file_path, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Error: Could not open file for reading." << std::endl;
                throw std::runtime_error("File open error");
            }
            auto tmp = stool::dynamic_r_index::DynamicFMIndex::build_from_data(ifs);
            dfmi.swap(tmp);
        }
        dfmi.print_light_statistics();
        auto tmp_result = stool::dynamic_r_index::process_query_file(dfmi, query_ifs, log_os, alternative_tab_key, alternative_line_break_key);
        result.swap(tmp_result);

        if (os_exist)
        {
            stool::dynamic_r_index::DynamicFMIndex::save(dfmi, os);
        }
    }
    else if (mark == stool::dynamic_r_index::DynamicRIndex::LOAD_KEY)
    {
        index_name = "Dynamic r-index";

        stool::dynamic_r_index::DynamicRIndex drfmi;
        {
            std::ifstream ifs;
            ifs.open(input_file_path, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Error: Could not open file for reading." << std::endl;
                throw std::runtime_error("File open error");
            }
            auto tmp = stool::dynamic_r_index::DynamicRIndex::build_from_data(ifs);
            drfmi.swap(tmp);
        }
        drfmi.print_light_statistics();
        hash1 = drfmi.compute_RLBWT_hash();
        auto tmp_result = stool::dynamic_r_index::process_query_file(drfmi, query_ifs, log_os, alternative_tab_key, alternative_line_break_key);
        hash2 = drfmi.compute_RLBWT_hash();

        result.swap(tmp_result);

        if (os_exist)
        {
            stool::dynamic_r_index::DynamicRIndex::save(drfmi, os);
        }
    }
    else
    {
        throw std::runtime_error("Invalid input path");
    }

    if (os_exist)
    {
        os.close();
    }
    query_ifs.close();
    log_os.close();

    st2 = std::chrono::system_clock::now();

    std::cout << "Hash: " << hash1 << " -> " << hash2 << std::endl;

    std::cout << "\033[36m";
    std::cout << "=============RESULT===============" << std::endl;
    std::cout << "Index File: \t\t\t\t\t" << input_file_path << std::endl;
    std::cout << "Index Type: \t\t\t\t\t" << index_name << std::endl;
    std::cout << "Output File: \t\t\t\t\t" << output_file_path << std::endl;
    std::cout << "Query File: \t\t\t\t\t" << query_file_path << std::endl;
    std::cout << "Log File: \t\t\t\t\t" << log_file_path << std::endl;
    std::cout << "Checksum: \t\t\t\t\t" << result.check_sum << std::endl;
    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
    std::cout << "Total time: \t\t\t\t\t" << sec_time << " sec" << std::endl;
    stool::Memory::print_memory_usage();

    print_query_result(result, stool::QueryType::NONE);
    print_query_result(result, stool::QueryType::PRINT);
    print_query_result(result, stool::QueryType::INSERT);
    print_query_result(result, stool::QueryType::DELETE);
    print_query_result(result, stool::QueryType::COUNT);
    print_query_result(result, stool::QueryType::LOCATE);
    print_query_result(result, stool::QueryType::LOCATE_SUM);
    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

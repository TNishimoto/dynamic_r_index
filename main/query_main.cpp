#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
#include <filesystem>
#include "stool/include/stool.hpp"
#include "../include/dynamic_r_index.hpp"
#include "libdivsufsort/sa.hpp"

enum class VecType
{
    PATTERN_LENGTH,
    BACKWARD_SEARCH_TIME,
    COMPUTING_SA_TIME,
    REORDER_COUNT,
    OCCURRENCE_COUNT,
    ELAPSED_TIME
};

struct QueryResults
{
public:
    std::vector<stool::QueryType> query_types;
    std::vector<uint64_t> pattern_length_vector;
    std::vector<uint64_t> elapsed_time_vector;
    std::vector<uint64_t> backward_search_time_vector;
    std::vector<uint64_t> computing_SA_time_vector;
    std::vector<uint64_t> reorder_count_vector;
    std::vector<uint64_t> occurrence_count_vector;

    uint64_t check_sum = 0;

    QueryResults()
    {
        this->query_types.clear();
        this->pattern_length_vector.clear();
        this->elapsed_time_vector.clear();
        this->backward_search_time_vector.clear();
        this->computing_SA_time_vector.clear();
        this->reorder_count_vector.clear();
        this->occurrence_count_vector.clear();
        this->check_sum = 0;
    }
    
    void swap(QueryResults &item)
    {
        this->query_types.swap(item.query_types);
        this->pattern_length_vector.swap(item.pattern_length_vector);
        this->elapsed_time_vector.swap(item.elapsed_time_vector);
        this->reorder_count_vector.swap(item.reorder_count_vector);
        this->backward_search_time_vector.swap(item.backward_search_time_vector);
        this->computing_SA_time_vector.swap(item.computing_SA_time_vector);

        this->occurrence_count_vector.swap(item.occurrence_count_vector);
        std::swap(this->check_sum, item.check_sum);
    }
    

    std::vector<uint64_t> get_query_count_vector() const
    {
        std::vector<uint64_t> r;
        r.resize(5, 0);
        for (stool::QueryType c : this->query_types)
        {
            r[(int)c]++;
        }
        return r;
    }
    std::vector<uint64_t> get_vector(stool::QueryType type1, VecType type2) const
    {
        const std::vector<uint64_t> *vec;

        if (type2 == VecType::BACKWARD_SEARCH_TIME)
        {
            vec = &this->backward_search_time_vector;
        }
        else if (type2 == VecType::COMPUTING_SA_TIME)
        {
            vec = &this->computing_SA_time_vector;
        }
        else if (type2 == VecType::ELAPSED_TIME)
        {
            vec = &this->elapsed_time_vector;
        }
        else if (type2 == VecType::OCCURRENCE_COUNT)
        {
            vec = &this->occurrence_count_vector;
        }
        else if (type2 == VecType::PATTERN_LENGTH)
        {
            vec = &this->pattern_length_vector;
        }
        else if (type2 == VecType::REORDER_COUNT)
        {
            vec = &this->reorder_count_vector;
        }

        std::vector<uint64_t> r;
        for (uint64_t i = 0; i < this->query_types.size(); i++)
        {
            if (this->query_types[i] == type1)
            {
                r.push_back(vec->at(i));
            }
        }
        return r;
    }
    void push_back(stool::QueryType type, uint64_t pattern_length, uint64_t elapsed_time, uint64_t reorder_count, uint64_t backward_search_time, uint64_t sa_time, uint64_t occurrence_count)
    {
        this->query_types.push_back(type);
        this->pattern_length_vector.push_back(pattern_length);
        this->elapsed_time_vector.push_back(elapsed_time);
        this->reorder_count_vector.push_back(reorder_count);
        this->backward_search_time_vector.push_back(backward_search_time);
        this->computing_SA_time_vector.push_back(sa_time);
        this->occurrence_count_vector.push_back(occurrence_count);
    }
};

template <typename DYNINDEX>
QueryResults process_query_file(DYNINDEX &dyn_index, std::ifstream &query_ifs, std::ostream &log_os)
{
    std::string line;
    uint64_t query_number = 0;
    QueryResults r;

    while (std::getline(query_ifs, line))
    {
        std::chrono::system_clock::time_point st1, st2, st3;

        stool::LineQuery q = stool::LineQuery::load_line(line);

        if(q.type == stool::QueryType::VIEW){
            //std::cout << "Text: " << dyn_index.to_string() << std::endl;
            log_os << query_number << ", " << "VIEW, " << "Text length," << dyn_index.size() << ", Text, " << dyn_index.get_text_str() << ", BWT, " << dyn_index.get_bwt_str() << std::endl;

        }
        else if (q.type == stool::QueryType::INSERT)
        {
            if (query_number % 10 == 0)
            {
                std::cout << "Processed " << query_number << " queries..." << std::endl;
            }
            st1 = std::chrono::system_clock::now();

            stool::fm_index::FMIndexEditHistory edit_history;
            if (q.pattern.size() == 1)
            {
                r.check_sum += dyn_index.insert_char(q.position, q.pattern[0], edit_history);
            }
            else
            {
                r.check_sum += dyn_index.insert_string(q.position, q.pattern, edit_history);
            }
            st2 = std::chrono::system_clock::now();
            uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

            r.push_back(q.type, q.pattern.size(), micro_time, edit_history.move_history.size(), 0, 0, 0);

            log_os << query_number << ", " << "INSERT, " << "Time (microseconds), " << micro_time << ", Reorder counts, " << edit_history.move_history.size() << std::endl;
        }
        else if (q.type == stool::QueryType::DELETE)
        {
            if (query_number % 10 == 0)
            {
                std::cout << "Processed " << query_number << " queries..." << std::endl;
            }

            st1 = std::chrono::system_clock::now();

            stool::fm_index::FMIndexEditHistory edit_history;
            if (q.length == 1)
            {
                r.check_sum += dyn_index.delete_char(q.position, edit_history);
            }
            else
            {
                r.check_sum += dyn_index.delete_string(q.position, q.length, edit_history);
            }

            st2 = std::chrono::system_clock::now();

            uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
            r.push_back(q.type, q.pattern.size(), micro_time, edit_history.move_history.size(), 0, 0, 0);

            log_os << query_number << ", " << "DELETE, " << "Time (microseconds), " << micro_time << ", Reorder counts, " << edit_history.move_history.size() << std::endl;
        }
        else if (q.type == stool::QueryType::COUNT)
        {
            if (query_number % 100 == 0)
            {
                std::cout << "Processed " << query_number << " queries..." << std::endl;
            }

            st1 = std::chrono::system_clock::now();
            uint64_t count_result = dyn_index.count_query(q.pattern);
            st2 = std::chrono::system_clock::now();
            uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

            r.push_back(q.type, q.pattern.size(), micro_time, 0, 0, 0, count_result);

            log_os << query_number << ", " << "COUNT, " << "Time (microseconds), " << micro_time << ", The number of occurrencces, " << count_result << std::endl;
        }
        else if (q.type == stool::QueryType::LOCATE)
        {
            if (query_number % 100 == 0)
            {
                std::cout << "Processed " << query_number << " queries..." << std::endl;
            }

            st1 = std::chrono::system_clock::now();
            stool::fm_index::BackwardSearchResult bsr = dyn_index.backward_search(q.pattern);
            st2 = std::chrono::system_clock::now();
            std::vector<uint64_t> sa_values = dyn_index.compute_sa_values(bsr);
            st3 = std::chrono::system_clock::now();

            uint64_t micro_time1 = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
            uint64_t micro_time2 = std::chrono::duration_cast<std::chrono::microseconds>(st3 - st2).count();
            uint64_t total_time = std::chrono::duration_cast<std::chrono::microseconds>(st3 - st1).count();

            
            uint64_t locate_sum = 0;
            for (uint64_t v : sa_values)
            {
                locate_sum += v;
            }
            

            r.push_back(q.type, q.pattern.size(), total_time, 0, micro_time1, micro_time2, sa_values.size());

            log_os << query_number << ", " << "LOCATE, " << "The execution time of backward search (microseconds), " << micro_time1 << ", The execution time of computing sa-values (microseconds), " << micro_time2
                   << ", Total time (microseconds)," << total_time << ", The number of occurrencces, " << sa_values.size() << ", Checksum, " << locate_sum << std::endl;
        }
        else if (q.type == stool::QueryType::LOCATE_VIEW){
            if (query_number % 100 == 0)
            {
                std::cout << "Processed " << query_number << " queries..." << std::endl;
            }

            st1 = std::chrono::system_clock::now();
            stool::fm_index::BackwardSearchResult bsr = dyn_index.backward_search(q.pattern);
            st2 = std::chrono::system_clock::now();
            std::vector<uint64_t> sa_values = dyn_index.compute_sa_values(bsr);
            st3 = std::chrono::system_clock::now();

            uint64_t micro_time1 = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
            uint64_t micro_time2 = std::chrono::duration_cast<std::chrono::microseconds>(st3 - st2).count();
            uint64_t total_time = std::chrono::duration_cast<std::chrono::microseconds>(st3 - st1).count();

            
            uint64_t locate_sum = 0;

            std::string occ_str = "";
            for (uint64_t v : sa_values)
            {
                locate_sum += v;
            }
            for(uint64_t i = 0; i < sa_values.size(); i++){
                occ_str += std::to_string(sa_values[i]);
                if(i < sa_values.size() - 1){
                    occ_str += ", ";
                }
            }

            r.push_back(q.type, q.pattern.size(), total_time, 0, micro_time1, micro_time2, sa_values.size());

            log_os << query_number << ", " << "LOCATE!, " << "The execution time of backward search (microseconds), " << micro_time1 << ", The execution time of computing sa-values (microseconds), " << micro_time2
                   << ", Total time (microseconds)," << total_time << ", The number of occurrencces, " << sa_values.size() << ", Checksum, " << locate_sum << std::endl;
            log_os << query_number << ", " << "Occurrences of Pattern, " << occ_str << std::endl;

        }
        else
        {
            std::cout << "N" << std::flush;

            r.push_back(q.type, 0, 0, 0, 0, 0, 0);

            log_os << query_number << ", " << "NONE" << std::endl;
        }
        query_number++;
    }

    return r;
}
/*
void print_NONE_query_result(const QueryResults &result, int message_paragraph = 0){
    std::vector<uint64_t> query_count_vector = result.get_query_count_vector();
    std::cout << "The number of NONE queries: \t\t\t\t\t" << query_count_vector[stool::QueryType::NONE] << std::endl;
}
*/

void print_sub(const QueryResults &result, stool::QueryType type1, VecType type2, std::string message_name, std::string unit_name, int message_paragraph = stool::Message::SHOW_MESSAGE)
{
    std::vector<uint64_t> vec = result.get_vector(type1, type2);
    uint64_t _total = std::reduce(std::begin(vec), std::end(vec));
    uint64_t _average = _total / vec.size();
    uint64_t _min = *min_element(std::begin(vec), std::end(vec));
    uint64_t _max = *max_element(std::begin(vec), std::end(vec));
    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << message_name << ": \t\t\t\t\t" << _total << " " << unit_name << std::endl;
    std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Average: \t\t\t\t\t" << _average << " " << unit_name << std::endl;
    std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Min: \t\t\t\t\t" << _min << " " << unit_name << std::endl;
    std::cout << stool::Message::get_paragraph_string(message_paragraph + 2) << "Max: \t\t\t\t\t" << _max << " " << unit_name << std::endl;
}

void print_query_result(const QueryResults &result, stool::QueryType type, int message_paragraph = stool::Message::SHOW_MESSAGE)
{
    std::vector<uint64_t> query_count_vector = result.get_query_count_vector();
    uint64_t counter = query_count_vector[(int)type];

    if (type == stool::QueryType::INSERT)
    {
        std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of INSERT queries: \t\t\t\t\t" << counter << std::endl;
        if(counter > 0){
        print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
        print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
        print_sub(result, type, VecType::REORDER_COUNT, "Total reorder count", "", message_paragraph);
        }
    }
    else if (type == stool::QueryType::DELETE)
    {
        std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of DELETE queries: \t\t\t\t\t" << counter << std::endl;

        if(counter > 0){
        print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
        print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
        print_sub(result, type, VecType::REORDER_COUNT, "Total reorder count", "", message_paragraph);
        }
    }
    else if (type == stool::QueryType::COUNT)
    {
        std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of COUNT queries: \t\t\t\t\t" << counter << std::endl;
        if(counter > 0){
        print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
        print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
        print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);

        }
    }
    else if (type == stool::QueryType::LOCATE)
    {
        std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of LOCATE queries: \t\t\t\t\t" << counter << std::endl;
        if(counter > 0){
        print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
        print_sub(result, type, VecType::BACKWARD_SEARCH_TIME, "Total backward search time", "microseconds", message_paragraph);
        print_sub(result, type, VecType::COMPUTING_SA_TIME, "Total computing SA time", "microseconds", message_paragraph);
        print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
        print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);
        }
    }
    else
    {
        std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of NONE queries: \t\t\t\t\t" << counter << std::endl;
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

    p.add<std::string>("input_file", 'i', "input index name", true);
    p.add<std::string>("output_file", 'o', "output index name", false, "");
    p.add<std::string>("query_file", 'q', "query file", true);
    p.add<std::string>("log_file", 'w', "log file", true);
    //p.add<bool>("detail_flag", 'b', "detail_flag", true);


    p.parse_check(argc, argv);
    std::string input_file_path = p.get<std::string>("input_file");
    std::string query_file_path = p.get<std::string>("query_file");
    std::string log_file_path = p.get<std::string>("log_file");
    std::string output_file_path = p.get<std::string>("output_file");
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

    QueryResults result;
    std::string index_name = "";

    if (mark == stool::fm_index::DynamicFMIndex::LOAD_KEY)
    {
        index_name = "Dynamic FM-index";
        stool::fm_index::DynamicFMIndex dfmi;
        {
            std::ifstream ifs;
            ifs.open(input_file_path, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Error: Could not open file for reading." << std::endl;
                throw std::runtime_error("File open error");
            }
            auto tmp = stool::fm_index::DynamicFMIndex::build_from_data(ifs);
            dfmi.swap(tmp);
        }
        dfmi.print_light_statistics();
        auto tmp_result = process_query_file(dfmi, query_ifs, log_os);
        result.swap(tmp_result);

        if (os_exist)
        {
            stool::fm_index::DynamicFMIndex::save(dfmi, os);
        }
    }
    else if (mark == stool::r_index::DynamicRIndex::LOAD_KEY)
    {
        index_name = "Dynamic r-index";

        stool::r_index::DynamicRIndex drfmi;
        {
            std::ifstream ifs;
            ifs.open(input_file_path, std::ios::binary);
            if (!ifs)
            {
                std::cerr << "Error: Could not open file for reading." << std::endl;
                throw std::runtime_error("File open error");
            }
            auto tmp = stool::r_index::DynamicRIndex::build_from_data(ifs);
            drfmi.swap(tmp);
        }
        drfmi.print_light_statistics();
        hash1 = drfmi.compute_RLBWT_hash();
        auto tmp_result = process_query_file(drfmi, query_ifs, log_os);
        hash2 = drfmi.compute_RLBWT_hash();

        result.swap(tmp_result);

        if (os_exist)
        {
            stool::r_index::DynamicRIndex::save(drfmi, os);
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
    stool::print_memory_usage();

    print_query_result(result, stool::QueryType::NONE);
    print_query_result(result, stool::QueryType::INSERT);
    print_query_result(result, stool::QueryType::DELETE);
    print_query_result(result, stool::QueryType::COUNT);
    print_query_result(result, stool::QueryType::LOCATE);

    std::cout << "==================================" << std::endl;
    std::cout << "\033[39m" << std::endl;
}

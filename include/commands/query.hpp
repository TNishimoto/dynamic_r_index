#pragma once
#include "./line_query.hpp"
#include "stool/include/lib.hpp"
#include "../all.hpp"

namespace stool
{
    namespace dynamic_r_index
    {

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
                r.resize(8, 0);
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
        QueryResults process_query_file(DYNINDEX &dyn_index, std::ifstream &query_ifs, std::ostream &log_os, std::string alternative_tab_key, std::string alternative_line_break_key, bool replace_mode)
        {
            std::string line;
            uint64_t query_number = 0;
            QueryResults query_results;

            while (std::getline(query_ifs, line))
            {
                std::chrono::system_clock::time_point st1, st2, st3;

                stool::LineQuery q = stool::LineQuery::load_line(line, alternative_tab_key, alternative_line_break_key);

                if (replace_mode && q.type == stool::QueryType::LOCATE)
                {
                    q.type = stool::QueryType::LOCATE_SUM;
                }

                if (q.type == stool::QueryType::PRINT)
                {
                    // std::cout << "Text: " << dyn_index.to_string() << std::endl;
                    query_results.push_back(q.type, 0, 0, 0, 0, 0, 0);

                    log_os << query_number << "\t" << "PRINT" << "\t" << "Text:" << "\t" << dyn_index.get_text_str() << "\t" << "BWT:" << "\t" << dyn_index.get_bwt_str() << std::endl;
                }
                else if (q.type == stool::QueryType::INSERT)
                {
                    if (query_number % 10 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }
                    st1 = std::chrono::system_clock::now();

                    stool::dynamic_r_index::FMIndexEditHistory edit_history;
                    if (q.pattern.size() == 1)
                    {
                        query_results.check_sum += dyn_index.insert_char(q.position, q.pattern[0], edit_history);
                    }
                    else
                    {
                        query_results.check_sum += dyn_index.insert_string(q.position, q.pattern, edit_history);
                    }
                    st2 = std::chrono::system_clock::now();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();

                    query_results.push_back(q.type, q.pattern.size(), micro_time, edit_history.move_history.size(), 0, 0, 0);

                    log_os << query_number << "\t" << "INSERT" << "\t" << "Reorder count: " << "\t" << edit_history.move_history.size() << "\t" << "Time (microseconds): " << "\t" << micro_time  << std::endl;
                }
                else if (q.type == stool::QueryType::DELETE)
                {
                    if (query_number % 10 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    st1 = std::chrono::system_clock::now();

                    stool::dynamic_r_index::FMIndexEditHistory edit_history;
                    if (q.length == 1)
                    {
                        query_results.check_sum += dyn_index.delete_char(q.position, edit_history);
                    }
                    else
                    {
                        query_results.check_sum += dyn_index.delete_string(q.position, q.length, edit_history);
                    }

                    st2 = std::chrono::system_clock::now();

                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    query_results.push_back(q.type, q.pattern.size(), micro_time, edit_history.move_history.size(), 0, 0, 0);

                    log_os << query_number << "\t" << "DELETE" << "\t" << "Reorder count: " << "\t" << edit_history.move_history.size() << "\t" << "Time (microseconds): " << "\t" << micro_time  << std::endl;

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

                    query_results.push_back(q.type, q.pattern.size(), micro_time, 0, 0, 0, count_result);

                    log_os << query_number << "\t" << "COUNT" << "\t" << "The number of occurrencces of the given pattern: " << "\t" << count_result<< "\t" << "Time (microseconds): " << "\t" << micro_time  << std::endl;
                }
                else if (q.type == stool::QueryType::LOCATE)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    st1 = std::chrono::system_clock::now();
                    stool::dynamic_r_index::BackwardSearchResult bsr = dyn_index.backward_search(q.pattern);
                    st2 = std::chrono::system_clock::now();
                    std::vector<uint64_t> sa_values = dyn_index.compute_sa_values(bsr);
                    st3 = std::chrono::system_clock::now();

                    uint64_t micro_time1 = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t micro_time2 = std::chrono::duration_cast<std::chrono::microseconds>(st3 - st2).count();
                    uint64_t total_time = std::chrono::duration_cast<std::chrono::microseconds>(st3 - st1).count();

                    std::string sa_value_str = "";
                    sa_value_str += "[";
                    for(uint64_t i = 0; i < sa_values.size(); i++){
                        sa_value_str += std::to_string(sa_values[i]);
                        if(i < sa_values.size() - 1){
                            sa_value_str += ", ";
                        }
                    }
                    sa_value_str += "]";

                    query_results.push_back(q.type, q.pattern.size(), total_time, 0, micro_time1, micro_time2, sa_values.size());

                    log_os << query_number << "\t" << "LOCATE" << "\t" << "The occurrences of the given pattern: " << "\t" << sa_value_str<< "\t" << "Time (microseconds): " << "\t" << total_time  << std::endl;
                }
                else if(q.type == stool::QueryType::LOCATE_DETAIL){
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    st1 = std::chrono::system_clock::now();
                    stool::dynamic_r_index::BackwardSearchResult bsr = dyn_index.backward_search(q.pattern);
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

                    query_results.push_back(q.type, q.pattern.size(), total_time, 0, micro_time1, micro_time2, sa_values.size());

                    log_os << query_number << ", " << "LOCATE, " << "The execution time of backward search (microseconds), " << micro_time1 << ", The execution time of computing sa-values (microseconds), " << micro_time2
                           << ", Total time (microseconds)," << total_time << ", The number of occurrencces, " << sa_values.size() << ", Checksum, " << locate_sum << std::endl;

                }
                else if (q.type == stool::QueryType::LOCATE_SUM)
                {
                    if (query_number % 100 == 0)
                    {
                        std::cout << "Processed " << query_number << " queries..." << std::endl;
                    }

                    st1 = std::chrono::system_clock::now();
                    stool::dynamic_r_index::BackwardSearchResult bsr = dyn_index.backward_search(q.pattern);
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
                    query_results.push_back(q.type, q.pattern.size(), total_time, 0, micro_time1, micro_time2, sa_values.size());

                    log_os << query_number << "\t" << "LOCATE_SUM" << "\t" << "The sum of occurrence positions of the given pattern: " << "\t" << locate_sum 
                    << "\t" << "The number of occurences: " << "\t" << sa_values.size() << "\t" << "The execution time of backward search (microseconds):" << "\t" << micro_time1 
                    << "\t" << "The execution time of computing sa-values (microseconds): " << "\t" << micro_time2  << "\t" << "Time (microseconds): " << "\t" << total_time  << std::endl;
                    

                }
                else
                {
                    std::cout << "N" << std::flush;

                    query_results.push_back(q.type, 0, 0, 0, 0, 0, 0);

                    log_os << query_number << ", " << "NONE" << std::endl;
                }
                query_number++;
            }

            return query_results;
        }
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

            if (type == stool::QueryType::PRINT)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of PRINT queries: \t\t\t\t\t" << counter << std::endl;
            }
            else if (type == stool::QueryType::INSERT)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of INSERT queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::REORDER_COUNT, "Total reorder count", "", message_paragraph);
                }
            }
            else if (type == stool::QueryType::DELETE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of DELETE queries: \t\t\t\t\t" << counter << std::endl;

                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::REORDER_COUNT, "Total reorder count", "", message_paragraph);
                }
            }
            else if (type == stool::QueryType::COUNT)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of COUNT queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);
                }
            }
            else if (type == stool::QueryType::LOCATE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of LOCATE queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
                    print_sub(result, type, VecType::ELAPSED_TIME, "Total time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::BACKWARD_SEARCH_TIME, "Total backward search time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::COMPUTING_SA_TIME, "Total computing SA time", "microseconds", message_paragraph);
                    print_sub(result, type, VecType::PATTERN_LENGTH, "Total pattern length", "", message_paragraph);
                    print_sub(result, type, VecType::OCCURRENCE_COUNT, "Total occurrences", "", message_paragraph);
                }
            }
            else if (type == stool::QueryType::LOCATE_SUM)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "The number of LOCATE_SUM queries: \t\t\t\t\t" << counter << std::endl;
                if (counter > 0)
                {
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
    }

}
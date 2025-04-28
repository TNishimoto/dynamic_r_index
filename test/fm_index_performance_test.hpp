#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "libdivsufsort/sa.hpp"
#include "stool/include/stool.hpp"
#include "../include/drfmi.hpp"
#include "./dynamic_fm_index_test.hpp"

namespace stool
{
    namespace fm_index_test
    {
        using namespace r_index;
#ifdef DEBUG
        uint64_t TRIAL1 = 100;
#else
        uint64_t TRIAL1 = 100;
#endif

#ifdef DEBUG
        uint64_t TRIAL2 = 10;
#else
        uint64_t TRIAL2 = 10;
#endif

        class PerformanceTest
        {
        public:
            static void performance_test(DynamicBWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Performance Test" << std::endl;
                performance_access_test(dbwt, seed, message_paragraph + 1);

                performance_rank_test(dbwt, seed, message_paragraph + 1);
                performance_select_test(dbwt, seed, message_paragraph + 1);
                performance_LF_test(dbwt, seed, message_paragraph + 1);
                performance_insert_and_delete_test(dbwt, seed, message_paragraph + 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            static void performance_test(DynamicFMIndex &dfmi, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                auto __dbwt_pointer = dfmi._get_dbwt_pointer();
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Performance Test" << std::endl;

                /*
                performance_test(*__dbwt_pointer, seed, message_paragraph + 1);

                auto __dsa_pointer = dfmi._get_dsa_pointer();
                performance_SA_test(*__dsa_pointer, seed, message_paragraph + 1);
                */
                performance_insert_and_delete_test_on_index(dfmi, seed, message_paragraph + 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            template <typename DSA>
            static uint64_t performance_SA_test(const DSA &dsa, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = dsa.size();
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "SA Test" << std::endl;

                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }

                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;

                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    check_sum += dsa.sa(rand_pos_vec[i]);
                }
                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)micro_time / (double)trial);

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << ms_time << " ms (" << per_time << " microseconds per query), checksum = " << check_sum << std::endl;
                }
                return check_sum;
            }

            template <typename BWT>
            static uint64_t performance_access_test(const BWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = dbwt.size();
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Access Test" << std::endl;

                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }

                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    uint64_t x = dbwt.access(rand_pos_vec[i]);
                    check_sum += x;
                }
                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)micro_time / (double)trial);

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << ms_time << " ms (" << per_time << " microseconds per query), checksum = " << check_sum << std::endl;
                }
                return check_sum;
            }
            template <typename BWT>
            static uint64_t performance_select_test(const BWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = dbwt.size();
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Select Test" << std::endl;
                auto alphabet = dbwt.get_alphabet();
                std::vector<uint64_t> c_counters;
                for (uint8_t c : alphabet)
                {
                    c_counters.push_back(dbwt.count_c(c));
                }

                std::vector<uint8_t> rand_char_vec;
                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    uint8_t c = dbwt.access(get_rand_uni_pos(mt64));
                    uint64_t c_rank = dbwt.get_c_id(c);
                    rand_char_vec.push_back(c);
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64) % c_counters[c_rank]);
                }

                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    uint64_t x = dbwt.select(rand_char_vec[i], rand_pos_vec[i]);
                    check_sum += x;
                }
                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)micro_time / (double)trial);

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << ms_time << " ms (" << per_time << " microseconds per query), checksum = " << check_sum << std::endl;
                }
                return check_sum;
            }
            template <typename BWT>
            static uint64_t performance_LF_test(const BWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = dbwt.size();

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "LF Test" << std::endl;
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }

                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    check_sum += dbwt.LF(rand_pos_vec[i]);
                }
                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)micro_time / (double)trial);

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << ms_time << " ms (" << per_time << " microseconds per query), checksum = " << check_sum << std::endl;
                }
                return check_sum;
            }
            template <typename BWT>
            static uint64_t performance_rank_test(const BWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = dbwt.size();

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Rank Test" << std::endl;
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::vector<uint8_t> rand_char_vec;
                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_char_vec.push_back(dbwt.access(get_rand_uni_pos(mt64)));
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }

                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    check_sum += dbwt.rank(rand_char_vec[i], rand_pos_vec[i]);
                }
                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)micro_time / (double)trial);

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << ms_time << " ms (" << per_time << " microseconds per query), checksum = " << check_sum << std::endl;
                }
                return check_sum;
            }
            template <typename BWT>
            static uint64_t performance_insert_and_delete_test(BWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                uint64_t text_size = dbwt.size();

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Insert and Delete Test" << std::endl;
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::vector<uint8_t> rand_char_vec;
                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_char_vec.push_back(dbwt.access(get_rand_uni_pos(mt64)));
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }

                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    uint64_t pos = rand_pos_vec[i];
                    dbwt.insert_BWT_character(pos, rand_char_vec[i]);
                    dbwt.remove_BWT_character(pos);
                    check_sum += rand_char_vec[i] + pos;
                }
                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)micro_time / (double)(trial * 2));

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << ms_time << " ms (" << per_time << " microseconds per query), checksum = " << check_sum << std::endl;
                }
                return check_sum;
            }

            template <typename FMINDEX>
            static uint64_t performance_insert_and_delete_test_on_index(FMINDEX &dfmi, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                uint64_t text_size = dfmi.size();

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Insert and Delete Test on Index" << std::endl;
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::vector<uint8_t> rand_char_vec;
                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = TRIAL2;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_char_vec.push_back(dfmi.access(get_rand_uni_pos(mt64)));
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }

                uint64_t check_sum1 = 0;
                uint64_t check_sum2 = 0;
                uint64_t elapsed_micro_time = 0;
                for (uint64_t i = 0; i < trial; i++)
                {
                    FMIndexEditHistory history1;
                    FMIndexEditHistory history2;

                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                    uint64_t pos = rand_pos_vec[i];
                    dfmi.insert_char(pos, rand_char_vec[i], history1);
                    dfmi.delete_char(pos, history2);

                st2 = std::chrono::system_clock::now();
                    uint64_t micro_time = std::chrono::duration_cast<std::chrono::microseconds>(st2 - st1).count();
                    elapsed_micro_time += micro_time;


                    check_sum1 += history1.move_history.size();
                    check_sum2 += history2.move_history.size();

                }

                if (message_paragraph >= 0 && trial > 0)
                {
                    uint64_t per_time = ((double)elapsed_micro_time / (double)(trial * 2));

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << (elapsed_micro_time/1000) << " ms (" << per_time << " microseconds per query), checksum1 = " << check_sum1 << ", checksum2 = " << check_sum2 << std::endl;
                }
                return check_sum1 + check_sum2;
            }
        };

    }
}

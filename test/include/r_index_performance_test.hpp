#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "libdivsufsort/sa.hpp"
#include "stool/include/stool.hpp"
#include "../../include/dynamic_r_index.hpp"
#include "./fm_index_performance_test.hpp"

namespace stool
{
    namespace r_index_test
    {
        using namespace r_index;

        class PerformanceTest
        {
        public:
            static void performance_test(DynamicRLBWT &dbwt, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Performance Test" << std::endl;

                
                stool::fm_index_test::PerformanceTest::performance_access_test(dbwt, seed, message_paragraph + 1);

                stool::fm_index_test::PerformanceTest::performance_rank_test(dbwt, seed, message_paragraph + 1);
                stool::fm_index_test::PerformanceTest::performance_select_test(dbwt, seed, message_paragraph + 1);
                stool::fm_index_test::PerformanceTest::performance_LF_test(dbwt, seed, message_paragraph + 1);
                


                stool::fm_index_test::PerformanceTest::performance_insert_and_delete_test(dbwt, seed, message_paragraph + 1);



                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            static void performance_test(DynamicRIndex &drfmi, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                
                auto __dbwt_pointer = drfmi._get_dbwt_pointer();

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Performance Test" << std::endl;

                //performance_test(*__dbwt_pointer, seed, message_paragraph + 1);

                auto __dsa_pointer = drfmi._get_dsa_pointer();
                //performance_phi_test(*__dsa_pointer, seed, message_paragraph+1);

               #ifdef TIME_DEBUG
                stool::r_index::__reset_time();
               #endif 

                stool::fm_index_test::PerformanceTest::performance_insert_and_delete_test_on_index(drfmi, seed, message_paragraph + 1);
               #ifdef TIME_DEBUG
                stool::r_index::__print_time();
               #endif 




                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

template<typename DSA>
            static uint64_t performance_phi_test(const DSA &dsa, uint64_t seed, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = dsa.size();
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_pos(0, text_size - 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Phi Test" << std::endl;

                std::vector<uint64_t> rand_pos_vec;
                uint64_t trial = stool::fm_index_test::TRIAL1;

                for (uint64_t i = 0; i < trial; i++)
                {
                    rand_pos_vec.push_back(get_rand_uni_pos(mt64));
                }


                uint64_t check_sum = 0;
                std::chrono::system_clock::time_point st1, st2;

                st1 = std::chrono::system_clock::now();
                for (uint64_t i = 0; i < trial; i++)
                {
                    check_sum += dsa.phi(rand_pos_vec[i]);
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
            

        };

    }
}

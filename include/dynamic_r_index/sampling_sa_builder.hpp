#pragma once
#include "./dynamic_rlbwt_helper.hpp"
#include "stool/include/lib.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief Builder class for constructing sampled suffix arrays from RLBWT
         * 
         * This class builds sampled SA values at the first and last positions
         * of each run in the RLBWT, which are used to initialize DynamicPhi.
         * 
         * \ingroup DynamicRIndexes
         */
        class SamplingSATBuilder
        {
            using RLBWT = stool::rlbwt2::RLE<uint8_t>;

        public:
            /**
             * @brief Build sampled suffix arrays from a static RLBWT
             * @param static_rlbwt The static run-length encoded BWT
             * @param output_fst_sampling_sa Output vector for first SA values of each run
             * @param output_last_sampling_sa Output vector for last SA values of each run
             * @param message_paragraph Message indentation level for progress output
             * @throws std::logic_error if sampling fails for any run
             * @note Uses backward ISA traversal to compute SA values
             */
            static void build(const RLBWT &static_rlbwt, std::vector<uint64_t> &output_fst_sampling_sa, std::vector<uint64_t> &output_last_sampling_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = static_rlbwt.str_size();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing two sampled suffix arrays from static RLBWT..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();


                stool::WT wt = stool::rlbwt2::WaveletTreeOnHeadChars::build(&static_rlbwt);
                stool::rlbwt2::LightFPosDataStructure fpos_array;
                fpos_array.build(static_rlbwt.get_head_char_vec(), *static_rlbwt.get_lpos_vec(), &wt, stool::Message::increment_paragraph_level(message_paragraph));
                using LF_DATA = stool::rlbwt2::LFDataStructureBasedOnRLBWT<RLBWT, stool::rlbwt2::LightFPosDataStructure>;
                LF_DATA rle_wt(&static_rlbwt, &fpos_array);

                uint64_t end_marker_lposition = static_rlbwt.get_end_rle_lposition();
                uint64_t end_marker_position = static_rlbwt.get_lpos(end_marker_lposition);

                output_fst_sampling_sa.resize(static_rlbwt.rle_size(), UINT64_MAX);
                output_last_sampling_sa.resize(static_rlbwt.rle_size(), UINT64_MAX);

                stool::bwt::BackwardISA<LF_DATA> isa_ds;

                //uint64_t max_value_pos_in_suffix_array = rle_wt.lf(end_marker_position);
                //assert(max_value_pos_in_suffix_array == 0);


                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "Reading the input string by LF function..." << std::endl;
                }
                uint64_t message_counter = 10000000;
                uint64_t processed_text_length = 0;


                isa_ds.set(&rle_wt, end_marker_position, text_size);
                int64_t text_position = text_size;


                for (stool::bwt::BackwardISA<LF_DATA>::iterator it = isa_ds.begin(); it != isa_ds.end(); ++it)
                {

                    text_position--;
                    message_counter++;
                    processed_text_length++;
                    if (message_paragraph >= 0 && message_counter > 10000000)
                    {
                        std::cout <<  stool::Message::get_paragraph_string(message_paragraph+1)  << "[" << (processed_text_length/1000000) << "/" << (text_size/1000000) << "MB] \r" << std::flush;
                        message_counter=0;
                    }

                    uint64_t lindex = static_rlbwt.get_lindex_containing_the_position(*it);
                    uint64_t run_length = static_rlbwt.get_run(lindex);
                    uint64_t starting_position = static_rlbwt.get_lpos(lindex);
                    uint64_t diff = (*it) - starting_position;


                    if (diff == 0)
                    {
                        output_fst_sampling_sa[lindex] = text_position;
                    }
                    if (diff + 1 == run_length)
                    {
                        output_last_sampling_sa[lindex] = text_position;
                    }
                }
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << std::endl;
                    std::cout << stool::Message::get_paragraph_string(message_paragraph+1) << "[END]" << std::endl;
                }


                for (uint64_t i = 0; i < output_fst_sampling_sa.size(); i++)
                {
                    if (output_fst_sampling_sa[i] == UINT64_MAX)
                    {
                        throw std::logic_error("Error1/ i = " + std::to_string(i));
                    }
                    if (output_last_sampling_sa[i] == UINT64_MAX)
                    {
                        throw std::logic_error("Error2");
                    }
                }

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double) text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }

            /*
            static void build(std::string file_path, std::vector<uint64_t> &output_fst_sampling_sa, std::vector<uint64_t> &output_last_sampling_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                // RLBWT static_rlbwt;
                stool::rlbwt2::BWTAnalysisResult analyzer;
                RLBWT static_rlbwt = RLBWT::build(file_path, analyzer);


                // isa_ds.set()
            }
            */

            /*
            static void build_sampling_suffix_array(const DynamicRLBWT &rlbwt, std::vector<uint64_t> &fst_samp_sa, std::vector<uint64_t> &last_samp_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0 && rlbwt.text_size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Sampled Suffix Array from RLBWT..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                int64_t size = rlbwt.run_count();
                last_samp_sa.resize(size, UINT64_MAX);
                fst_samp_sa.resize(size, UINT64_MAX);
                // next_samp_sa.resize(size, UINT64_MAX);

                int64_t len = rlbwt.text_size();
                int64_t pos = rlbwt.LF(rlbwt.get_end_marker_pos());
                for (int64_t i = 0; i < len; i++)
                {
                    if ((i + 1) % 1000 == 0)
                    {
                        std::cout << "[" << i << "/" << len << "]" << std::endl;
                    }
                    RunPosition rp = rlbwt.to_run_position(pos);
                    uint64_t run_length = rlbwt.get_run_length(rp.run_index);
                    if (rp.position_in_run + 1 == run_length)
                    {
                        assert(last_samp_sa[rp.run_index] == UINT64_MAX);
                        last_samp_sa[rp.run_index] = len - 1 - i;
                    }
                    if (rp.position_in_run == 0)
                    {
                        assert(fst_samp_sa[rp.run_index] == UINT64_MAX);

                        fst_samp_sa[rp.run_index] = len - 1 - i;
                    }
                    pos = rlbwt.LF(rp.run_index, rp.position_in_run);
                }

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && rlbwt.text_size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)rlbwt.text_size()) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }
            */
        };
    }
}

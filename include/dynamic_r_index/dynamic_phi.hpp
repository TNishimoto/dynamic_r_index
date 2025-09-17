#pragma once
#include "./dynamic_partial_sa.hpp"
#include "stool/include/stool.hpp"
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        ////////////////////////////////////////////////////////////////////////////////
        /// @class      DynamicPhi
        /// @brief      A dynamic data structure supporting phi and inverse phi queries.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class DynamicPhi
        {
        public:
            DynamicPartialSA sampled_first_sa;
            DynamicPartialSA sampled_last_sa;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief Initialize this data strcuture
            ///   @param degree The degree of the B-tree used in this data structure
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            DynamicPhi()
            {
                this->clear();
            }
            DynamicPhi &operator=(const DynamicPhi &) = delete;
            DynamicPhi(DynamicPhi &&) noexcept = default;
            DynamicPhi &operator=(DynamicPhi &&) noexcept = default;

            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicPhi):" << std::endl;
                this->sampled_first_sa.print_statistics(message_paragraph + 1);
                this->sampled_last_sa.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicPhi):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "sampled_first_sa: " << std::flush;
                this->sampled_first_sa.print_content(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "sampled_last_sa: " << std::flush;

                this->sampled_last_sa.print_content(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /*
            void set_degree(int64_t degree)
            {
                this->sampled_first_sa.set_degree(degree);
                this->sampled_last_sa.set_degree(degree);
            }
            */
            void clear()
            {
                this->sampled_first_sa.clear();
                this->sampled_last_sa.clear();
            }

            static void store_to_file(DynamicPhi &item, std::ofstream &os)
            {
                DynamicPartialSA::store_to_file(item.sampled_first_sa, os);
                DynamicPartialSA::store_to_file(item.sampled_last_sa, os);
            }
            static DynamicPhi load_from_file(std::ifstream &ifs)
            {
                DynamicPartialSA tmp_fsa = DynamicPartialSA::load_from_file(ifs);
                DynamicPartialSA tmp_lsa = DynamicPartialSA::load_from_file(ifs);

                DynamicPhi r;
                r.sampled_first_sa.swap(tmp_fsa);
                r.sampled_last_sa.swap(tmp_lsa);

                return r;
            }

            /*
            void swap(DynamicPhi &item){

            }
            */

            void swap(DynamicPhi &item)
            {
                this->sampled_first_sa.swap(item.sampled_first_sa);
                this->sampled_last_sa.swap(item.sampled_last_sa);
            }
            const DynamicPartialSA *get_pointer_of_first_sa_values_of_BWT_run() const
            {
                return &this->sampled_first_sa;
            }
            const DynamicPartialSA *get_pointer_of_last_sa_values_of_BWT_run() const
            {
                return &this->sampled_last_sa;
            }
            static DynamicPhi build_from_sampled_sa_indexes(const std::vector<uint64_t> &sampled_last_sa_indexes, std::vector<uint64_t> &sampled_first_sa_indexes, uint64_t __text_size, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0 && __text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic Phi from Sampled Suffix Array..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicPhi r;
                // this->sampled_first_sa.build(sampled_first_sa_indexes, __text_size, stool::Message::add_message_paragraph(message_paragraph));
                // this->sampled_last_sa.build(sampled_last_sa_indexes, __text_size, stool::Message::add_message_paragraph(message_paragraph));

                // DynamicPartialSA::build_from_sampled_sa_indexes(this->sampled_first_sa, sampled_first_sa_indexes, __text_size, stool::Message::add_message_paragraph(message_paragraph));
                // this->sampled_first_sa.verify();

                // this->sampled_first_sa.swap(tmp_fsa);

                auto tmp_fsa = DynamicPartialSA::build_from_sampled_sa_indexes(sampled_first_sa_indexes, __text_size, stool::Message::add_message_paragraph(message_paragraph));
                r.sampled_first_sa.swap(tmp_fsa);

                auto tmp_lsa = DynamicPartialSA::build_from_sampled_sa_indexes(sampled_last_sa_indexes, __text_size, stool::Message::add_message_paragraph(message_paragraph));
                r.sampled_last_sa.swap(tmp_lsa);

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && __text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)__text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            static DynamicPhi build_from_RLBWT(const stool::rlbwt2::RLE<uint8_t> &static_rlbwt, [[maybe_unused]] int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                using RLBWT = stool::rlbwt2::RLE<uint8_t>;
                uint64_t p1 = static_rlbwt.get_end_rle_lposition();
                uint64_t p2 = static_rlbwt.get_lpos(p1);
                uint64_t rle_size = static_rlbwt.rle_size();

                stool::WT wt = stool::rlbwt2::WaveletTreeOnHeadChars::build(&static_rlbwt);
                stool::rlbwt2::LightFPosDataStructure fpos_array;
                fpos_array.build(static_rlbwt.get_head_char_vec(), *static_rlbwt.get_lpos_vec(), &wt);
                using LF_DATA = stool::rlbwt2::LFDataStructureBasedOnRLBWT<RLBWT, stool::rlbwt2::LightFPosDataStructure>;
                LF_DATA rle_wt(&static_rlbwt, &fpos_array);

                stool::bwt::BackwardISA<LF_DATA> isa_ds;

                uint64_t p3 = rle_wt.lf(p2);

                uint64_t text_size = static_rlbwt.str_size();

                isa_ds.set(&rle_wt, p3, text_size);
                int64_t text_position = text_size;

                DynamicPartialSA fsa;
                fsa.clear();
                //fsa.set_degree(degree);
                DynamicPartialSA lsa;
                lsa.clear();
                //lsa.set_degree(degree);

                stool::bptree::DynamicPermutation &first_dp = fsa.get_dynamic_permutation();
                stool::bptree::SimpleDynamicPrefixSum &first_spsi = fsa.get_spsi();
                stool::bptree::DynamicPermutation &last_dp = lsa.get_dynamic_permutation();
                stool::bptree::SimpleDynamicPrefixSum &last_spsi = lsa.get_spsi();

                stool::bptree::DynamicPermutationBuilder first_dpb, last_dpb;
                first_dpb.initialize(first_dp, rle_size);
                last_dpb.initialize(last_dp, rle_size);

                uint64_t x_text_position = UINT64_MAX;
                uint64_t y_text_position = UINT64_MAX;

                for (stool::bwt::BackwardISA<LF_DATA>::iterator it = isa_ds.begin(); it != isa_ds.end(); ++it)
                {

                    text_position--;
                    if (text_position % 10000000 == 0)
                    {
                        std::cout << "[" << text_position << "/" << text_size << "]" << std::endl;
                    }

                    uint64_t lindex = static_rlbwt.get_lindex_containing_the_position(*it);
                    uint64_t run_length = static_rlbwt.get_run(lindex);
                    uint64_t starting_position = static_rlbwt.get_lpos(lindex);
                    uint64_t diff = (*it) - starting_position;

                    if (diff == 0)
                    {
                        first_dpb.push_front(lindex);
                        if (x_text_position != UINT64_MAX)
                        {
                            uint64_t diff = x_text_position - text_position;
                            first_spsi.push_front(diff);
                        }
                        x_text_position = text_position;
                        // output_fst_sampling_sa[lindex] = text_position;
                    }
                    if (diff + 1 == run_length)
                    {
                        last_dpb.push_front(lindex);
                        if (y_text_position != UINT64_MAX)
                        {
                            uint64_t diff = y_text_position - text_position;
                            last_spsi.push_front(diff);
                        }
                        y_text_position = text_position;

                        // output_last_sampling_sa[lindex] = text_position;
                    }
                }
                if (text_position != 0)
                {
                    throw std::logic_error("Error: build_from_bwt_file");
                }
                if (x_text_position != UINT64_MAX)
                {
                    first_spsi.push_front(x_text_position);
                }
                if (y_text_position != UINT64_MAX)
                {
                    last_spsi.push_front(y_text_position);
                }

                first_dpb.finish();
                last_dpb.finish();
                fsa.set_text_size(static_rlbwt.str_size());
                lsa.set_text_size(static_rlbwt.str_size());

                DynamicPhi r;
                r.sampled_first_sa.swap(fsa);
                r.sampled_last_sa.swap(lsa);
                return r;
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Const Methods
            ///   The const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            uint64_t text_size() const
            {
                return this->sampled_first_sa._text_size;
            }
            uint64_t run_count() const
            {
                assert(this->sampled_first_sa.size() == this->sampled_last_sa.size());
                return this->sampled_first_sa.size();
            }
            uint64_t size() const
            {
                return this->text_size();
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief Compute the inverse phi function
            ///   @param i The input of the inverse phi function
            ////////////////////////////////////////////////////////////////////////////////
            uint64_t inverse_phi(uint64_t i) const
            {
                uint64_t size = this->sampled_last_sa.size();

                if (i >= this->text_size())
                {
                    throw std::logic_error("Error: inverse_phi");
                }

                int64_t idx = this->sampled_last_sa.circular_predecessor_index_query_on_sampled_isa(i);
                int64_t found_sa_index = this->sampled_last_sa.get_sampled_sa_index(idx);
                int64_t found_sa_value = this->sampled_last_sa.get_sampled_isa_value(idx);

                if (found_sa_index + 1 < (int64_t)size)
                {
                    int64_t next_sa = this->sampled_first_sa.get_sampled_sa_value(found_sa_index + 1);
                    uint64_t r = ((int64_t)i) + (next_sa - found_sa_value);
                    assert(r < this->text_size());
                    return r;
                }
                else
                {
                    int64_t next_sa = this->sampled_first_sa.get_sampled_sa_value(0);
                    uint64_t r = ((int64_t)i) + (next_sa - found_sa_value);
                    assert(r < this->text_size());
                    return r;
                }
            }
            uint64_t phi(uint64_t i) const
            {
                uint64_t size = this->sampled_last_sa.size();
                if (i >= this->text_size())
                {
                    std::cout << "Phi function error: The input position i is larger than the text size:" + std::to_string(i) + "/" + std::to_string(this->text_size()) << std::endl;
                    throw std::logic_error("Phi function error");
                }

                int64_t idx = this->sampled_first_sa.circular_predecessor_index_query_on_sampled_isa(i);
                assert(idx < this->sampled_first_sa.size());
                int64_t found_sa_index = this->sampled_first_sa.get_sampled_sa_index(idx);
                int64_t found_sa_value = this->sampled_first_sa.get_sampled_isa_value(idx);

                if (found_sa_index > 0)
                {
                    int64_t next_sa = this->sampled_last_sa.get_sampled_sa_value(found_sa_index - 1);
                    return ((int64_t)i) + (next_sa - found_sa_value);
                }
                else
                {
                    int64_t next_sa = this->sampled_last_sa.get_sampled_sa_value(size - 1);
                    return ((int64_t)i) + (next_sa - found_sa_value);
                }
            }

            bool verify(int mode = 0) const
            {
                bool b1 = this->sampled_first_sa.verify();
                bool b2 = this->sampled_last_sa.verify();

                if (mode == 0)
                {
                    auto sa = this->get_sa();
                    auto isa = this->get_isa();
                    DynamicRLBWT::verify_permutation(sa, "SA");
                    DynamicRLBWT::verify_permutation(isa, "ISA");
                }

                return b1 && b2;
            }
            uint64_t get_sampled_first_sa_value(uint64_t i) const
            {
                return this->sampled_first_sa.get_sampled_sa_value(i);
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief Compute the phi function
            ///   @param i The input of the phi function
            ////////////////////////////////////////////////////////////////////////////////

        public:
            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief Consider the situation where BWT[i] is replaced with character new_c for a position $i$ on BWT.
            ///   Then, this function returns phi^{-1}(SA[LF(i)]) (i.e., SA[LF(i)+1])
            ///   @param i_rp The integer i represented as a RunPosition
            ///   @param new_c BWT[i] is replaced with this character
            ///   @param sa_value_at_i_plus SA[i+1]
            ///   @param dbwt RLBWT
            ////////////////////////////////////////////////////////////////////////////////
            SAValue LF_inverse_phi(RunPosition i_rp, uint8_t new_c, SAValue sa_value_at_i_plus, const DynamicRLBWT &dbwt) const
            {

                uint8_t i_c = dbwt.get_char(i_rp.run_index);

                uint64_t run_length = dbwt.get_run_length(i_rp.run_index);

                if (i_rp.position_in_run + 1 < run_length && i_c == new_c)
                {
                    return sa_value_at_i_plus > 0 ? sa_value_at_i_plus - 1 : this->text_size() - 1;
                }
                else
                {
                    RunPosition rp = DynamicRLBWTHelper::proper_successor_on_F(dbwt, i_rp, new_c);

                    assert(rp.position_in_run == 0);
                    int64_t v = this->sampled_first_sa.get_sampled_sa_value(rp.run_index);
                    return v > 0 ? v - 1 : this->text_size() - 1;
                }

                // return LF_inverse_phi(i, old_c, c, next_sa_value, dbwt);
            }

            SAValue LF_inverse_phi_for_deletion_sub(RunPosition u, const DynamicRLBWT &dbwt, uint64_t u_value_inv_phi) const
            {
                uint64_t run_length = dbwt.get_run_length(u.run_index);
                if (u.position_in_run + 1 < run_length)
                {
                    return u_value_inv_phi > 0 ? u_value_inv_phi - 1 : this->text_size() - 1;
                }
                else
                {
                    uint8_t c = dbwt.get_char(u.run_index);
                    // uint64_t p = dbwt.get_position(u.run_index, u.position_in_run);
                    RunPosition rp2 = DynamicRLBWTHelper::proper_successor_on_F(dbwt, u, c);
                    int64_t v = this->sampled_first_sa.get_sampled_sa_value(rp2.run_index);
                    return v > 0 ? v - 1 : this->text_size() - 1;
                }
            }

            SAValue LF_inverse_phi_for_deletion(RunPosition i_rp, SAValue sa_value_at_i_plus, const DynamicRLBWT &dbwt, uint64_t i_pos, uint64_t next_i_pos, uint64_t u_value, uint64_t u_value_inv_phi) const
            {
                if (next_i_pos + 1 == i_pos)
                {
                    return sa_value_at_i_plus;
                }
                else
                {
                    uint8_t i_c = dbwt.get_char(i_rp.run_index);

                    uint64_t run_length = dbwt.get_run_length(i_rp.run_index);

                    if (i_rp.position_in_run + 1 < run_length)
                    {
                        if (sa_value_at_i_plus == u_value)
                        {

                            RunPosition rp = dbwt.to_run_position(i_pos + 1);
                            return LF_inverse_phi_for_deletion_sub(rp, dbwt, u_value_inv_phi);
                        }
                        else
                        {

                            return sa_value_at_i_plus > 0 ? sa_value_at_i_plus - 1 : this->text_size() - 1;
                        }
                    }
                    else
                    {

                        RunPosition rp = DynamicRLBWTHelper::proper_successor_on_F(dbwt, i_rp, i_c);

                        assert(rp.position_in_run == 0);
                        int64_t v = this->sampled_first_sa.get_sampled_sa_value(rp.run_index);
                        if (v == (int64_t)u_value)
                        {
                            return LF_inverse_phi_for_deletion_sub(rp, dbwt, u_value_inv_phi);
                        }
                        else
                        {
                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                }
            }

            bool check_whether_succeeding_replaced_char(RunPosition i_rp, uint8_t replaced_char, SAIndex replaced_sa_index, const DynamicRLBWT &dbwt) const
            {
                uint8_t i_c = dbwt.get_char(i_rp.run_index);
                uint64_t i = dbwt.get_position(i_rp.run_index, i_rp.position_in_run);
                return replaced_char < i_c || ((replaced_char == i_c) && (replaced_sa_index <= i));
            }

            int64_t LF_phi_for_move(RunPosition i_on_rlbwt, SAValue sa_value_at_i_minus, const DynamicRLBWT &dbwt) const
            {

                if (i_on_rlbwt.position_in_run > 0)
                {
                    return sa_value_at_i_minus > 0 ? sa_value_at_i_minus - 1 : this->text_size() - 1;
                }
                else
                {
                    uint8_t i_c = dbwt.get_char(i_on_rlbwt.run_index);
                    RunPosition i_rp_prev = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, i_on_rlbwt, i_c);
                    int64_t v = this->sampled_last_sa.get_sampled_sa_value(i_rp_prev.run_index);
                    return v > 0 ? v - 1 : this->text_size() - 1;
                }
            };
            SAValue LF_inverse_phi_for_move(RunPosition i_on_rlbwt, SAValue sa_value_at_i_plus, const DynamicRLBWT &dbwt) const
            {
                uint64_t run_length = dbwt.get_run_length(i_on_rlbwt.run_index);

                if (i_on_rlbwt.position_in_run + 1 < run_length)
                {
                    return sa_value_at_i_plus > 0 ? sa_value_at_i_plus - 1 : this->text_size() - 1;
                }
                else
                {
                    uint8_t i_c = dbwt.get_char(i_on_rlbwt.run_index);
                    RunPosition i_rp_next = DynamicRLBWTHelper::proper_successor_on_F(dbwt, i_on_rlbwt, i_c);
                    int64_t v = this->sampled_first_sa.get_sampled_sa_value(i_rp_next.run_index);
                    return v > 0 ? v - 1 : this->text_size() - 1;
                }
            }

            /*
            Memo: This method should be replaced with LF_inverse_phi_for_insertionX.
            */
            SAValue LF_inverse_phi_for_insertion(RunPosition i_rp, SAValue sa_value_at_i_plus, uint8_t replaced_char, SAIndex replaced_sa_index, uint64_t insertion_pos, const DynamicRLBWT &dbwt) const
            {
                uint8_t i_c = dbwt.get_char(i_rp.run_index);
                uint64_t run_length = dbwt.get_run_length(i_rp.run_index);
                bool b1 = this->check_whether_succeeding_replaced_char(i_rp, replaced_char, replaced_sa_index, dbwt);

                if (i_rp.position_in_run + 1 < run_length)
                {
                    RunPosition i_rp_next = RunPosition(i_rp.run_index, i_rp.position_in_run + 1);
                    bool b2 = this->check_whether_succeeding_replaced_char(i_rp_next, replaced_char, replaced_sa_index, dbwt);
                    if (b1 == false && b2 == true)
                    {
                        return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                    }
                    else
                    {
                        return sa_value_at_i_plus > 0 ? sa_value_at_i_plus - 1 : this->text_size() - 1;
                    }
                }
                else
                {
                    RunPosition i_rp_next = DynamicRLBWTHelper::proper_successor_on_F(dbwt, i_rp, i_c);
                    bool b2 = this->check_whether_succeeding_replaced_char(i_rp_next, replaced_char, replaced_sa_index, dbwt);                    
                    uint8_t i_c_next = dbwt.get_char(i_rp_next.run_index);



                    if (i_c <= i_c_next)
                    {
                        if (b1 == false && b2 == true)
                        {
                            return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                        }
                        else
                        {

                            int64_t v = this->sampled_first_sa.get_sampled_sa_value(i_rp_next.run_index);

                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                    else
                    {

                        if (b1 == b2)
                        {
                            return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                        }
                        else
                        {

                            int64_t v = this->sampled_first_sa.get_sampled_sa_value(i_rp_next.run_index);
                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                }
            }
            /*
            Memo: This method contains a bug, but dynamic r-index works correctly. I will fix this method later.
            */
            SAValue LF_inverse_phi_for_insertionX(RunPosition i_rp, SAValue sa_value_at_i_plus, SAIndex i_minus_p, uint64_t y_p, uint64_t insertion_pos, const DynamicRLBWT &dbwt) const
            {
                uint64_t b1 = false;
                if (y_p + 1 < this->text_size())
                {
                    b1 = (i_minus_p == y_p + 1);
                }
                else
                {
                    b1 = (i_minus_p == 0);
                }


                if (b1)
                {
                    return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                }
                else
                {
                    uint64_t run_length = dbwt.get_run_length(i_rp.run_index);
                    if (i_rp.position_in_run + 1 < run_length)
                    {
                        return sa_value_at_i_plus > 0 ? sa_value_at_i_plus - 1 : this->text_size() - 1;
                    }
                    else
                    {
                        uint8_t i_c = dbwt.get_char(i_rp.run_index);
                        RunPosition i_rp_next = DynamicRLBWTHelper::proper_successor_on_F(dbwt, i_rp, i_c);
                        int64_t v = this->sampled_first_sa.get_sampled_sa_value(i_rp_next.run_index);
                        return v > 0 ? v - 1 : this->text_size() - 1;
                    }
                }
            }

            SAValue LF_phi_for_insertionX(RunPosition i_rp, SAValue sa_value_at_i_minus, SAIndex i_minus_p, uint64_t y_p, uint64_t insertion_pos, const DynamicRLBWT &dbwt) const
            {
                uint64_t b1 = false;
                if (y_p != 0)
                {
                    b1 = i_minus_p + 1 == y_p;
                }
                else
                {
                    b1 = i_minus_p == this->text_size();
                }

                if (b1)
                {
                    return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                }
                else
                {
                    if (i_rp.position_in_run != 0)
                    {
                        return sa_value_at_i_minus > 0 ? sa_value_at_i_minus - 1 : this->text_size() - 1;
                    }
                    else
                    {
                        uint8_t i_c = dbwt.get_char(i_rp.run_index);

                        RunPosition i_rp_prev = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, i_rp, i_c);
                        int64_t v = this->sampled_last_sa.get_sampled_sa_value(i_rp_prev.run_index);
                        return v > 0 ? v - 1 : this->text_size() - 1;
                    }
                }
            }

            /*
            Memo: This method should be replaced with LF_inverse_phi_for_insertionX.
            */
            SAValue LF_phi_for_insertion(RunPosition i_rp, SAValue sa_value_at_i_minus, uint8_t replaced_char, SAIndex replaced_sa_index, uint64_t insertion_pos, const DynamicRLBWT &dbwt) const
            {
                uint8_t i_c = dbwt.get_char(i_rp.run_index);
                // uint64_t run_length = dbwt.get_run_length(i_rp.run_index);
                bool b1 = this->check_whether_succeeding_replaced_char(i_rp, replaced_char, replaced_sa_index, dbwt);

                if (i_rp.position_in_run > 0)
                {
                    RunPosition i_rp_prev = RunPosition(i_rp.run_index, i_rp.position_in_run - 1);
                    bool b2 = this->check_whether_succeeding_replaced_char(i_rp_prev, replaced_char, replaced_sa_index, dbwt);
                    if (b1 == true && b2 == false)
                    {
                        return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                    }
                    else
                    {
                        return sa_value_at_i_minus > 0 ? sa_value_at_i_minus - 1 : this->text_size() - 1;
                    }
                }
                else
                {
                    RunPosition i_rp_prev = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, i_rp, i_c);
                    uint8_t i_c_prev = dbwt.get_char(i_rp_prev.run_index);
                    bool b2 = this->check_whether_succeeding_replaced_char(i_rp_prev, replaced_char, replaced_sa_index, dbwt);

                    if (i_c_prev <= i_c)
                    {
                        if (b1 == true && b2 == false)
                        {
                            return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                        }
                        else
                        {
                            int64_t v = this->sampled_last_sa.get_sampled_sa_value(i_rp_prev.run_index);
                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                    else
                    {
                        if (b1 == b2)
                        {
                            return insertion_pos > 0 ? insertion_pos - 1 : this->text_size() - 1;
                        }
                        else
                        {
                            int64_t v = this->sampled_last_sa.get_sampled_sa_value(i_rp_prev.run_index);
                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @brief Consider the situation where old_c = BWT[i] is replaced with character new_c for a position $i$ on BWT.
            ///   Then, this function returns phi(SA[LF(i)])
            ////////////////////////////////////////////////////////////////////////////////

            SAValue LF_phi(RunPosition i, uint8_t old_c, uint8_t new_c, SAValue prev_sa_value, const DynamicRLBWT &dbwt) const
            {

                if (i.position_in_run > 0 && old_c == new_c)
                {
                    return prev_sa_value > 0 ? prev_sa_value - 1 : this->text_size() - 1;
                }
                else
                {

                    assert(i.position_in_run == 0);
                    RunPosition rp = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, i, new_c);
#if DEBUG
                    uint64_t run_length = dbwt.get_run_length(rp.run_index);
                    assert(rp.position_in_run + 1 == run_length);
#endif
                    int64_t v = this->sampled_last_sa.get_sampled_sa_value(rp.run_index);
                    return v > 0 ? v - 1 : this->text_size() - 1;
                }
            }

            SAValue LF_phi_for_deletion_sub(RunPosition u, const DynamicRLBWT &dbwt, uint64_t u_value_phi) const
            {
                if (u.position_in_run > 0)
                {
                    return u_value_phi > 0 ? u_value_phi - 1 : this->text_size() - 1;
                }
                else
                {
                    uint8_t c = dbwt.get_char(u.run_index);
                    // uint64_t p = dbwt.get_position(u.run_index, u.position_in_run);
                    RunPosition rp2 = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, u, c);
                    int64_t v = this->sampled_last_sa.get_sampled_sa_value(rp2.run_index);
                    return v > 0 ? v - 1 : this->text_size() - 1;
                }
            }

            SAValue LF_phi_for_deletion(RunPosition i, SAValue prev_sa_value, const DynamicRLBWT &dbwt, uint64_t i_pos, uint64_t next_i_pos, uint64_t u_value, uint64_t u_value_phi) const
            {
                if (i_pos == next_i_pos)
                {
                    return prev_sa_value;
                }
                else
                {

                    uint8_t old_c = dbwt.get_char(i.run_index);

                    if (i.position_in_run > 0)
                    {
                        if (prev_sa_value == u_value)
                        {

                            RunPosition rp = dbwt.to_run_position(i_pos - 1);
                            return LF_phi_for_deletion_sub(rp, dbwt, u_value_phi);
                        }
                        else
                        {

                            return prev_sa_value > 0 ? prev_sa_value - 1 : this->text_size() - 1;
                        }
                    }
                    else
                    {

                        assert(i.position_in_run == 0);
                        RunPosition rp = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, i, old_c);
                        int64_t v = this->sampled_last_sa.get_sampled_sa_value(rp.run_index);

                        if (v == (int64_t)u_value)
                        {

                            return LF_phi_for_deletion_sub(rp, dbwt, u_value_phi);
                        }
                        else
                        {

                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                }
            }

            /*
            SAValue LF_phi_for_deletion(RunPosition i, SAValue prev_sa_value, const DynamicRLBWT &dbwt, uint64_t i_pos, uint64_t next_i_pos, uint64_t u_value, uint64_t u_value_phi) const
            {
                if (i_pos == next_i_pos)
                {
                    return prev_sa_value;
                }
                else
                {

                    uint8_t old_c = dbwt.get_char(i.run_index);

                    if (i.position_in_run > 0)
                    {
                        if (prev_sa_value == u_value)
                        {
                            RunPosition rp = dbwt.to_run_position(i_pos - 1);
                            return LF_phi_for_deletion_sub(rp, dbwt, u_value_phi);
                        }
                        else
                        {
                            return prev_sa_value > 0 ? prev_sa_value - 1 : this->text_size() - 1;
                        }
                    }
                    else
                    {

                        assert(i.position_in_run == 0);
                        RunPosition rp = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, i, old_c);
                        int64_t v = this->sampled_last_sa.get_sampled_sa_value(rp.run_index);

                        if (v == (int64_t)u_value)
                        {
                            return LF_phi_for_deletion_sub(rp, dbwt, u_value_phi);
                        }
                        else
                        {
                            return v > 0 ? v - 1 : this->text_size() - 1;
                        }
                    }
                }
            }
            */

            SAValue LF_phi(RunPosition i, SAValue prev_sa_value, const DynamicRLBWT &dbwt) const
            {
                uint8_t old_c = dbwt.get_char(i.run_index);
                return this->LF_phi(i, old_c, old_c, prev_sa_value, dbwt);
            }

            SAValue LF_phi(RunPosition i, uint8_t c, SAValue prev_sa_value, const DynamicRLBWT &dbwt) const
            {

                uint8_t old_c = dbwt.get_char(i.run_index);
                return this->LF_phi(i, old_c, c, prev_sa_value, dbwt);
            }

            std::pair<uint64_t, uint64_t> nearest_search_by_text_position(uint64_t text_position, const DynamicRLBWT &dbwt) const
            {
                auto pair1 = this->sampled_first_sa.nearest_search_by_text_position(text_position);
                auto pair2 = this->sampled_first_sa.nearest_search_by_text_position(text_position);

                uint64_t t1 = pair1.second;
                uint64_t t2 = pair2.second;

                uint64_t dist1 = t1 <= text_position ? text_position - t1 : t1 - text_position;
                uint64_t dist2 = t2 <= text_position ? text_position - t2 : t2 - text_position;

                if (dist1 <= dist2)
                {
                    int64_t idx_on_samp_sa = this->sampled_first_sa.get_sampled_sa_index(pair1.first);
                    uint64_t j_on_sa = dbwt.get_position(idx_on_samp_sa, 0);
                    return std::pair<uint64_t, uint64_t>(j_on_sa, t2);
                }
                else
                {
                    int64_t idx_on_samp_sa = this->sampled_last_sa.get_sampled_sa_index(pair2.first);
                    int64_t run_length = dbwt.get_run_length(idx_on_samp_sa);
                    uint64_t j_on_sa = dbwt.get_position(idx_on_samp_sa, run_length - 1);
                    return std::pair<uint64_t, uint64_t>(j_on_sa, t2);
                }
            }

            uint64_t isa(uint64_t i, const DynamicRLBWT &dbwt) const
            {
                std::pair<uint64_t, uint64_t> sa_pair = this->nearest_search_by_text_position(i, dbwt);
                uint64_t sa_value = sa_pair.second;
                uint64_t j_on_sa = sa_pair.first;
                uint64_t text_size = dbwt.text_size();

                if (sa_value < i)
                {
                    uint64_t dist1 = i - sa_value;
                    uint64_t dist2 = sa_value + (text_size - i);
                    if (dist1 <= dist2)
                    {
                        for (uint64_t x = 0; x < dist1; x++)
                        {
                            j_on_sa = dbwt.inverse_LF(j_on_sa);
                        }
                    }
                    else
                    {
                        for (uint64_t x = 0; x < dist2; x++)
                        {
                            j_on_sa = dbwt.LF(j_on_sa);
                        }
                    }
                }
                else if (sa_value > i)
                {
                    uint64_t dist1 = sa_value - i;
                    uint64_t dist2 = i + (text_size - sa_value);
                    if (dist1 <= dist2)
                    {
                        for (uint64_t x = 0; x < dist1; x++)
                        {
                            j_on_sa = dbwt.LF(j_on_sa);
                        }
                    }
                    else
                    {
                        for (uint64_t x = 0; x < dist2; x++)
                        {
                            j_on_sa = dbwt.inverse_LF(j_on_sa);
                        }
                    }
                }
                return j_on_sa;
            }

            SAValue get_sa(int64_t i, bool is_cirular_array = false) const
            {
                std::vector<uint64_t> sa = this->get_sa();
                if (is_cirular_array)
                {
                    if (i == -1)
                    {
                        return sa[this->text_size() - 1];
                    }
                    else if (i == (int64_t)this->text_size())
                    {
                        return sa[0];
                    }
                    else
                    {
                        return sa[i];
                    }
                }
                else
                {
                    return sa[i];
                }
            }

            std::vector<uint64_t> get_sa() const
            {

                std::vector<uint64_t> sa;
                int64_t sa_size = this->text_size();
                sa.resize(sa_size, UINT64_MAX);
                uint64_t sa_value = this->sampled_first_sa.get_sampled_sa_value(0);

                sa[0] = sa_value;

                for (int64_t i = 1; i < sa_size; i++)
                {
                    assert(sa_value < this->text_size());
                    sa_value = this->inverse_phi(sa_value);
                    assert(sa_value < this->text_size());

                    sa[i] = sa_value;
                }

                return sa;
            }

            std::vector<uint64_t> get_sampling_sa(const DynamicRLBWT &dbwt) const
            {
                std::vector<uint64_t> sa;
                int64_t sa_size = this->text_size();
                sa.resize(sa_size, UINT64_MAX);
                for (uint64_t i = 0; i < (uint64_t)this->sampled_first_sa.size(); i++)
                {
                    uint64_t p = dbwt.get_starting_position(i);
                    uint64_t len = dbwt.get_run_length(i);
                    sa[p] = this->sampled_first_sa.get_sampled_sa_value(i);
                    sa[p + len - 1] = this->sampled_last_sa.get_sampled_sa_value(i);
                }
                return sa;
            }

            std::vector<uint64_t> get_first_sa_values() const
            {
                std::vector<uint64_t> sa;
                int64_t sa_size = this->run_count();
                sa.resize(sa_size, UINT64_MAX);
                for (uint64_t i = 0; i < (uint64_t)this->sampled_first_sa.size(); i++)
                {
                    sa[i] = this->sampled_first_sa.get_sampled_sa_value(i);
                }
                return sa;
            }
            std::vector<uint64_t> get_last_sa_values() const
            {
                std::vector<uint64_t> sa;
                int64_t sa_size = this->run_count();
                sa.resize(sa_size, UINT64_MAX);
                for (uint64_t i = 0; i < (uint64_t)this->sampled_first_sa.size(); i++)
                {
                    sa[i] = this->sampled_last_sa.get_sampled_sa_value(i);
                }
                return sa;
            }

            std::vector<uint64_t> get_isa() const
            {
                std::vector<uint64_t> isa;
                int64_t sa_size = this->text_size();

                isa.resize(sa_size, UINT64_MAX);
                uint64_t sa_value = this->sampled_last_sa.get_sampled_sa_value(this->sampled_last_sa.size() - 1);

                isa[sa_value] = sa_size - 1;

                for (int64_t i = 1; i < sa_size; i++)
                {
                    sa_value = this->phi(sa_value);
                    isa[sa_value] = sa_size - 1 - i;
                }

                return isa;
            }
            void print() const
            {
                std::vector<uint64_t> fsa = this->sampled_first_sa.get_sampled_sa_values();
                std::vector<uint64_t> lsa = this->sampled_last_sa.get_sampled_sa_values();

                std::cout << "========= DynamicPhi ==========" << std::endl;
                stool::Printer::print("Sampled first sa values", fsa);
                stool::Printer::print("Sampled last sa values", lsa);
                std::cout << "===============================" << std::endl;
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Non-const Methods
            ///   The non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            void extend_text(int64_t new_position)
            {
                this->sampled_first_sa.extend_text(new_position);
                this->sampled_last_sa.extend_text(new_position);
                // assert(this->verify());
            }
            void shrink_text(int64_t remove_position)
            {
                this->sampled_first_sa.shrink_text(remove_position);
                this->sampled_last_sa.shrink_text(remove_position);
                // assert(this->verify());
            }

            void replace_element_for_insertion(int64_t replaced_sa_index, int64_t upper_sa_value, int64_t new_sa_value, int64_t lower_sa_value, const DetailedReplacement type)
            {
                if (type.no_replancement)
                {
                    if (type.is_first)
                    {
                        this->sampled_first_sa.replace(replaced_sa_index, new_sa_value);
                    }

                    if (type.is_last)
                    {
                        this->sampled_last_sa.replace(replaced_sa_index, new_sa_value);
                    }
                }
                else
                {
                    if (type.is_first)
                    {
                        if (type.is_last)
                        {
                            if (type.upper_merge)
                            {
                                if (type.lower_merge)
                                {
                                    this->sampled_first_sa.remove(replaced_sa_index + 1);
                                    this->sampled_first_sa.remove(replaced_sa_index);

                                    this->sampled_last_sa.remove(replaced_sa_index);
                                    this->sampled_last_sa.remove(replaced_sa_index - 1);
                                }
                                else
                                {
                                    this->sampled_first_sa.remove(replaced_sa_index);
                                    this->sampled_last_sa.remove(replaced_sa_index);
                                    this->sampled_last_sa.replace(replaced_sa_index - 1, new_sa_value);
                                }
                            }
                            else
                            {
                                if (type.lower_merge)
                                {
                                    this->sampled_first_sa.replace(replaced_sa_index + 1, new_sa_value);
                                    this->sampled_first_sa.remove(replaced_sa_index);
                                    this->sampled_last_sa.remove(replaced_sa_index);
                                }
                                else
                                {
                                    this->sampled_first_sa.replace(replaced_sa_index, new_sa_value);
                                    this->sampled_last_sa.replace(replaced_sa_index, new_sa_value);
                                }
                            }
                        }
                        else
                        {
                            if (type.upper_merge)
                            {
                                this->sampled_last_sa.replace(replaced_sa_index - 1, new_sa_value);
                                this->sampled_first_sa.replace(replaced_sa_index, lower_sa_value);
                            }
                            else
                            {
                                this->sampled_first_sa.replace(replaced_sa_index, lower_sa_value);
                                this->sampled_first_sa.insert(replaced_sa_index, new_sa_value);
                                this->sampled_last_sa.insert(replaced_sa_index, new_sa_value);
                            }
                        }
                    }
                    else
                    {
                        if (type.is_last)
                        {
                            if (type.lower_merge)
                            {
                                this->sampled_first_sa.replace(replaced_sa_index + 1, new_sa_value);
                                this->sampled_last_sa.replace(replaced_sa_index, upper_sa_value);
                            }
                            else
                            {
                                this->sampled_last_sa.replace(replaced_sa_index, upper_sa_value);
                                this->sampled_first_sa.insert(replaced_sa_index + 1, new_sa_value);
                                this->sampled_last_sa.insert(replaced_sa_index + 1, new_sa_value);
                            }
                        }
                        else
                        {
                            this->sampled_last_sa.insert(replaced_sa_index, new_sa_value);
                            this->sampled_last_sa.insert(replaced_sa_index, upper_sa_value);

                            this->sampled_first_sa.insert(replaced_sa_index + 1, lower_sa_value);
                            this->sampled_first_sa.insert(replaced_sa_index + 1, new_sa_value);
                        }
                    }
                }
            }
            void insert_element_for_insertion(SampledSAIndex insertion_position, int64_t upper_sa_value, int64_t new_sa_value, int64_t lower_sa_value, RunInsertionType type)
            {
                assert(insertion_position <= this->run_count());

                if (type == RunInsertionType::FirstCharInsertion)
                {
                    this->sampled_first_sa.replace(insertion_position, new_sa_value);
                }
                else if (type == RunInsertionType::LastCharInsertion)
                {
                    this->sampled_last_sa.replace(insertion_position, new_sa_value);
                }
                else if (type == RunInsertionType::NewRunInsertionWithSplit)
                {
                    this->sampled_last_sa.insert(insertion_position, new_sa_value);
                    this->sampled_last_sa.insert(insertion_position, upper_sa_value);

                    this->sampled_first_sa.insert(insertion_position + 1, lower_sa_value);
                    this->sampled_first_sa.insert(insertion_position + 1, new_sa_value);
                }
                else if (type == RunInsertionType::NewRunInsertionWithoutSplit)
                {
                    this->sampled_last_sa.insert(insertion_position, new_sa_value);
                    this->sampled_first_sa.insert(insertion_position, new_sa_value);
                }
            }
            void remove_element_for_insertion(SampledSAIndex replaced_sa_index, int64_t upper_sa_value, int64_t lower_sa_value, RunRemovalType type)
            {

                if (type == RunRemovalType::FirstCharRemoval)
                {
                    this->sampled_first_sa.replace(replaced_sa_index, lower_sa_value);
                }
                else if (type == RunRemovalType::LastCharRemoval)
                {
                    this->sampled_last_sa.replace(replaced_sa_index, upper_sa_value);
                }
                else if (type == RunRemovalType::CenterCharRemoval)
                {
                    /*
                    assert(replaced_sa_index <= (uint64_t)this->sampled_last_sa.size());
                    this->sampled_last_sa.insert(replaced_sa_index, upper_sa_value);
                    assert((replaced_sa_index + 1) <= (uint64_t)this->sampled_first_sa.size());

                    this->sampled_first_sa.insert(replaced_sa_index + 1, lower_sa_value);
                    */
                }
                else if (type == RunRemovalType::RunRemovalWithoutMerge || type == RunRemovalType::RunRemovalDummy)
                {
                    this->sampled_last_sa.remove(replaced_sa_index);
                    this->sampled_first_sa.remove(replaced_sa_index);
                }
                else
                {
                    this->sampled_first_sa.remove(replaced_sa_index + 1);
                    this->sampled_first_sa.remove(replaced_sa_index);
                    this->sampled_last_sa.remove(replaced_sa_index);
                    this->sampled_last_sa.remove(replaced_sa_index - 1);
                }
            }

            void merge_runs(SampledSAIndex replaced_sa_index)
            {
                this->sampled_last_sa.remove(replaced_sa_index);
                this->sampled_first_sa.remove(replaced_sa_index + 1);
            }
            uint64_t size_in_bytes() const
            {
                return this->sampled_last_sa.size_in_bytes() + sampled_first_sa.size_in_bytes();
            }

            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->sampled_last_sa.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->sampled_first_sa.get_memory_usage_info(message_paragraph + 1);

                uint64_t total_bytes = this->size_in_bytes();
                uint64_t run_count = this->sampled_first_sa.size();
                uint64_t byte_per_run = total_bytes / run_count;

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=Dynamic Phi: " + std::to_string(total_bytes) + " bytes (" + std::to_string(byte_per_run) + " bytes per run in BWT) =");
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + " The number of runs: " + std::to_string(run_count));
                for (std::string &s : log1)
                {
                    r.push_back(s);
                }
                for (std::string &s : log2)
                {
                    r.push_back(s);
                }
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
                return r;
            }

            void print_memory_usage(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {

                std::vector<std::string> log = this->get_memory_usage_info(message_paragraph);
                for (std::string &s : log)
                {
                    std::cout << s << std::endl;
                }
            }

            //@}
        };
    }
}

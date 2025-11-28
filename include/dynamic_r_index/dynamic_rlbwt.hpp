#pragma once
#include "b_tree_plus_alpha/include/all.hpp"
#include "../dynamic_fm_index/time_debug.hpp"
#include "../dynamic_fm_index/tools.hpp"
#include "../dynamic_fm_index/c_array.hpp"
#include <functional>
namespace stool
{
    namespace dynamic_r_index
    {

        /**
         * @brief A dynamic data structure storing the RLBWT (c_{0}, ℓ_{0}), ..., (c_{r-1}, ℓ_{r-1}) of a string T[0..n-1]. [in progress].
         * @details This data structure consists of the following components:
         * @li A dynamic wavelet tree for sequence c_{0}, ..., c_{r-1}. Here, c_{i} is the character of the i-th run in the RLBWT.
         * @li A prefix-sum data structure for sequence ℓ_{0}, ..., ℓ_{r-1}. Here, ℓ_{i} is the length of the i-th run in the RLBWT.
         * @li A prefix-sum data structure for sequence u_{0}, ..., u_{σ'-1}.
         * Here, Σ' = {c'_{0}, c'_{2}, ..., c'_{σ'-1}} is a set of characters that appear in the string \p T[0..n-1],
         * and u_{i} is the number of runs of c'_{i} in the RLBWT.
         * @li A prefix-sum data structure for sequence ℓ_{Π[0]}, ..., ℓ_{Π[r-1]}.
         * Here, (c_{Π[0]}, ℓ_{Π[0]}), ..., (c_{Π[r-1]}, ℓ_{Π[r-1]}) is the repetitions obtained by appyling LF-mapping to the RLBWT.
         * These repetitions are called f-runs, and let F be the concatenation of the f-runs.
         * @li The C and D arrays for the string \p T
         * \ingroup DynamicRIndexes
         * \ingroup MainDataStructures
         */
        class DynamicRLBWT
        {
        private:
            stool::bptree::DynamicWaveletTree head_chars_of_RLBWT;               // A dynamic wavelet tree for sequence c_{1}, ..., c_{r}
            stool::bptree::SimpleDynamicPrefixSum run_length_vector;             // A prefix-sum data structure for sequence ℓ_{1}, ..., ℓ_{r}
            stool::bptree::SimpleDynamicPrefixSum c_run_counters;                // A prefix-sum data structure for sequence u_{0}, ..., u_{σ'-1}.
            stool::bptree::SimpleDynamicPrefixSum run_length_vector_sorted_by_F; // A prefix-sum data structure for sequence ℓ_{Π[0]}, ..., ℓ_{Π[r-1]}
            stool::dynamic_r_index::CArray cArray;                               // The C and D arrays for the string \p T
            int64_t _text_size = 0;

            // std::vector<uint8_t> effective_alphabet;

        public:
            using FRunPosition = RunPosition;

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Constructors and Destructor
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Default constructor.
             */
            DynamicRLBWT()
            {
            }

            /**
             * @brief Default move constructor.
             */
            DynamicRLBWT(DynamicRLBWT &&) noexcept = default;
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Operators
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Deleted copy assignment operator.
             */
            DynamicRLBWT &operator=(const DynamicRLBWT &) = delete;
            /**
             * @brief Default move assignment operator.
             */
            DynamicRLBWT &operator=(DynamicRLBWT &&) noexcept = default;
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Return the reference to the C array in this instance
             */
            const stool::dynamic_r_index::CArray &get_c_array() const
            {
                return this->cArray;
            }

            /**
             * @brief Return the size \p n of T[0..n-1]
             */
            uint64_t size() const
            {
                return this->text_size();
            }
            /**
             * @brief The alias for size()
             */
            uint64_t text_size() const
            {
                return this->_text_size;
            }
            /**
             * @brief Return the number of runs \p r in the RLBWT
             */
            uint64_t run_count() const
            {
                return this->head_chars_of_RLBWT.size();
            }
            /**
             * @brief Return the smallest character in this RLBWT
             */
            int64_t get_end_marker() const
            {
                return this->head_chars_of_RLBWT.get_smallest_character_in_alphabet();
            }
            /**
             * @brief Return the alphabet size \p σ of the RLBWT
             */
            uint64_t get_alphabet_size() const
            {
                return this->head_chars_of_RLBWT.get_alphabet_size();
            }

            /**
             * @brief Returns the total memory usage in bytes
             */
            uint64_t size_in_bytes() const
            {
                return this->run_length_vector.size_in_bytes() + this->c_run_counters.size_in_bytes() + this->run_length_vector_sorted_by_F.size_in_bytes() + this->head_chars_of_RLBWT.size_in_bytes() + this->cArray.size_in_bytes() + sizeof(this->_text_size);
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Queries for accessing to runs and F-runs
            ////////////////////////////////////////////////////////////////////////////////

            /**
             * @brief Return the lexicographic order of a given character c in the alphabet Σ of the RLBWT if it exists. Otherwise, return -1.
             */
            int64_t get_lexicographic_order(uint8_t c) const
            {
                return this->head_chars_of_RLBWT.get_lexicographic_order(c);
            }

            /**
             * @brief Return the starting position of the i-th run in \p T[0..n-1]
             * @note O(log r) time
             */
            int64_t get_starting_position(int64_t i) const
            {
                if (i == 0)
                {
                    return 0;
                }
                else
                {
                    assert(i - 1 < (int64_t)this->run_length_vector.size());
                    return this->run_length_vector.psum(i - 1);
                }
            }

            /**
             * @brief Return the length of the i-th run
             * @note O(log r) time
             */
            uint64_t get_run_length(uint64_t i) const
            {
                return this->run_length_vector.at(i);
            }

            /**
             * @brief Return the character of the i-th run
             * @note O(log r) time
             */
            uint8_t get_char(uint64_t i) const
            {
                return this->head_chars_of_RLBWT.at(i);
            }

            /**
             * @brief Return the position of the end marker in \p T[0..n-1]
             * @note O(log r) time
             */
            int64_t get_end_marker_pos() const
            {
                int64_t pos = this->head_chars_of_RLBWT.select(0, this->get_end_marker());
                assert(pos >= 0 && pos < (int64_t)this->head_chars_of_RLBWT.size());
                return this->get_starting_position(pos);
            }

            /**
             * @brief Return the index of the run containing the position \p pos in \p T[0..n-1]
             * @note O(log r) time
             */
            int64_t search_run_index(int64_t pos) const
            {
                return this->run_length_vector.search(pos + 1);
            }

            /**
             * @brief Return the characters occuring in this RLBWT.
             * @note O(σ) time
             */
            std::vector<uint8_t> get_effective_alphabet() const
            {
                return this->cArray.get_effective_alphabet();
            }

            /**
             * @brief Return the alphabet Σ of the RLBWT as a vector.
             */
            std::vector<uint8_t> to_alphabet_vector() const
            {
                return this->head_chars_of_RLBWT.to_alphabet_vector();
            }

            /**
             * @brief Return the number of runs of the character \p c in the RLBWT
             * @note O(log σ) time
             */
            uint64_t c_run_count(uint8_t c) const
            {
                int64_t id = this->cArray.get_c_id(c);
                if (id != -1)
                {
                    return this->c_run_counters.at(id);
                }
                else
                {
                    return 0;
                }
            }

            /**
             * @brief Return (i) the index of the f-run \p F[p..q] containing the position \p pos in the string \p F and (ii) (pos - p)
             * @note O(log r) time
             */
            FRunPosition to_frun_position(int64_t pos) const
            {
                uint64_t idx = this->run_length_vector_sorted_by_F.search(pos + 1);
                if (idx == 0)
                {
                    return FRunPosition(idx, pos);
                }
                else
                {
                    uint64_t p = this->run_length_vector_sorted_by_F.psum(idx - 1);
                    return RunPosition(idx, pos - p);
                }
            }

            /**
             * @brief Return (i) the index of the run containing the position \p pos in \p T[0..n-1] and (ii) (pos - p)
             * @note O(log r) time
             * @param special_flag If true and pos = n, then this function returns a pair of r and 0
             */
            RunPosition to_run_position(int64_t pos, bool special_flag = false) const
            {
                if (special_flag && pos == this->_text_size)
                {
                    return RunPosition(this->run_count(), 0);
                }
                else
                {
                    int64_t idx = this->run_length_vector.search(pos + 1);

                    assert(idx != -1);
                    if (idx == 0)
                    {
                        return RunPosition(idx, pos);
                    }
                    else
                    {
                        int64_t p = this->run_length_vector.psum(idx - 1);
                        return RunPosition(idx, pos - p);
                    }
                }

            }

            /**
             * @brief Return the position in \p T[0..n-1] corresponding to a given position in a run
             * @note O(log r) time
             */
            uint64_t to_position(RunPosition rp) const
            {
                uint64_t p = this->get_starting_position(rp.run_index);
                return p + rp.position_in_run;
            }

            /**
             * @brief Return the position in \p T[0..n-1] corresponding to a given position \p q in the p-th run
             * @note O(log r) time
             */
            int64_t to_position(int64_t run_index_p, int64_t pos_in_run_q) const
            {
                if (run_index_p == 0)
                {
                    return pos_in_run_q;
                }
                else
                {
                    return this->run_length_vector.psum(run_index_p - 1) + pos_in_run_q;
                }
            }
         

            /**
             * @brief Return the character of the i-th run
             * @note O(log r) time
             */
            uint8_t access_character_by_run_index(int64_t i) const
            {
                return this->head_chars_of_RLBWT.at(i);
            }

            /**
             * @brief Return L[i] of the BWT \p L[0..n-1]
             * @note O(log r) time
             */
            uint8_t access(int64_t i) const
            {
                int64_t idx = this->run_length_vector.search(i + 1);
                uint8_t c = this->head_chars_of_RLBWT.at(idx);

                // uint8_t c = DynRankSWrapper::access(*this->bwt_top_chars, idx);
                // assert(this->head_chars_of_RLBWT.at(idx) == c);

                // assert(c == this->_debug_bwt[i]);
                return c;
            }
            /**
             * @brief Return the index of the f-run corresponding to the i-th run
             * @note O(log r) time
             */
            int64_t get_f_index(RunIndex i) const
            {
                uint8_t c = this->head_chars_of_RLBWT.at(i);
                int64_t rank = this->head_chars_of_RLBWT.one_based_rank(i + 1, c);

                // uint8_t c = DynRankSWrapper::access(*this->bwt_top_chars, i);
                // assert(this->head_chars_of_RLBWT.at(i) == c);
                // int64_t rank = DynRankSWrapper::rank(*this->bwt_top_chars, c, i);
                // assert(this->head_chars_of_RLBWT.one_based_rank(i+1, c) == rank);

                int64_t id = this->cArray.get_c_id(c);
                int64_t psum = id > 0 ? this->c_run_counters.psum(id - 1) : 0;
                return psum + rank - 1;
            }

            /**
             * @brief Return the index of the first f-run of the character \p c'_{i}
             * @note O(log σ) time
             */
            int64_t get_f_index_by_c_id(uint8_t i) const
            {
                if (i == 0)
                {
                    return 0;
                }
                else
                {
                    return this->c_run_counters.psum(i - 1);
                }
            }

            /**
             * @brief Return the index of the first f-run of the character \p c if the BWT contains c, otherwise return 0
             * @note O(log σ) time
             */
            int64_t get_f_index_by_char(uint8_t c) const
            {
                int64_t id = this->cArray.predecessor_on_effective_alphabet(c);
                bool b = this->cArray.occurs_in_effective_alphabet(c);
                if (id != -1)
                {
                    if (b)
                    {
                        return this->get_f_index_by_c_id(id);
                    }
                    else
                    {
                        return this->c_run_counters.psum(id);
                    }
                }
                else
                {
                    return 0;
                }
            }

            /**
             * @brief Return the starting position of the i-th f-run in the string \p F
             * @note O(log r) time
             */
            int64_t get_starting_position_on_F(RunIndex i) const
            {
                return i > 0 ? this->run_length_vector_sorted_by_F.psum(i - 1) : 0;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Rank, select, LF, and inverse LF queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{


            /**
             * @brief Compute special rank value for a position within a run
             * @param run_index The index of the run
             * @param pos_in_run The position within the run
             * @return The special rank value
             */
            int64_t special_rank(int64_t run_index, int64_t pos_in_run) const
            {
                int64_t f_index = this->get_f_index(run_index);
                uint8_t c = this->get_char(run_index);

#if DEBUG
                int64_t id = this->cArray.get_c_id(c);
                assert(id != -1);
#endif

                return this->get_starting_position_on_F(f_index) - this->cArray.at(c) + pos_in_run + 1;
            }


            /**
             * @brief Count the number of occurrences of character c up to position i
             * @param c The character to count
             * @param i The run position up to which to count
             * @return The number of occurrences of c in BWT[0..i]
             */
            int64_t rank(uint8_t c, RunPosition i) const
            {
                uint8_t c1 = this->head_chars_of_RLBWT.at(i.run_index);
                // uint8_t c1 = DynRankSWrapper::access(*this->bwt_top_chars, i.run_index);
                // assert(this->head_chars_of_RLBWT.at(i.run_index) == c1);
                if (c == c1)
                {
                    return this->special_rank(i.run_index, i.position_in_run);
                }
                else
                {
                    int64_t id = this->cArray.get_c_id(c);
                    if (id != -1)
                    {
                        uint64_t rank = this->head_chars_of_RLBWT.one_based_rank(i.run_index + 1, c);
                        // uint64_t rank = DynRankSWrapper::rank(*this->bwt_top_chars, c, i.run_index);
                        // assert(rank == this->head_chars_of_RLBWT.one_based_rank(i.run_index+1, c));
                        if (rank > 0)
                        {
                            int64_t f_index = id > 0 ? this->c_run_counters.psum(id - 1) : 0;
                            int64_t beg = f_index > 0 ? this->run_length_vector_sorted_by_F.psum(f_index - 1) : 0;
                            int64_t end = this->run_length_vector_sorted_by_F.psum(f_index - 1 + rank);

                            return end - beg;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        return 0;
                    }
                }
            }

            /**
             * @brief Count the number of occurrences of character c up to position i
             * @param c The character to count
             * @param i The absolute position in the BWT
             * @return The number of occurrences of c in BWT[0..i]
             */
            int64_t rank(uint8_t c, int64_t i) const
            {
                return this->rank(c, this->to_run_position(i));
            }

            /**
             * @brief Find the position of the nth occurrence of character c
             * @param c The character to search for
             * @param nth The occurrence number (0-indexed)
             * @return The position of the nth occurrence, or -1 if not found
             */
            int64_t select(uint8_t c, int64_t nth) const
            {
                if (nth == 0)
                {
                    return -1;
                }
                else
                {
                    nth--;
                    int64_t c_rank = this->cArray.get_c_id(c);
                    if (c_rank == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        uint64_t f_idx = this->get_f_index_by_char(c);
                        uint64_t psum1 = f_idx > 0 ? this->run_length_vector_sorted_by_F.psum(f_idx - 1) : 0;
                        int64_t x = this->run_length_vector_sorted_by_F.search(psum1 + nth + 1);
                        uint64_t c_rank2 = x - f_idx;
                        uint64_t f_idx2 = f_idx + c_rank2;
                        uint64_t psum2 = f_idx2 > 0 ? this->run_length_vector_sorted_by_F.psum(f_idx2 - 1) : 0;

                        assert(c_rank2 < this->head_chars_of_RLBWT.count_c(c));
                        int64_t nth3 = this->head_chars_of_RLBWT.select(c_rank2, c);
                        assert(nth3 != -1);

                        assert(nth3 < (int64_t)this->run_length_vector.size());
                        uint64_t psum3 = nth3 > 0 ? this->run_length_vector.psum(nth3 - 1) : 0;
                        uint64_t c_rank3 = (psum1 + nth) - psum2;

                        return psum3 + (c_rank3);
                    }
                }
            }
            /**
             * @brief Count the number of runs starting with character c up to a given run index
             * @param c The character to count
             * @param run_index The run index up to which to count
             * @return The number of runs starting with c up to run_index
             */
            int64_t rank_on_first_characters_of_RLBWT(uint8_t c, int64_t run_index) const
            {
                return this->head_chars_of_RLBWT.one_based_rank(run_index + 1, c);
                // assert(DynRankSWrapper::rank(*this->bwt_top_chars, c, run_index) == this->head_chars_of_RLBWT.one_based_rank(run_index+1, c));
                // return DynRankSWrapper::rank(*this->bwt_top_chars, c, run_index);
            }

            /**
             * @brief Find the run index of the ith run starting with character c
             * @param ith The occurrence number (0-indexed)
             * @param c The character to search for
             * @return The run index of the ith run starting with c
             */
            int64_t select_on_first_characters_of_RLBWT(int64_t ith, uint8_t c) const
            {
                return this->head_chars_of_RLBWT.select(ith, c);
                // assert(DynRankSWrapper::select(*this->bwt_top_chars, c, ith) == this->head_chars_of_RLBWT.select(ith-1, c));
                // return DynRankSWrapper::select(*this->bwt_top_chars,c, ith);
            }

            /**
             * @brief Get the character ID of a run in F-sorted order
             * @param frun_index The F-run index
             * @return The character ID
             */
            uint8_t get_c_id_of_f_run(int64_t frun_index) const
            {
                return this->c_run_counters.search(frun_index + 1);
            }

            /**
             * @brief Compute inverse LF mapping for a position in F-sorted order
             * @param frun_index The F-run index
             * @param pos_in_frun The position within the F-run
             * @return The corresponding position in the BWT
             */
            int64_t inverse_LF(int64_t frun_index, int64_t pos_in_frun) const
            {
                uint8_t c_id = this->get_c_id_of_f_run(frun_index);
                uint8_t c = this->cArray.get_character_in_effective_alphabet(c_id);
                uint64_t c_rank = frun_index - this->get_f_index_by_c_id(c_id);

                uint64_t p = this->select_on_first_characters_of_RLBWT(c_rank, c);
                return this->to_position(p, pos_in_frun);
            }

            /**
             * @brief Compute inverse LF mapping for a BWT position
             * @param i The position in the BWT
             * @return The previous position in the BWT after inverse LF mapping
             */
            int64_t inverse_LF(int64_t i) const
            {
                FRunPosition fp = this->to_frun_position(i);
                return this->inverse_LF(fp.run_index, fp.position_in_run);
            }

            /**
             * @brief Compute LF mapping for a run position
             * @param run_index The run index
             * @param pos_in_run The position within the run
             * @return The next position in the BWT after LF mapping
             */
            int64_t LF(int64_t run_index, int64_t pos_in_run) const
            {
                int64_t f_index = this->get_f_index(run_index);
                return this->get_starting_position_on_F(f_index) + pos_in_run;
            }

            /**
             * @brief Compute LF mapping for a run position
             * @param pos The run position
             * @return The next position in the BWT after LF mapping
             */
            int64_t LF(RunPosition &pos) const
            {
                return this->LF(pos.run_index, pos.position_in_run);
            }

            /**
             * @brief Compute LF mapping for a BWT position
             * @param i The position in the BWT
             * @return The next position in the BWT after LF mapping
             */
            int64_t LF(int64_t i) const
            {
                assert(i >= 0);
                assert(i < (int64_t)this->text_size());
                RunPosition rp = this->to_run_position(i);
                return this->LF(rp.run_index, rp.position_in_run);
            }

            /**
             * @brief Compute LF mapping during insertion operation
             * @param i The current position
             * @param insertion_position_on_sa The SA position where insertion occurs
             * @param last_processed_character The last processed character
             * @param old_character The old character being replaced
             * @return The LF value adjusted for insertion
             */
            int64_t LF_for_insertion(int64_t i, uint64_t insertion_position_on_sa, uint8_t last_processed_character, uint8_t old_character) const
            {
                uint64_t lf = this->LF(i);
                bool b = this->check_special_LF(insertion_position_on_sa, i, last_processed_character, old_character);
                if (b)
                {
                    lf++;
                }
                return lf;
            }

            /**
             * @brief Compute LF mapping during deletion operation
             * @param i The current position
             * @param new_char The new character replacing the deleted one
             * @param replace_pos The position being replaced
             * @param current_processing_position The current processing position
             * @return The LF value adjusted for deletion
             */
            int64_t LF_for_deletion(uint64_t i, uint8_t new_char, uint64_t replace_pos, uint64_t current_processing_position) const
            {
                if (i == replace_pos)
                {
                    return LF_for_deletion(current_processing_position, new_char, replace_pos, current_processing_position);
                }
                else
                {
                    uint8_t c = this->access(i);
                    uint64_t p = this->LF(i);

                    if (c > new_char || (i > replace_pos && c == new_char))
                    {
                        if (p > 0)
                        {
                            p--;
                        }
                        else
                        {
                            p = this->text_size() - 2;
                        }
                    }
                    return p;
                }
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Convertion functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Construct the complete LF array
             * @return Vector containing LF values for all positions
             * @note This is mainly used for debugging and verification
             */
            std::vector<uint64_t> construct_LF_array() const
            {
                std::vector<uint64_t> r;
                r.resize(this->text_size(), UINT64_MAX);

                for (uint64_t i = 0; i < r.size(); i++)
                {
                    RunPosition rp = this->to_run_position(i);
                    uint64_t v = this->LF(rp.run_index, rp.position_in_run);
                    r[i] = v;
                }
                return r;
            }

            /**
             * @brief Get the complete BWT as a byte vector
             * @return The BWT represented as a vector of bytes
             */
            std::vector<uint8_t> get_bwt() const
            {
                std::vector<uint8_t> r;
                r.resize(this->_text_size, 0);
                int64_t x = 0;
                uint64_t run_count = this->run_count();
                for (int64_t i = 0; i < (int64_t)run_count; i++)
                {
                    uint8_t c = this->get_char(i);
                    uint64_t run_length = this->get_run_length(i);
                    for (uint64_t j = 0; j < run_length; j++)
                    {
                        r[x++] = c;
                    }
                }
                return r;
            }

            /**
             * @brief Get the BWT as a string
             * @param new_end_marker Optional replacement for end marker character (-1 to keep original)
             * @return The BWT represented as a string
             */
            std::string get_bwt_str(int64_t new_end_marker = -1) const
            {
                std::vector<uint8_t> r = this->get_bwt();
                std::string s;
                for (auto c : r)
                {
                    if (new_end_marker != -1 && c == this->get_end_marker())
                    {
                        s.push_back(new_end_marker);
                    }
                    else
                    {
                        s.push_back(c);
                    }
                }
                return s;
            }

            /**
             * @brief Reconstruct the original text from the RLBWT
             * @return The original text as a byte vector
             * @note Uses LF mapping to traverse backwards from the end marker
             */
            std::vector<uint8_t> get_text() const
            {
                std::vector<uint8_t> r;
                r.resize(this->text_size(), 0);
                r[r.size() - 1] = this->get_end_marker();
                int64_t current_pos_on_bwt = this->get_end_marker_pos();
                int64_t x = this->text_size() - 1;
                while (x > 0)
                {

                    current_pos_on_bwt = this->LF(current_pos_on_bwt);

                    r[--x] = this->access(current_pos_on_bwt);
                }
                return r;
            }

            /**
             * @brief Get the original text as a string
             * @return The original text represented as a string
             */
            std::string get_text_str() const
            {
                std::vector<uint8_t> r = this->get_text();
                std::string s;
                for (auto c : r)
                {
                    s.push_back(c);
                }
                return s;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Update operations
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Swap the contents with another DynamicRLBWT instance
             * @param item The DynamicRLBWT instance to swap with
             */
            void swap(DynamicRLBWT &item)
            {
                this->run_length_vector.swap(item.run_length_vector);
                this->c_run_counters.swap(item.c_run_counters);
                this->run_length_vector_sorted_by_F.swap(item.run_length_vector_sorted_by_F);
                this->head_chars_of_RLBWT.swap(item.head_chars_of_RLBWT);
                this->cArray.swap(item.cArray);
                std::swap(_text_size, item._text_size);
            }

            /**
             * @brief Set the alphabet and clear the structure
             * @param _alphabet The alphabet to use
             */
            void set_alphabet(const std::vector<uint8_t> &_alphabet)
            {
                // this->end_marker = *std::min_element(std::begin(_alphabet), std::end(_alphabet));;
                // this->end_marker = _end_marker;
                // this->initialize_rank_select_data_structure(_alphabet);
                this->head_chars_of_RLBWT.set_alphabet(_alphabet);
                this->clear();
            }

            /**
             * @brief Clear all data and initialize with a single end marker run
             */
            void clear()
            {
                this->head_chars_of_RLBWT.clear();
                this->run_length_vector.clear();
                this->run_length_vector_sorted_by_F.clear();
                this->c_run_counters.clear();
                this->cArray.clear();
                this->_text_size = 0;
                this->insert_new_run(0, this->get_end_marker(), 1);
            }

            /**
             * @brief Increment the length of a run
             * @param run_index The index of the run to increment
             * @param delta The amount to increment by
             */
            void increment_run(int64_t run_index, uint64_t delta)
            {
#ifdef TIME_DEBUG
                stool::increment_run_count++;

                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();
#endif

                uint64_t f_index = this->get_f_index(run_index);
                this->run_length_vector.increment(run_index, delta);
                this->run_length_vector_sorted_by_F.increment(f_index, delta);
                uint8_t c = this->get_char(run_index);
                this->cArray.increase(c, delta);
                this->_text_size += delta;

#ifdef TIME_DEBUG
                st2 = std::chrono::system_clock::now();
                uint64_t time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(st2 - st1).count();
                stool::increment_run_time += time1;
#endif
            }
            /**
             * @brief Insert a character at a given position in the BWT
             * @param pos The position to insert at
             * @param c The character to insert
             * @note This may split runs or merge adjacent runs if necessary
             */
            void insert(int64_t pos, uint8_t c)
            {
                auto ridx = this->to_run_position(pos);
                uint8_t c2 = this->access_character_by_run_index(ridx.run_index);
                uint64_t len = this->run_length_vector.at(ridx.run_index);
                if (c == c2)
                {
                    this->increment_run(ridx.run_index, 1);
                }
                else
                {
                    if (ridx.position_in_run == 0)
                    {
                        if (ridx.run_index > 0 && this->access_character_by_run_index(ridx.run_index - 1) == c)
                        {
                            this->increment_run(ridx.run_index - 1, 1);
                        }
                        else
                        {
                            this->insert_new_run(ridx.run_index, c, 1);
                        }
                    }
                    else
                    {
                        uint64_t pref = ridx.position_in_run;
                        uint64_t suf = len - pref;
                        this->decrement_run(ridx.run_index, suf);
                        this->insert_new_run(ridx.run_index + 1, c2, suf);
                        this->insert_new_run(ridx.run_index + 1, c, 1);
                    }
                }
            }

            /**
             * @brief Remove a character at a given position in the BWT
             * @param pos The position to remove from
             * @note This may merge adjacent runs if they have the same character
             */
            void remove(int64_t pos)
            {
                auto ridx = this->to_run_position(pos);
                uint64_t len = this->run_length_vector.at(ridx.run_index);
                if (len > 1)
                {
                    this->decrement_run(ridx.run_index, 1);
                }
                else
                {
                    uint64_t len = this->run_count();
                    this->remove_BWT_run(ridx.run_index);
                    if (ridx.run_index >= 1 && ridx.run_index + 1 < len)
                    {
                        uint8_t c1 = this->access_character_by_run_index(ridx.run_index - 1);
                        uint8_t c2 = this->access_character_by_run_index(ridx.run_index);
                        if (c1 == c2)
                        {
                            // uint64_t pref = this-run_length_vector[ridx.run_index-1];
                            uint64_t suf = this->run_length_vector.at(ridx.run_index);
                            this->remove_BWT_run(ridx.run_index);
                            this->increment_run(ridx.run_index - 1, suf);
                        }
                    }
                }
            }

            /**
             * @brief Insert a new run at a given index
             * @param run_index The index where to insert the new run
             * @param c The character of the new run
             * @param run_length The length of the new run
             */
            void insert_new_run(int64_t run_index, uint8_t c, int64_t run_length)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point __st1, __st2;
                __st1 = std::chrono::system_clock::now();

#endif

                this->head_chars_of_RLBWT.insert(run_index, c);
                uint64_t rank = this->head_chars_of_RLBWT.one_based_rank(run_index + 1, c);
                int64_t new_f_index = this->get_f_index_by_char(c) + rank - 1;

                bool c_b = this->cArray.occurs_in_effective_alphabet(c);
                this->cArray.increase(c, run_length);
                int64_t c_id = this->cArray.get_c_id(c);

                assert(c_id != -1);
                assert(c_id <= (int64_t)this->c_run_counters.size());
                if (!c_b)
                {
                    this->c_run_counters.insert(c_id, 1);
                }
                else
                {
                    this->c_run_counters.increment(c_id, 1);
                }

                this->run_length_vector.insert(run_index, run_length);
                this->run_length_vector_sorted_by_F.insert(new_f_index, run_length);
                this->_text_size += run_length;

#ifdef TIME_DEBUG
                __st2 = std::chrono::system_clock::now();
                uint64_t __ns_time = std::chrono::duration_cast<std::chrono::nanoseconds>(__st2 - __st1).count();
                __insert_new_run_count++;
                __insert_new_run_time += __ns_time;
#endif
            }

            /**
             * @brief Remove a BWT run completely
             * @param run_index The index of the run to remove
             * @note This updates all related data structures including C array and run counters
             */
            void remove_BWT_run(RunIndex run_index)
            {
                int64_t run_length = this->get_run_length(run_index);
                uint8_t c = this->head_chars_of_RLBWT.at(run_index);
                // uint8_t c = DynRankSWrapper::access(*this->bwt_top_chars, run_index);
                // assert(c == this->head_chars_of_RLBWT.at(run_index));
                uint64_t f_index = this->get_f_index(run_index);
                // DynRankSWrapper::remove(*this->bwt_top_chars, run_index);
                this->head_chars_of_RLBWT.remove(run_index);

                this->run_length_vector.remove(run_index);
                this->run_length_vector_sorted_by_F.remove(f_index);
                // this->update_C_info_for_deletion(c);
                this->_text_size -= run_length;

                int64_t id = this->cArray.get_c_id(c);
                assert(id != -1);
                int64_t counter = this->c_run_counters.at(id);
                assert(counter > 0);
                if (counter > 1)
                {
                    this->c_run_counters.decrement(id, 1);
                }
                else
                {
                    this->c_run_counters.remove(id);
                }
                this->cArray.decrease(c, run_length);
            }

            /**
             * @brief Decrement the length of a run
             * @param run_index The index of the run to decrement
             * @param delta The amount to decrement by
             * @note If the run length becomes zero, the run is removed
             */
            void decrement_run(int64_t run_index, uint64_t delta)
            {
                int64_t clen = this->run_length_vector.at(run_index);
                assert(clen > 0 && (int64_t)delta <= clen);
                int64_t new_clen = clen - (int64_t)delta;

                if (new_clen > 0)
                {
                    uint64_t f_index = this->get_f_index(run_index);
                    this->run_length_vector.decrement(run_index, delta);
                    this->run_length_vector_sorted_by_F.decrement(f_index, delta);
                    uint8_t c = this->get_char(run_index);
                    this->cArray.decrease(c, delta);
                }
                else
                {
                    this->remove_BWT_run(run_index);
                }
                this->_text_size -= delta;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Print and verification functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Get memory usage information for all components
             * @param message_paragraph Message indentation level
             * @return Vector of strings containing memory usage details
             */
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->run_length_vector.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->c_run_counters.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log3 = this->run_length_vector_sorted_by_F.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log4 = this->head_chars_of_RLBWT.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log5 = this->cArray.get_memory_usage_info(message_paragraph + 1);

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicRLBWT: " + std::to_string(this->size_in_bytes()) + " bytes =");
                for (std::string &s : log1)
                {
                    r.push_back(s);
                }
                for (std::string &s : log2)
                {
                    r.push_back(s);
                }
                for (std::string &s : log3)
                {
                    r.push_back(s);
                }
                for (std::string &s : log4)
                {
                    r.push_back(s);
                }
                for (std::string &s : log5)
                {
                    r.push_back(s);
                }

                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "==");
                return r;
            }

            /**
             * @brief Print memory usage information to stdout
             * @param message_paragraph Message indentation level
             */
            void print_memory_usage(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log = this->get_memory_usage_info(message_paragraph);
                for (std::string &s : log)
                {
                    std::cout << s << std::endl;
                }
            }

            /**
             * @brief Print statistics about the data structure
             * @param message_paragraph Message indentation level
             */
            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicRLBWT):" << std::endl;
                this->run_length_vector.print_statistics(message_paragraph + 1);
                this->c_run_counters.print_statistics(message_paragraph + 1);
                this->run_length_vector_sorted_by_F.print_statistics(message_paragraph + 1);
                this->head_chars_of_RLBWT.print_statistics(message_paragraph + 1);
                this->cArray.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /**
             * @brief Verify the correctness of the RLBWT structure
             * @param mode Verification mode (0: full verification including LF array)
             * @note Throws std::logic_error if verification fails
             */
            void verify(int mode = 0) const
            {
                this->verify1();
                if (mode == 0)
                {
                    std::vector<uint64_t> LF_array = this->construct_LF_array();
                    DynamicRLBWT::verify_permutation(LF_array, "LF");
                }
            }

            /**
             * @brief Print the internal structure of the RLBWT
             * @param message_paragraph Message indentation level
             */
            void print(int message_paragraph = 0) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "=========== Data Structure for RLBWT ============" << std::endl;
                // std::cout << "BWT: " << this->get_bwt_str() << std::endl;
                std::vector<uint8_t> charVec;
                std::vector<int64_t> rVec;
                std::vector<int64_t> fVec;
                std::vector<int64_t> stVec;
                std::vector<uint64_t> cVec;
                std::vector<uint64_t> intvVec;

                for (int64_t i = 0; i < (int64_t)this->run_count(); i++)
                {
                    // assert(DynRankSWrapper::access(*this->bwt_top_chars, i) == this->head_chars_of_RLBWT.at(i));
                    // charVec.push_back(DynRankSWrapper::access(*this->bwt_top_chars, i));
                    charVec.push_back(this->head_chars_of_RLBWT.at(i));
                    rVec.push_back(this->run_length_vector.at(i));
                    fVec.push_back(this->run_length_vector_sorted_by_F.at(i));
                    stVec.push_back(this->get_starting_position(i));
                }

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << std::flush;
                stool::DebugPrinter::print_characters(charVec, "Chr");
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << std::flush;
                stool::DebugPrinter::print_integers(rVec, "RLV");
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << std::flush;
                stool::DebugPrinter::print_integers(stVec, "ST ");
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << std::flush;
                stool::DebugPrinter::print_integers(fVec, "FLV");
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "=================================================" << std::endl;
            }
            /**
             * @brief Print the content (alias for print)
             * @param message_paragraph Message indentation level
             */
            void print_content(int message_paragraph = 0) const
            {
                this->print(message_paragraph);
            }

            /**
             * @brief Verify that no two consecutive runs have the same character
             * @return true if verification passes
             * @throws std::logic_error if consecutive runs have the same character
             */
            bool verify1() const
            {
                for (uint64_t i = 1; i < this->run_count(); i++)
                {
                    uint8_t c1 = this->get_char(i - 1);
                    uint8_t c2 = this->get_char(i);
                    if (c1 == c2)
                    {
                        std::cout << std::endl;
                        throw std::logic_error("Error: DynamicRLBWT::verify(), the same concecutive characters");
                    }
                }
                return true;
            }

            /**
             * @brief Check if LF mapping needs special adjustment during insertion
             * @param positionToReplace The position being replaced
             * @param currentPosition The current processing position
             * @param new_char The new character
             * @param old_char The old character
             * @return true if LF value needs to be incremented
             */
            bool check_special_LF(SAIndex positionToReplace, SAIndex currentPosition, uint8_t new_char, uint8_t old_char) const
            {
                // std::cout << "check_special_LF: " << positionToReplace << "/" << currentPosition << "/" << (char)new_char << "/" << (char)old_char << std::endl;
                if (old_char < new_char)
                {
                    return true;
                }
                else if (positionToReplace <= currentPosition && new_char == old_char)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            /**
             * @brief Get a debug representation of the BWT
             * @return String representation of the BWT with run boundaries marked
             * @note Internal debug function, not part of public API
             */
            std::string __get_debug_bwt() const
            {
                std::string r;
                for (uint64_t i = 0; i < this->run_count(); i++)
                {
                    uint64_t len = this->run_length_vector.at(i);
                    uint8_t c = this->head_chars_of_RLBWT.at(i);
                    uint8_t c_next = i + 1 < this->run_count() ? this->head_chars_of_RLBWT.at(i + 1) : UINT8_MAX;

                    for (uint64_t j = 0; j < len; j++)
                    {
                        r.push_back(c);
                        if (j + 1 < len)
                        {
                            r.push_back(' ');
                        }
                        else
                        {
                            if (c == c_next)
                            {
                                r.push_back('|');
                            }
                            else
                            {
                                r.push_back(' ');
                            }
                        }
                    }
                }
                return r;
            }

            /**
             * @brief Compute a hash value for the RLBWT structure
             * @param message_paragraph Message indentation level
             * @return Hash value computed from run characters and lengths
             * @note Used for verification and debugging
             */
            uint64_t compute_RLBWT_hash(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Computing the hash of the RLBWT..." << std::endl;
                }

                std::string str = this->head_chars_of_RLBWT.to_string();
                std::hash<std::string> hasher;
                uint64_t hash1 = hasher(str);

                uint64_t i = 0;
                uint64_t hash2 = 0;

                for (uint64_t s : this->run_length_vector)
                {
                    hash2 ^= (i ^ s);
                    i++;
                }
                uint64_t r = hash1 ^ hash2;

                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[DONE]" << std::endl;
                }

                return r;
            }

            bool verify_inserted_string(const std::vector<uint8_t> &inserted_string) const
            {

                for (uint8_t c : inserted_string)
                {
                    int64_t lexicographic_order = this->get_lexicographic_order(c);
                    if (lexicographic_order == -1)
                    {
                        std::string c_str;
                        c_str.push_back(c);
                        stool::DebugPrinter::print_integers_with_characters(this->to_alphabet_vector(), "Alphabet");
                        std::string error_message = "The character " + c_str + " (" + std::to_string(c) + ") is not contained in the alphabet of the RLBWT.";
                        throw std::runtime_error(error_message);
                    }
                }
                return true;
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Write the BWT to a binary file
             * @param file_path Path to the output file
             * @throws std::runtime_error if file cannot be opened
             * @note Uses buffered I/O for efficiency
             */
            void write_BWT(std::string file_path) const
            {
                std::ofstream ofs(file_path, std::ios::binary);
                if (!ofs)
                {
                    throw std::runtime_error("File cannot be opened");
                }
                uint64_t buffer_size = 8096;
                std::vector<char> buffer;
                buffer.reserve(buffer_size);
                for (uint64_t i = 0; i < this->run_count(); i++)
                {
                    uint8_t c = this->get_char(i);
                    uint64_t run_length = this->get_run_length(i);
                    for (uint64_t x = 0; x < run_length; x++)
                    {

                        if (buffer.size() >= buffer_size)
                        {
                            // ofs.rdbuf()->pubsetbuf(buffer.data(), buffer.size());
                            ofs.write(reinterpret_cast<const char *>(&buffer[0]), buffer.size() * sizeof(uint8_t));

                            buffer.clear();
                        }
                        buffer.push_back(c);
                    }
                }
                if (buffer.size() > 0)
                {
                    ofs.write(reinterpret_cast<const char *>(&buffer[0]), buffer.size() * sizeof(uint8_t));
                    buffer.clear();
                }
                ofs.close();
            }

            /**
             * @brief Build an RLBWT containing only the end marker
             * @param alphabet The alphabet to use
             * @return A DynamicRLBWT instance initialized with the end marker
             */
            static DynamicRLBWT build_RLBWT_of_end_marker(const std::vector<uint8_t> &alphabet)
            {
                DynamicRLBWT r;
                r.set_alphabet(alphabet);
                // r.insert_new_run(0, r.get_end_marker(), 1);
                return r;
            }

            /**
             * @brief Build DynamicRLBWT from run-length encoded BWT data
             * @param _rlbwt_chars Vector of characters for each run
             * @param _rlbwt_runs Vector of run lengths
             * @param alphabet The alphabet to use
             * @param message_paragraph Message indentation level for progress output
             * @return A new DynamicRLBWT instance built from the RLBWT data
             */
            static DynamicRLBWT build_from_RLBWT(const std::vector<uint8_t> &_rlbwt_chars, const std::vector<uint64_t> &_rlbwt_runs, const std::vector<uint8_t> &alphabet, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0 && _rlbwt_chars.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic RLBWT from RLBWT..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                stool::bptree::DynamicWaveletTree tmp_head_chars_of_RLBWT = stool::bptree::DynamicWaveletTree::build(_rlbwt_chars, alphabet);
                stool::bptree::SimpleDynamicPrefixSum tmp_run_length_vector = stool::bptree::SimpleDynamicPrefixSum::build(_rlbwt_runs);

                std::vector<uint64_t> c_counters;
                std::vector<uint64_t> c_run_counters;
                std::vector<uint64_t> c_run_counters2;

                c_counters.resize(256, 0);
                c_run_counters.resize(256, 0);

                for (int64_t i = 0; i < (int64_t)_rlbwt_chars.size(); i++)
                {
                    uint8_t c = _rlbwt_chars[i];
                    int64_t run = _rlbwt_runs[i];
                    c_counters[c] += run;
                    c_run_counters[c]++;
                }
                for (uint64_t v : c_run_counters)
                {
                    if (v != 0)
                    {
                        c_run_counters2.push_back(v);
                    }
                }

                stool::dynamic_r_index::CArray tmp_cArray = stool::dynamic_r_index::CArray::build(c_counters);
                stool::bptree::SimpleDynamicPrefixSum tmp_c_run_counters = stool::bptree::SimpleDynamicPrefixSum::build(c_run_counters2);

                std::vector<uint64_t> indexes;
                indexes.resize(_rlbwt_chars.size(), UINT64_MAX);
                for (uint64_t i = 0; i < _rlbwt_chars.size(); i++)
                {
                    indexes[i] = i;
                }
                std::sort(indexes.begin(), indexes.end(), [&](const int64_t &lhs, const int64_t &rhs)
                          {
                        if(_rlbwt_chars[lhs] != _rlbwt_chars[rhs]){
                            return _rlbwt_chars[lhs] < _rlbwt_chars[rhs];
                        }else{
                            return lhs < rhs;
                        } });
                stool::bptree::SimpleDynamicPrefixSum tmp_run_length_vector_sorted_by_F;
                for (auto i : indexes)
                {
                    tmp_run_length_vector_sorted_by_F.push_back(_rlbwt_runs[i]);
                }
                DynamicRLBWT r;
                r.head_chars_of_RLBWT.swap(tmp_head_chars_of_RLBWT);
                r.run_length_vector.swap(tmp_run_length_vector);
                r.run_length_vector_sorted_by_F.swap(tmp_run_length_vector_sorted_by_F);
                r.c_run_counters.swap(tmp_c_run_counters);
                r.cArray.swap(tmp_cArray);
                r._text_size = r.run_length_vector.psum();

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && _rlbwt_chars.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)r._text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            /**
             * @brief Build DynamicRLBWT from a BWT vector
             * @param _bwt The BWT as a vector of bytes
             * @param alphabet The alphabet to use
             * @param message_paragraph Message indentation level for progress output
             * @return A new DynamicRLBWT instance built from the BWT
             * @note First converts BWT to RLBWT format, then builds the dynamic structure
             */
            static DynamicRLBWT build_from_BWT(const std::vector<uint8_t> &_bwt, const std::vector<uint8_t> &alphabet, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                std::vector<uint8_t> rlbwt_chars;
                std::vector<uint64_t> rlbwt_runs;
                stool::RLEIO::build_RLBWT_from_BWT(_bwt, rlbwt_chars, rlbwt_runs, message_paragraph);

                return DynamicRLBWT::build_from_RLBWT(rlbwt_chars, rlbwt_runs, alphabet, message_paragraph);

                // this->build(rlbwt_chars, rlbwt_runs, alphabet, message_paragraph);
            }

            /**
             * @brief Save DynamicRLBWT to a binary file
             * @param item The DynamicRLBWT instance to save
             * @param os Output file stream (must be opened in binary mode)
             */
            static void store_to_file(DynamicRLBWT &item, std::ofstream &os)
            {
                stool::bptree::SimpleDynamicPrefixSum::store_to_file(item.run_length_vector, os);
                stool::bptree::SimpleDynamicPrefixSum::store_to_file(item.c_run_counters, os);
                stool::bptree::SimpleDynamicPrefixSum::store_to_file(item.run_length_vector_sorted_by_F, os);
                stool::bptree::DynamicWaveletTree::store_to_file(item.head_chars_of_RLBWT, os);
                stool::dynamic_r_index::CArray::store_to_file(item.cArray, os);
            }

            /**
             * @brief Load DynamicRLBWT from a binary file
             * @param ifs Input file stream (must be opened in binary mode)
             * @return A new DynamicRLBWT instance loaded from the file
             */
            static DynamicRLBWT load_from_file(std::ifstream &ifs)
            {
                stool::bptree::SimpleDynamicPrefixSum tmp_run_length_vector = stool::bptree::SimpleDynamicPrefixSum::load_from_file(ifs);
                stool::bptree::SimpleDynamicPrefixSum tmp_c_run_counters = stool::bptree::SimpleDynamicPrefixSum::load_from_file(ifs);
                stool::bptree::SimpleDynamicPrefixSum tmp_run_length_vector_sorted_by_F = stool::bptree::SimpleDynamicPrefixSum::load_from_file(ifs);
                stool::bptree::DynamicWaveletTree tmp_head_chars_of_RLBWT = stool::bptree::DynamicWaveletTree::load_from_file(ifs);
                stool::dynamic_r_index::CArray tmp_cArray = stool::dynamic_r_index::CArray::load_from_file(ifs);

                DynamicRLBWT r;
                r.head_chars_of_RLBWT.swap(tmp_head_chars_of_RLBWT);
                r.run_length_vector.swap(tmp_run_length_vector);
                r.run_length_vector_sorted_by_F.swap(tmp_run_length_vector_sorted_by_F);
                r.c_run_counters.swap(tmp_c_run_counters);
                r.cArray.swap(tmp_cArray);
                r._text_size = r.run_length_vector.psum();

                return r;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Other static functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Verify that a vector represents a valid permutation
             * @param perm The vector to verify
             * @param name Name of the permutation (for error messages)
             * @return true if the vector is a valid permutation, false otherwise
             * @note A valid permutation contains each value from 0 to n-1 exactly once
             */
            static bool verify_permutation(const std::vector<uint64_t> &perm, std::string name)
            {
                std::vector<bool> checker;
                checker.resize(perm.size(), false);

                for (uint64_t v : perm)
                {
                    if (v >= perm.size())
                    {
                        throw std::logic_error(name + " array contains an invalid value!");
                    }

                    if (checker[v])
                    {
                        throw std::logic_error(name + " array contains the same values!");
                    }
                    checker[v] = true;
                }
                return true;
            }
            //@}
        };
    }
}

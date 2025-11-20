/**
 * @file dynamic_bwt.hpp
 * @brief A dynamic data structure for storing and manipulating BWT (Burrows-Wheeler Transform).
 */

#pragma once
#include "./time_debug.hpp"
#include "./fm_index_edit_history.hpp"
#include "b_tree_plus_alpha/include/all.hpp"
#include "./c_array.hpp"

// #include "./packed_spsi_wrapper.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        bool __view_flag = false;

        /**
         * @brief A dynamic data structure storing the BWT \p L[0..n-1] of a string \p T[0..n-1] over an alphabet \p Σ[0..σ-1]. [in progress]
         * @note $O(n log σ)$ bits of space
         * \ingroup DynamicFMIndexes
         * \ingroup MainDataStructures
         */
        class DynamicBWT
        {
            stool::dynamic_r_index::CArray cArray; /// C and D arrays
            stool::bptree::DynamicWaveletTree bwt; /// L

        public:
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Constructors and Destructor
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Default constructor
             */
            DynamicBWT()
            {
            }
            /**
             * @brief Move constructor
             */
            DynamicBWT(DynamicBWT &&) noexcept = default;

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Operators
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Deleted copy assignment operator
             */
            DynamicBWT &operator=(const DynamicBWT &) = delete;
            /**
             * @brief Default move assignment operator
             */
            DynamicBWT &operator=(DynamicBWT &&) noexcept = default;
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Return \p |L| (i.e., \p n)
             */
            int64_t size() const
            {
                return this->bwt.size();
            }

            /**
             * @brief Returns the total memory usage in bytes
             */
            uint64_t size_in_bytes() const
            {
                return this->cArray.size_in_bytes() + this->bwt.size_in_bytes();
            }

            /**
             * @brief Return the alphabet size \p σ
             */
            uint64_t get_alphabet_size() const
            {
                return this->bwt.get_alphabet_size();
            }
            /**
             * @brief Return the smallest character in the alphabet \p Σ
             */
            uint8_t get_end_marker() const
            {
                return this->bwt.get_smallest_character_in_alphabet();
            }
            /**
             * @brief Return the reference to the C array in this instance
             */
            const stool::dynamic_r_index::CArray &get_c_array() const
            {
                return this->cArray;
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Return the effective alphabet \p Σ'[0..σ'-1]  as a vector
             * @note O(σ') time
             */
            std::vector<uint8_t> get_effective_alphabet() const
            {
                return this->cArray.get_effective_alphabet();
            }

            /**
             * @brief Return the rank of a given character \p c in the effective alphabet \p Σ'[0..σ'-1] if it exists, otherwise return -1.
             * @note O(σ') time
             */
            int64_t get_c_id(uint8_t c) const
            {
                return this->cArray.get_c_id(c);
            }

            /**
             * @brief Return the largest character in BWT \p L
             * @note O(log σ log n) time
             */
            int64_t get_end_marker_pos() const
            {
                return this->bwt.select(0, this->get_end_marker());
            }
            /**
             * @brief Return L[i]
             * @note O(log σ log n) time
             */
            uint8_t access(int64_t i) const
            {
                return this->bwt.at(i);
            }
            /**
             * @brief Return the number of occurrences of a character \p c in BWT \p L.
             * @note O(1) time
             */
            uint64_t count_c(uint8_t c) const
            {
                return this->bwt.count_c(c);
            }
            /**
             * @brief Counts the number of occurrences of a character \p c in \p L[0..i].
             * @note O(log σ log n) time
             */
            int64_t rank(int64_t i, uint8_t c) const
            {
                if(i < 0){
                    return 0;
                }
                return this->bwt.one_based_rank(i + 1, c);
            }
            /**
             * @brief Returns the position \p p of the (i+1)-th 1 in \p L if such a position exists, otherwise returns -1
             * @note O(log σ log n) time
             */
            int64_t select(int64_t ith, uint8_t c) const
            {
                return this->bwt.select(ith, c);
            }

            /**
             * @brief Compute the LF function for a given position \p i.
             * @note O(log σ log n) time
             */
            int64_t LF(int64_t i) const
            {
                assert(i >= 0);
                assert(i < (int64_t)this->size());
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif

                // uint8_t c = this->bwt[i];
                uint8_t c = this->access(i);

                // uint64_t lf = this->C[c] + this->rank(c, i);
                uint64_t lf = (this->cArray.at(c) + this->rank(i, c)) - 1;
                // assert((int64_t)this->bwt->rank(c, i + 1) == this->rank(c, i));

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::LF_time += elapsed;
                stool::LF_count += 1;
#endif
                // assert(lf == naive_LF(i));
                return lf;
            }

            /**
             * @brief Compute the special LF mapping for deletion
             * @note O(log σ log n) time
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
                            p = this->size() - 2;
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
             * @brief Return \p T as a vector of characters
             */
            std::vector<uint8_t> to_original_string() const
            {
                std::vector<uint8_t> r;
                r.resize(this->size(), 0);
                r[r.size() - 1] = this->get_end_marker();
                int64_t current_pos_on_bwt = this->get_end_marker_pos();
                int64_t x = this->size() - 1;
                while (x > 0)
                {
                    current_pos_on_bwt = this->LF(current_pos_on_bwt);
                    r[--x] = this->access(current_pos_on_bwt);
                }
                return r;
            }
            /**
             * @brief Return \p T as a string
             */
            std::string to_original_string_str() const
            {
                std::vector<uint8_t> r = this->to_original_string();
                std::string s(reinterpret_cast<const char*>(r.data()), r.size());
                return s;
            }
            /**
             * @brief Return \p L as a vector of characters
             */
            std::vector<uint8_t> to_bwt(int64_t endmarker = -1) const
            {
                std::vector<uint8_t> r;
                r.resize(this->size(), 0);
                for (int64_t i = 0; i < this->size(); i++)
                {
                    uint8_t c = this->access(i);
                    if (endmarker != -1 && c == this->get_end_marker())
                    {
                        r[i] = endmarker;
                    }
                    else
                    {
                        r[i] = c;
                    }
                }
                return r;
            }

            /**
             * @brief Return \p L as a string
             */
            std::string to_bwt_str(int64_t endmarker = -1) const
            {
                std::vector<uint8_t> r = this->to_bwt(endmarker);
                std::string s(reinterpret_cast<const char*>(r.data()), r.size());
                return s;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Update operations
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Initialize this instance with an empty BWT and a given alphabet
             */
            void initialize(const std::vector<uint8_t> &_alphabet)
            {
                if (_alphabet.size() == 0)
                {
                    std::cerr << "The alphabet of FM-index must contain a character" << std::endl;
                    assert(_alphabet.size() > 0);
                    throw std::invalid_argument("The alphabet of FM-index must contain a character");
                }
                this->bwt.set_alphabet(_alphabet);

                this->clear();
            }
            /**
             * @brief Swap operation
             */
            void swap(DynamicBWT &item)
            {
                this->cArray.swap(item.cArray);
                this->bwt.swap(item.bwt);
            }

            /**
             * @brief Clear all the elements in this instance
             * @param message_paragraph The paragraph depth of message logs
             */
            void clear(int message_paragraph = stool::Message::NO_MESSAGE)
            {
                // DynRankSWrapper::clear(*this->bwt);

                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Clearing Dynamic BWT..." << std::endl;
                }

                uint8_t end_marker = this->get_end_marker();
                this->bwt.clear();
                this->cArray.clear();
                this->bwt.push_back(end_marker);
                this->cArray.increase(end_marker, 1);

                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
                }
            }
            /**
             * @brief Replace \p L[pos] with \p c
             * @note O(σ + log σ log n) time
             */
            void set_character(int64_t pos, uint8_t c)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif

                this->cArray.decrease(this->access(pos));
                this->cArray.increase(c);

                this->bwt.remove(pos);
                this->bwt.insert(pos, c);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::wavelet_tree_update_time += elapsed;
                stool::wavelet_tree_update_count += 2;
#endif
            }
            /**
             * @brief Insert a given character \p c into \p L at position \p pos
             * @note O(σ + log σ log n) time
             */
            void insert(int64_t pos, uint8_t c)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif
                this->cArray.increase(c, 1);
                this->bwt.insert(pos, c);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::wavelet_tree_update_time += elapsed;
                stool::wavelet_tree_update_count += 1;
#endif
            }

            /**
             * @brief Remove the \pos-th character from \p L
             * @note O(σ + log σ log n) time
             */
            void remove(int64_t pos)
            {
#ifdef TIME_DEBUG
                std::chrono::system_clock::time_point start, end;
                start = std::chrono::system_clock::now();
#endif
                assert(pos < this->size());
                this->cArray.decrease(this->access(pos));
                this->bwt.remove(pos);

#ifdef TIME_DEBUG
                end = std::chrono::system_clock::now();
                uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                stool::wavelet_tree_update_time += elapsed;
                stool::wavelet_tree_update_count += 1;
#endif
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Print and verification functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Return the memory usage information of this data structure as a vector of strings
             * @param message_paragraph The paragraph depth of message logs
             */
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->cArray.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->bwt.get_memory_usage_info(message_paragraph + 1);

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicBWT: " + std::to_string(this->size_in_bytes()) + " bytes =");
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
            /**
             * @brief Print the memory usage information of this data structure
             * @param message_paragraph The paragraph depth of message logs (-1 for no output)
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
             * @brief Print the statistics of this data structure
             * @param message_paragraph The paragraph depth of message logs
             */
            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicBWT):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: " << this->size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: " << this->get_alphabet_size() << std::endl;

                this->cArray.print_statistics(message_paragraph + 1);
                this->bwt.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            /**
             * @brief Print the BWT \p L
             * @param message_paragraph The paragraph depth of message logs
             */
            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicBWT): text length = " << this->size() << std::endl;
                this->bwt.print_content(message_paragraph + 1);

                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Build this instance from a given BWT \p _bwt and alphabet \p _alphabet
             * @param message_paragraph The paragraph depth of message logs
             */
            static DynamicBWT build(const std::vector<uint8_t> &_bwt, const std::vector<uint8_t> &_alphabet, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0 && _bwt.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic BWT from BWT... " << std::flush;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicBWT r;
                r.initialize(_alphabet);
                r.bwt.clear();
                r.cArray.clear();

                // uint64_t counter = 0;

                {
                    stool::bptree::DynamicWaveletTree tmp_dwt = stool::bptree::DynamicWaveletTree::build(_bwt, _alphabet);
                    r.bwt.swap(tmp_dwt);
                }

                {
                    std::vector<uint64_t> count_c_vector;
                    count_c_vector.resize(256, 0);
                    for (uint8_t c : _alphabet)
                    {
                        uint64_t count = r.bwt.count_c(c);
                        count_c_vector[c] = count;
                    }

                    auto tmp_c_array = CArray::build(count_c_vector);
                    r.cArray.swap(tmp_c_array);
                }

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && _bwt.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)_bwt.size()) * 1000000;

                    std::cout << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }
            /**
             * @brief Save the given instance \p item to a file stream \p os
             */
            static void store_to_file(DynamicBWT &item, std::ofstream &os)
            {
                stool::dynamic_r_index::CArray::store_to_file(item.cArray, os);
                stool::bptree::DynamicWaveletTree::store_to_file(item.bwt, os);
            }
            /**
             * @brief Return the DynamicBWT instance loaded from a file stream \p ifs
             */
            static DynamicBWT load_from_file(std::ifstream &ifs)
            {
                stool::dynamic_r_index::CArray cArray = stool::dynamic_r_index::CArray::load_from_file(ifs);
                stool::bptree::DynamicWaveletTree bwt = stool::bptree::DynamicWaveletTree::load_from_file(ifs);

                DynamicBWT r;
                r.cArray.swap(cArray);
                r.bwt.swap(bwt);
                return r;
            }
            //@}
        };
    }
}

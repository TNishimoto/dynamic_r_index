#pragma once
#include "dynamic_fm_index/dynamic_bwt.hpp"
#include "dynamic_fm_index/dynamic_isa.hpp"
#include "dynamic_fm_index/dynamic_sampled_sa.hpp"
#include "stool/include/all.hpp"
#include "libdivsufsort/sa.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief A dynamic data structure of FM-index. This implementation requires $O(n log n)$ words for the input string of length $n$.
         *
         * @details The DynamicFMIndex class provides a dynamic implementation of the FM-index, which is a compressed full-text substring index.
         * It allows for efficient string operations such as insertion, deletion, and searching within a text.
         * The class uses a combination of dynamic BWT (Burrows-Wheeler Transform) and dynamic sampled suffix array to achieve these operations.
         *
         * \ingroup DynamicFMIndexes
         * \ingroup StringIndexes
         * @tparam T The type of the elements stored in the index.
         */
        class DynamicFMIndex
        {
            DynamicBWT dbwt; // Dynamic Burrows-Wheeler Transform
            // DynamicISA disa;
            DynamicSampledSA dsa; // Dynamic Sampled Suffix Array

        public:
            static inline constexpr uint LOAD_KEY = 99999999; // Key used for loading the index

            using Interval = std::pair<int64_t, int64_t>; // Type for representing intervals

            /**
             * @brief Default constructor for DynamicFMIndex.
             */
            DynamicFMIndex()
            {
                this->dsa.set_BWT(&this->dbwt);
            }

            /**
             * @brief Deleted copy assignment operator.
             */
            DynamicFMIndex &operator=(const DynamicFMIndex &) = delete;

            /**
             * @brief Move constructor for DynamicFMIndex.
             * @param other The DynamicFMIndex to move from.
             */
            DynamicFMIndex(DynamicFMIndex &&other) noexcept
            {
                this->dbwt = std::move(other.dbwt);
                this->dsa = std::move(other.dsa);
                this->dsa.set_BWT(&this->dbwt);
            };

            /**
             * @brief Move assignment operator for DynamicFMIndex.
             * @param other The DynamicFMIndex to move from.
             * @return Reference to the current DynamicFMIndex.
             */
            DynamicFMIndex &operator=(DynamicFMIndex &&other) noexcept
            {
                if (this != &other)
                {
                    this->dbwt = std::move(other.dbwt);
                    this->dsa = std::move(other.dsa);
                    this->dsa.set_BWT(&this->dbwt);
                }
                return *this;
            };

            /**
             * @brief Get a pointer to the dynamic BWT.
             * @return Pointer to the dynamic BWT.
             */
            DynamicBWT *_get_dbwt_pointer()
            {
                return &this->dbwt;
            }

            /**
             * @brief Get a pointer to the dynamic sampled suffix array.
             * @return Pointer to the dynamic sampled suffix array.
             */
            DynamicSampledSA *_get_dsa_pointer()
            {
                return &this->dsa;
            }

            /**
             * @brief Get the size of the alphabet used in the index.
             * @return The size of the alphabet.
             */
            uint64_t get_alphabet_size() const
            {
                return this->dbwt.get_alphabet_size();
            }

            /**
             * @brief Get the alphabet used in the index.
             * @return A vector containing the alphabet.
             */
            std::vector<uint8_t> get_alphabet() const
            {
                return this->dbwt.get_alphabet();
            }

            /**
             * @brief Get the end marker used in the index.
             * @return The end marker.
             */
            uint64_t get_end_marker() const
            {
                return this->dbwt.get_end_marker();
            }

            /**
             * @brief Get the sampling interval for the suffix array.
             * @return The sampling interval.
             */
            uint64_t get_samling_interval() const
            {
                return this->dsa.get_sampling_interval();
            }

            /**
             * @brief Get the number of sampled suffix array values.
             * @return The number of sampled suffix array values.
             */
            uint64_t get_sampled_suffix_array_values_count() const
            {
                return this->dsa.get_sampled_suffix_array_values_count();
            }

            /**
             * @brief Get the size of the index in bytes.
             * @return The size of the index in bytes.
             */
            uint64_t size_in_bytes() const
            {
                return this->dbwt.size_in_bytes() + this->dsa.size_in_bytes();
            }

            /**
             * @brief Get memory usage information for the index.
             * @param message_paragraph The paragraph to start the message from.
             * @return A vector of strings containing memory usage information.
             */
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->dbwt.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->dsa.get_memory_usage_info(message_paragraph + 1);

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicFMIndex: " + std::to_string(this->size_in_bytes()) + " bytes =");
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
             * @brief Print memory usage information for the index.
             * @param message_paragraph The paragraph to start the message from.
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
             * @brief Print statistics for the index.
             * @param message_paragraph The paragraph to start the message from.
             */
            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicFMIndex):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: " << this->size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: " << this->dbwt.get_alphabet_size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Sampling interval: " << this->dsa.get_sampling_interval() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of sampled sa-values: " << this->dsa.get_sampled_suffix_array_values_count() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The density of sampled sa-values: " << (this->size() / this->dsa.get_sampled_suffix_array_values_count()) << std::endl;

                this->dbwt.print_statistics(message_paragraph + 1);
                this->dsa.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /**
             * @brief Print light statistics for the index.
             * @param message_paragraph The paragraph to start the message from.
             */
            void print_light_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicFMIndex):" << std::endl;

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text length: \t\t\t\t\t" << this->size() << std::endl;
                if (this->text_size() < 1000)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text: \t\t\t\t\t" << stool::ConverterToString::to_visible_string(this->get_text_str()) << std::endl;
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Text: \t\t\t\t\t" << "[Omitted]" << std::endl;
                }
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet size: \t\t\t\t" << this->get_alphabet_size() << std::endl;
                auto alphabet = this->get_alphabet();
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Alphabet: \t\t\t\t\t" << stool::ConverterToString::to_integer_string_with_characters(alphabet) << std::endl;

                if (this->text_size() < 1000)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "BWT: \t\t\t\t\t\t" << stool::ConverterToString::to_visible_string(this->get_bwt_str()) << std::endl;
                }
                else
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "BWT: \t\t\t\t\t\t" << "[Omitted]" << std::endl;
                }

                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Sampling interval for Sampled suffix array: \t" << this->get_samling_interval() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of sampled sa-values: \t\t" << this->get_sampled_suffix_array_values_count() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Average sampling interval: \t\t\t" << (this->size() / this->get_sampled_suffix_array_values_count()) << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /**
             * @brief Print the content of the index.
             * @param message_paragraph The paragraph to start the message from.
             */
            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicFMIndex):" << std::endl;
                this->dbwt.print_content(message_paragraph + 1);
                this->dsa.print_content(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /**
             * @brief Access a character at a specific position in the index.
             * @param i The position to access.
             * @return The character at the specified position.
             */
            uint8_t access(int64_t i) const
            {
                return this->dbwt.access(i);
            }

            

            /**
             * @brief Get the rank of a character up to a specific position in the index.
             * @param c The character to find the rank for.
             * @param i The position up to which to find the rank.
             * @return The rank of the character.
             */
            int64_t rank(uint8_t c, int64_t i) const
            {
                return this->dbwt.rank(c, i);
            }

            /**
             * @brief Get the position of the ith occurrence of a character in the index.
             * @param c The character to find.
             * @param ith The occurrence number.
             * @return The position of the ith occurrence of the character.
             */
            int64_t select(uint8_t c, int64_t ith) const
            {
                return this->dbwt.select(c, ith);
            }

            /**
             * @brief Save the index to a file.
             * @param item The DynamicFMIndex to save.
             * @param os The output stream to save to.
             * @param message_paragraph The paragraph to start the message from.
             */
            static void store_to_file(DynamicFMIndex &item, std::ofstream &os, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Writing Dynamic FM-index..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t key = DynamicFMIndex::LOAD_KEY;
                os.write(reinterpret_cast<const char *>(&key), sizeof(uint64_t));

                DynamicBWT::store_to_file(item.dbwt, os);
                DynamicSampledSA::store_to_file(item.dsa, os);

                uint64_t text_size = item.size();
                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }

            /**
             * @brief Get the size of the text in the index.
             * @return The size of the text.
             */
            uint64_t text_size() const
            {
                return this->dbwt.size();
            }

            static DynamicFMIndex build_from_text(const std::vector<uint8_t> &text_with_end_marker, const std::vector<uint8_t> &alphabet_with_end_marker, int message_paragraph = stool::Message::NO_MESSAGE)
            {
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text_with_end_marker, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text_with_end_marker, sa, stool::Message::NO_MESSAGE);
                DynamicFMIndex r = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, message_paragraph);

                return r;
            }

            /**
             * @brief Build a DynamicFMIndex from data in a file.
             * @param ifs The input stream to read from.
             * @param message_paragraph The paragraph to start the message from.
             * @return The constructed DynamicFMIndex.
             */
            static DynamicFMIndex load_from_file(std::ifstream &ifs, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic FM-index from Data..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t _key = 0;
                ifs.read(reinterpret_cast<char *>(&_key), sizeof(uint64_t));
                if (_key != DynamicFMIndex::LOAD_KEY)
                {
                    std::cout << "key: " << _key << std::endl;
                    throw std::runtime_error("This data is not Dynamic FM-index!");
                }

                auto tmp1 = DynamicBWT::load_from_file(ifs);
                DynamicFMIndex r;
                r.dbwt.swap(tmp1);

                auto tmp2 = DynamicSampledSA::load_from_file(ifs, &r.dbwt);
                r.dsa.swap(tmp2);

                uint64_t text_size = r.size();

                st2 = std::chrono::system_clock::now();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            /**
             * @brief Build a DynamicFMIndex from BWT, alphabet, and ISA.
             * @param bwt The Burrows-Wheeler Transform.
             * @param alphabet The alphabet used in the BWT.
             * @param isa The inverse suffix array.
             * @param sampling_interval_of_SA The sampling interval for the suffix array.
             * @param message_paragraph The paragraph to start the message from.
             * @return The constructed DynamicFMIndex.
             */
            static DynamicFMIndex build(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet, const std::vector<uint64_t> &isa, uint64_t sampling_interval_of_SA = DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic FM-index from BWT and ISA..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicFMIndex r;
                DynamicBWT _dbwt = DynamicBWT::build(bwt, alphabet, stool::Message::increment_paragraph_level(message_paragraph));
                r.dbwt.swap(_dbwt);

                DynamicSampledSA _dsa = DynamicSampledSA::build(isa, &r.dbwt, sampling_interval_of_SA, stool::Message::increment_paragraph_level(message_paragraph));
                r.dsa.swap(_dsa);

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)bwt.size()) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            /**
             * @brief Build a DynamicFMIndex from BWT and alphabet.
             * @param bwt The Burrows-Wheeler Transform.
             * @param alphabet The alphabet used in the BWT.
             * @param sampling_interval_of_SA The sampling interval for the suffix array.
             * @param message_paragraph The paragraph to start the message from.
             * @return The constructed DynamicFMIndex.
             */
            static DynamicFMIndex build(const std::vector<uint8_t> &bwt, const std::vector<uint8_t> &alphabet, uint64_t sampling_interval_of_SA = DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic FM-index from BWT..." << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                stool::bwt::LFDataStructure lfds = stool::bwt::LFDataStructure::build(bwt, stool::Message::increment_paragraph_level(message_paragraph));
                stool::bwt::BackwardISA<stool::bwt::LFDataStructure> bisa;
                bisa.set(&lfds, lfds.get_end_marker_position(), lfds.get_text_size());

                DynamicFMIndex r;
                DynamicBWT _dbwt = DynamicBWT::build(bwt, alphabet, stool::Message::increment_paragraph_level(message_paragraph));
                r.dbwt.swap(_dbwt);

                DynamicSampledSA _dsa = DynamicSampledSA::build(bisa, &r.dbwt, sampling_interval_of_SA, stool::Message::increment_paragraph_level(message_paragraph));
                r.dsa.swap(_dsa);

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && bwt.size() > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)bwt.size()) * 1000000;

                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }

                return r;
            }

            /**
             * @brief Swap the contents of this DynamicFMIndex with another.
             * @param item The DynamicFMIndex to swap with.
             */
            void swap(DynamicFMIndex &item)
            {
                this->dbwt.swap(item.dbwt);
                this->dsa.swap(item.dsa);
                item.dsa.set_BWT(&item.dbwt);
                this->dsa.set_BWT(&this->dbwt);
            }

            /**
             * @brief Clear the contents of the DynamicFMIndex.
             * @param message_paragraph The paragraph to start the message from.
             */
            void clear(int message_paragraph = stool::Message::NO_MESSAGE)
            {
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Clearing Dynamic FM-index..." << std::endl;
                }
                this->dbwt.clear(stool::Message::increment_paragraph_level(message_paragraph));
                // this->disa.clear();
                this->dsa.clear();
                if (message_paragraph >= 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Finished. [Dynamic FM-index]" << std::endl;
                }
            }

            /**
             * @brief Set the degree for the DynamicFMIndex.
             * @param degree The degree to set.
             */
            /*
            void set_degree(uint64_t degree)
            {
                this->dbwt.clear();
                // this->disa.clear();
                this->dsa.clear();
                // this->disa.set_degree(degree);
                this->dsa.set_degree(degree);
            }
            */

            /**
             * @brief Initialize the DynamicFMIndex with an alphabet.
             * @param alphabet The alphabet to initialize with.
             */
            void initialize(std::vector<uint8_t> &alphabet)
            {
                this->dbwt.initialize(alphabet);
            }

            /**
             * @brief Get the size of the DynamicFMIndex.
             * @return The size of the DynamicFMIndex.
             */
            uint64_t size() const
            {
                return this->dbwt.size();
            }

            /**
             * @brief Perform a backward search on the DynamicFMIndex.
             * @param intv The interval to search in.
             * @param c The character to search for.
             * @return The result of the backward search.
             */
            BackwardSearchResult backward_search(const Interval &intv, uint8_t c) const
            {
                uint64_t num1 = this->dbwt.rank(c, intv.first - 1);
                uint64_t c_count = this->dbwt.get_c_array().get_c_count(c);
                if (num1 < c_count)
                {
                    uint64_t fst_c_pos = this->dbwt.select(c, num1 + 1);

                    if ((int64_t)fst_c_pos <= intv.second)
                    {
                        uint64_t last_ith = this->dbwt.rank(c, intv.second);

                        uint64_t lst_c_pos = this->dbwt.select(c, last_ith);

                        return BackwardSearchResult(this->dbwt.LF(fst_c_pos), this->dbwt.LF(lst_c_pos));
                    }
                    else
                    {
                        return BackwardSearchResult::create_empty_result();
                    }
                }
                else
                {
                    return BackwardSearchResult::create_empty_result();
                }
            }

            /**
             * @brief Perform a backward search on the DynamicFMIndex with a pattern.
             * @param pattern The pattern to search for.
             * @return The result of the backward search.
             */
            BackwardSearchResult backward_search(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr(0, this->size() - 1);
                // Interval intv(0, this->size() - 1);
                for (int64_t i = pattern.size() - 1; i >= 0; i--)
                {
                    bsr = this->backward_search(bsr.get_sa_interval(), pattern[i]);
                    if (bsr.is_empty())
                    {
                        break;
                    }
                }
                return bsr;
            }

            /**
             * @brief Compute the suffix array values for a given interval.
             * @param intv The interval to compute suffix array values for.
             * @return A vector of suffix array values.
             */
            std::vector<uint64_t> compute_sa_values(const Interval &intv) const
            {
                std::vector<uint64_t> r;
                for (int64_t i = intv.first; i <= intv.second; i++)
                {
                    r.push_back(this->dsa.sa(i));
                }
                return r;
            }

            /**
             * @brief Compute the suffix array values for a given backward search result.
             * @param bsr The backward search result to compute suffix array values for.
             * @return A vector of suffix array values.
             */
            std::vector<uint64_t> compute_sa_values(const BackwardSearchResult &bsr) const
            {
                if (bsr.is_empty())
                {
                    return std::vector<uint64_t>();
                }
                else
                {
                    return this->compute_sa_values(bsr.get_sa_interval());
                }
            }

            /**
             * @brief Count the number of occurrences of a pattern in the index.
             * @param pattern The pattern to count.
             * @return The number of occurrences of the pattern.
             */
            uint64_t count_query(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr = this->backward_search(pattern);
                if (bsr.is_empty())
                {
                    return 0;
                }
                else
                {
                    return bsr.get_sa_interval_size();
                }
            }

            /**
             * @brief Locate the positions of a pattern in the index.
             * @param pattern The pattern to locate.
             * @return A vector of positions where the pattern occurs.
             */
            std::vector<uint64_t> locate_query(const std::vector<uint8_t> &pattern) const
            {
                BackwardSearchResult bsr = this->backward_search(pattern);
                return this->compute_sa_values(bsr);
            }

            /**
             * @brief Get the BWT of the index.
             * @return A vector containing the BWT.
             */
            std::vector<uint8_t> get_bwt() const
            {
                return this->dbwt.get_bwt();
            }

            /**
             * @brief Get the text of the index.
             * @return A vector containing the text.
             */
            std::vector<uint8_t> get_text() const
            {
                return this->dbwt.get_text();
            }

            /**
             * @brief Get the BWT of the index as a string.
             * @return A string containing the BWT.
             */
            std::string get_bwt_str() const
            {
                return this->dbwt.get_bwt_str();
            }

            /**
             * @brief Get the text of the index as a string.
             * @return A string containing the text.
             */
            std::string get_text_str() const
            {
                return this->dbwt.get_text_str();
            }

            /**
             * @brief Get the suffix array of the index.
             * @return A vector containing the suffix array.
             */
            std::vector<uint64_t> get_sa() const
            {
                return this->dsa.get_sa();
            }

            /**
             * @brief Get the inverse suffix array of the index.
             * @return A vector containing the inverse suffix array.
             */
            std::vector<uint64_t> get_isa() const
            {
                return this->dsa.get_isa();
            }

            /**
             * @brief Print the BWT table of the index.
             */
            void print_bwt_table() const
            {
                std::vector<uint64_t> sa = this->get_sa();
                std::vector<uint8_t> bwt = this->get_bwt();
                stool::DebugPrinter::print_bwt_table(bwt, sa);
            }

            /**
             * @brief Insert a character into the index.
             * @param pos The position to insert the character at.
             * @param c The character to insert.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t insert_char(int64_t pos, uint8_t c, FMIndexEditHistory &output_history)
            {
                output_history.clear();
                return this->insert_char(pos, c, &output_history);
            }
            uint64_t insert_string(int64_t pos, uint8_t c)
            {
                return this->insert_char(pos, c, nullptr);
            }

            /**
             * @brief Insert a character into the index.
             * @param pos The position to insert the character at.
             * @param c The character to insert.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t insert_char(int64_t pos, uint8_t c, FMIndexEditHistory *output_history = nullptr)
            {
                assert(c > this->dbwt.get_end_marker());

                SAIndex isa_of_insertionPosOnText = this->dsa.isa(pos);
                // assert(isa_of_insertionPosOnText == this->dsa.isa(pos));

                uint64_t positionToReplace = isa_of_insertionPosOnText;
                uint64_t prev_isa = this->dbwt.LF(isa_of_insertionPosOnText);
                uint64_t oldPositionToInsert = positionToReplace;

                if (output_history != nullptr)
                {
                    output_history->replaced_sa_index = positionToReplace;
                    output_history->type = EditType::InsertionOfChar;
                    output_history->inserted_string.push_back(c);
                }

                uint8_t oldChar = this->dbwt.access(positionToReplace);
                uint64_t j = prev_isa;
                uint8_t new_letter_L = c;

                this->dbwt.replace_BWT_character(positionToReplace, new_letter_L);
                // this->dbwt.update_C_for_deletion(oldChar);
                // this->dbwt.update_C_for_insertion(new_letter_L);

                uint64_t C_count = this->dbwt.get_c_array().at(new_letter_L);
                if (oldChar < new_letter_L)
                {
                    C_count++;
                }
                uint64_t rank_value = this->dbwt.rank(new_letter_L, positionToReplace);
                if (positionToReplace <= oldPositionToInsert && new_letter_L == oldChar)
                {
                    rank_value++;
                }

                uint64_t positionToInsert = C_count + rank_value - 1;

                if (output_history != nullptr)
                {
                    output_history->inserted_sa_index = positionToInsert;
                    output_history->inserted_sa_indexes.push_back(positionToInsert);
                }

                this->dbwt.insert_BWT_character(positionToInsert, oldChar);
                if (positionToInsert <= j)
                {
                    j++;
                }
                if (positionToInsert <= positionToReplace)
                {
                    positionToReplace++;
                }

                // this->disa.update_for_insertion(positionToInsert, pos);
                this->dsa.update_for_insertion(positionToInsert, pos);

                uint64_t j_prime = this->dbwt.LF(positionToInsert);

                // std::cout << "j = " << j << ", j' = " << j_prime << std::endl;
                if (output_history != nullptr)
                {
                    output_history->first_j = j;
                    output_history->first_j_prime = j_prime;
                }
                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);
                uint64_t sum = swap_history.size();

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }

                this->dsa.update_sample_marks(pos + 1);

                return sum;
            }

            /**
             * @brief Insert a string into the index.
             * @param pos The position to insert the string at.
             * @param pattern The string to insert.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t insert_string(int64_t pos, const std::vector<uint8_t> &pattern, FMIndexEditHistory &output_history)
            {
                return insert_string(pos, pattern, &output_history);
            }

            /**
             * @brief Insert a string into the index.
             * @param pos The position to insert the string at.
             * @param pattern The string to insert.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t insert_string(int64_t pos, const std::vector<uint8_t> &pattern, FMIndexEditHistory *output_history = nullptr)
            {
                assert(pattern.size() > 0);

                SAIndex isa_of_insertionPosOnText = this->dsa.isa(pos);
                // assert(isa_of_insertionPosOnText == this->dsa.isa(pos));
                uint64_t positionToReplace = isa_of_insertionPosOnText;
                uint64_t prev_isa = this->dbwt.LF(isa_of_insertionPosOnText);
                uint64_t oldPositionToInsert = positionToReplace;

                if (output_history != nullptr)
                {
                    output_history->replaced_sa_index = positionToReplace;
                    output_history->type = EditType::InsertionOfString;

                    for (size_t i = 0; i < pattern.size(); i++)
                    {
                        output_history->inserted_string.push_back(pattern[i]);
                    }
                }

                uint8_t oldChar = this->dbwt.access(positionToReplace);
                uint64_t j = prev_isa;
                assert(pattern.size() > 0);
                uint8_t new_letter_L = pattern[pattern.size() - 1];

                this->dbwt.replace_BWT_character(positionToReplace, pattern[pattern.size() - 1]);

                uint64_t C_count = this->dbwt.get_c_array().at(new_letter_L);
                if (oldChar < new_letter_L)
                {
                    C_count++;
                }
                uint64_t rank_value = this->dbwt.rank(new_letter_L, positionToReplace);
                if (positionToReplace <= oldPositionToInsert && new_letter_L == oldChar)
                {
                    rank_value++;
                }

                uint64_t positionToInsert = C_count + rank_value - 1;

                for (int64_t k = pattern.size() - 1; k > 0; k--)
                {
                    new_letter_L = pattern[k - 1];

                    this->dbwt.insert_BWT_character(positionToInsert, new_letter_L);
                    this->dsa.update_for_insertion(positionToInsert, pos);

                    if (output_history != nullptr)
                    {
                        output_history->inserted_sa_indexes.push_back(positionToInsert);
                    }

                    if (positionToInsert <= j)
                    {
                        j++;
                    }
                    if (positionToInsert <= positionToReplace)
                    {
                        positionToReplace++;
                    }

                    oldPositionToInsert = positionToInsert;
                    int64_t C_count = this->dbwt.get_c_array().at(new_letter_L);
                    if (oldChar < new_letter_L)
                    {
                        C_count++;
                    }
                    int64_t rank_value = this->dbwt.rank(new_letter_L, positionToInsert);
                    if (positionToReplace <= oldPositionToInsert && pattern[k - 1] == oldChar)
                        rank_value++;

                    positionToInsert = C_count + rank_value - 1;
                }

                new_letter_L = oldChar;

                if (output_history != nullptr)
                {
                    output_history->inserted_sa_indexes.push_back(positionToInsert);
                }

                this->dbwt.insert_BWT_character(positionToInsert, oldChar);
                // this->dbwt.update_C_for_insertion(oldChar);
                if (positionToInsert <= j)
                {
                    j++;
                }
                if (positionToInsert <= positionToReplace)
                {
                    positionToReplace++;
                }

                // this->disa.update_for_insertion(positionToInsert, pos);
                this->dsa.update_for_insertion(positionToInsert, pos);

                int64_t j_prime = this->dbwt.LF(positionToInsert);

                // std::cout << "j = " << j << ", j' = " << j_prime << std::endl;

                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }

                this->dsa.update_sample_marks(pos + pattern.size());

                return 0;
            }

            uint64_t delete_substring(int64_t pos)
            {
                return this->delete_char(pos, nullptr);
            }

            /**
             * @brief Delete a character from the index.
             * @param pos The position to delete the character from.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t delete_char(int64_t pos, FMIndexEditHistory &output_history)
            {
                output_history.clear();
                return this->delete_char(pos, &output_history);
            }

            /**
             * @brief Delete a character from the index.
             * @param pos The position to delete the character from.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t delete_char(int64_t pos, FMIndexEditHistory *output_history = nullptr)
            {

                SAIndex isa_pos = this->dsa.isa(pos);
                // assert(isa_pos = this->dsa.isa(pos));

                TextIndex pointer = pos + 1 < (int64_t)this->size() ? pos + 1 : 0;

                SAIndex isa_pos_succ = this->dsa.isa(pointer);
                // assert(isa_pos_succ == this->dsa.isa(pointer));
                uint8_t newChar = this->dbwt.access(isa_pos);

                SAIndex isa_pos_prev = this->dbwt.LF(isa_pos);
                // uint8_t oldChar = this->dbwt.access(isa_pos_succ);

                this->dbwt.replace_BWT_character(isa_pos_succ, newChar);
                this->dbwt.remove_BWT_character(isa_pos);
                // this->dbwt.update_C_for_deletion(oldChar);

                SAIndex new_isa_pos_succ = isa_pos_succ <= isa_pos ? isa_pos_succ : isa_pos_succ - 1;
                SAIndex new_isa_pos_prev = isa_pos_prev <= isa_pos ? isa_pos_prev : isa_pos_prev - 1;

                if (output_history != nullptr)
                {
                    output_history->replaced_sa_index = isa_pos_succ;
                    output_history->inserted_sa_index = isa_pos;
                }
                // this->disa.update_for_deletion(isa_pos);
                this->dsa.update_for_deletion(isa_pos, pos);

                /*
                auto _sa = this->disa.get_sa();
                stool::DebugPrinter::print_integers(_sa, "c SA");
                this->dsa.print_info();
                */

                std::vector<SAMove> swap_history = reorder_BWT(new_isa_pos_prev, this->dbwt.LF(new_isa_pos_succ));
                uint64_t sum = swap_history.size();

                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }

                this->dsa.update_sample_marks(pos);

                return sum;
            }

            /**
             * @brief Delete a string from the index.
             * @param pos The position to delete the string from.
             * @param len The length of the string to delete.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t delete_string(const int64_t pos, int64_t len, FMIndexEditHistory &output_history)
            {
                return delete_string(pos, len, &output_history);
            }

            /**
             * @brief Delete a string from the index.
             * @param pos The position to delete the string from.
             * @param len The length of the string to delete.
             * @param output_history The history of edits to output.
             * @return The number of operations performed.
             */
            uint64_t delete_string(const int64_t pos, int64_t len, FMIndexEditHistory *output_history = nullptr)
            {

                if (pos + len >= (int64_t)this->size())
                {
                    throw std::logic_error("Error: delete_string");
                }

                TextIndex pointer = pos + len < (int64_t)this->size() ? pos + len : 0;

                uint64_t positionToReplace = this->dsa.isa(pointer);

                SAIndex isa_pos2 = this->dsa.isa(pos);
                uint8_t new_char = this->dbwt.access(isa_pos2);
                // uint8_t old_char = this->dbwt.access(positionToReplace);
                uint64_t positionToDelete = this->dbwt.LF(positionToReplace);
                this->dbwt.replace_BWT_character(positionToReplace, new_char);

                uint64_t positionToDeleteOnText = pointer - 1;

                if (output_history != nullptr)
                {
                    output_history->type = EditType::DeletionOfString;
                    output_history->replaced_sa_index = positionToReplace;
                }

                // this->dbwt.replace_BWT_character(positionToReplace, new_char);

                // uint64_t tmp_rank;
                for (int64_t k = len - 1; k >= 0; k--)
                {

                    uint8_t current_letter = this->dbwt.access(positionToDelete);
                    uint64_t next_position_to_delete = UINT64_MAX;
                    next_position_to_delete = this->dbwt.LF(positionToDelete);
                    if ((new_char < current_letter) || (positionToReplace <= positionToDelete && current_letter == new_char))
                    {
                        next_position_to_delete--;
                    }

                    if (output_history != nullptr)
                    {
                        output_history->deleted_sa_indexes.push_back(positionToDelete);
                    }

                    this->dbwt.remove_BWT_character(positionToDelete);
                    this->dsa.update_for_deletion(positionToDelete, positionToDeleteOnText);

                    if (positionToDelete < positionToReplace)
                    {
                        positionToReplace--;
                    }
                    positionToDelete = next_position_to_delete;

                    positionToDeleteOnText--;
                }

                uint64_t j = positionToDelete;
                uint64_t j_prime = this->dbwt.LF(positionToReplace);

                if (output_history != nullptr)
                {
                    output_history->deleted_sa_indexes.push_back(positionToDelete);

                    output_history->first_j = j;
                    output_history->first_j_prime = j_prime;
                }

                // std::cout << "j = " << j << ", j' = " << j_prime << std::endl;
                std::vector<SAMove> swap_history = reorder_BWT(j, j_prime);
                if (output_history != nullptr)
                {
                    output_history->move_history.swap(swap_history);
                }
                this->dsa.update_sample_marks(pos);
                return 0;
            }

        private:
            /**
             * @brief Move a row in the BWT.
             * @param j The position to move from.
             * @param j_prime The position to move to.
             */
            void move_row(int64_t j, int64_t j_prime)
            {
                uint8_t j_char = this->dbwt.access(j);

                this->dbwt.remove_BWT_character(j);
                this->dbwt.insert_BWT_character(j_prime, j_char);
            }

            /**
             * @brief Perform a single reorder operation on the BWT.
             * @param j The position to reorder from.
             * @param j_prime The position to reorder to.
             */
            void single_reorder_BWT(int64_t &j, int64_t &j_prime)
            {
                int64_t new_j = this->dbwt.LF(j);

                this->move_row(j, j_prime);
                // this->disa.move_update(j, j_prime);
                this->dsa.move_update(j, j_prime);

                j = new_j;
                j_prime = this->dbwt.LF(j_prime);
            }

            /**
             * @brief Reorder the BWT.
             * @param j The position to reorder from.
             * @param j_prime The position to reorder to.
             * @return A vector of moves performed during the reorder.
             */
            std::vector<SAMove> reorder_BWT(int64_t j, int64_t j_prime)
            {

                std::vector<SAMove> swap_history;
                int64_t counter = 0;

                while (j != j_prime)
                {
                    // std::cout << "REORDER: j = " << j << ", j' = " << j_prime << std::endl;
                    swap_history.push_back(SAMove(j, j_prime));

                    this->single_reorder_BWT(j, j_prime);

                    counter++;

                    if (counter > (int64_t)this->size())
                    {
                        throw std::logic_error("Error: modify_SA_for_insertion");
                    }
                }
                // swap_history.push_back(SAMove(j, j_prime));

                return swap_history;
            }
        };
    }
}

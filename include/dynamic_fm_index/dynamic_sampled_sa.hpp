/**
 * @file dynamic_sampled_sa.hpp
 * @brief A dynamic data structure for accessing SA and ISA.
 */

#pragma once
#include "stool/include/lib.hpp"
#include "b_tree_plus_alpha/include/all.hpp"
#include "./dynamic_bwt.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief O(n log n / k)-bit dynamic data structure for accessing SA[0..n-1] and ISA[0..n-1] sampled by the sampling interval \p k.
         * @details This data structure consists of the following three components:
         * - Dynamic bit sequence \p B[0..n-1]: B[i] = 1 if and only if SA[i] is a sampled value.
         * - Dynamic bit sequence \p F[0..n-1]: F[i] = 1 if and only if i is a sampled SA value.
         * - Dynamic permutation \p Π[0..n/k]: Let j be the (i+1)-th sampled position in SA. Then Π[i] stores the number of 1 in F[0..SA[j]-1]. Here, SA[j] = select1_{F}(Π[i]) holds.
         * \ingroup DynamicFMIndexes
         * \ingroup MainDataStructures
         */
        class DynamicSampledSA
        {
            stool::bptree::DynamicPermutation dp; // Π
            stool::bptree::SimpleDynamicBitSequence sample_marks_on_text; // F
            stool::bptree::SimpleDynamicBitSequence sample_marks_on_sa; // B
            uint64_t sampling_interval = 32;
            DynamicBWT *bwt = nullptr;

            // stool::old_implementations::VPomPermutation pom;

        public:
            /**
             * @brief Default sampling interval.
             */
            static inline constexpr uint64_t DEFAULT_SAMPLING_INTERVAL = 32;
            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Constructors and Destructor
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Default constructor.
             */
            DynamicSampledSA()
            {
                this->clear();
            }
            /**
             * @brief Default move constructor.
             */
            DynamicSampledSA(DynamicSampledSA &&) noexcept = default;

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Operators
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Deleted copy assignment operator.
             */
            DynamicSampledSA &operator=(const DynamicSampledSA &) = delete;
            /**
             * @brief Default move assignment operator.
             */
            DynamicSampledSA &operator=(DynamicSampledSA &&) noexcept = default;
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Lightweight functions for accessing to properties of this class
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Return the size \p n of SA[0..n-1]
             */
            uint64_t size() const
            {
                return this->sample_marks_on_text.size();
            }
            /**
             * @brief Return the total memory usage in bytes
             */
            uint64_t size_in_bytes() const
            {
                return this->dp.size_in_bytes() + this->sample_marks_on_text.size_in_bytes() + this->sample_marks_on_sa.size_in_bytes() + sizeof(this->sampling_interval) + sizeof(this->bwt);
            }

            /**
             * @brief Return the number of values in the sampled suffix array.
             */
            uint64_t get_sampled_suffix_array_size() const
            {
                return this->sample_marks_on_text.count_c(true);
            }

            /**
             * @brief Set the BWT for the DynamicSampledSA.
             */
            void set_BWT(DynamicBWT *_bwt)
            {
                this->bwt = _bwt;
            }
            /**
             * @brief Returns the length \p k of the sampling interval.
             */
            uint64_t get_sampling_interval() const
            {
                return this->sampling_interval;
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Main queries
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Return the (i+1)-th sampled ISA value.
             * @note O(log n) time
             */
            int64_t sampled_isa(uint64_t i) const
            {
                assert(this->bwt != nullptr);
                assert(i < this->dp.size());
                uint64_t pos = this->dp.inverse(i);
                return this->sample_marks_on_sa.select1(pos);
            }
            /**
             * @brief Return the (i+1)-th sampled SA value.
             * @note O(log n) time
             */
            int64_t sampled_sa(uint64_t i) const
            {
                assert(this->bwt != nullptr);
                assert(i < this->dp.size());
                uint64_t pos = this->dp.access(i);
                return this->sample_marks_on_text.select1(pos);
            }

            /**
             * @brief Return SA[i].
             * @note O(k (log n + LF)) time, where LF is the time complexity of the LF function.
             */
            int64_t sa(uint64_t i) const
            {
                assert(this->bwt != nullptr);
                std::pair<uint64_t, uint64_t> pair = this->sample_predecessor_on_sa(i);
                return sa(pair.first, pair.second);
            }

            /**
             * @brief Return ISA[i].
             * @note O(k (log n + LF)) time, where LF is the time complexity of the LF function.
             */
            int64_t isa(uint64_t i) const
            {
                assert(this->bwt != nullptr);

                uint64_t rank = this->sample_marks_on_text.one_based_rank1(i + 1);
                bool b = this->sample_marks_on_text[i];
                if (b)
                {
                    return this->sampled_isa(rank - 1);
                }
                else
                {
                    uint64_t sel = this->sample_marks_on_text.select1(rank);
                    uint64_t dist = sel - i;
                    uint64_t sa_pos = this->sampled_isa(rank);
                    for (uint64_t j = 0; j < dist; j++)
                    {
                        sa_pos = this->bwt->LF(sa_pos);
                    }
                    return sa_pos;
                }
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Convertion functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Return the suffix array as a vector.
             */
            std::vector<uint64_t> to_sa() const
            {
                assert(this->bwt != nullptr);
                std::vector<uint64_t> r;
                r.resize(this->size(), UINT64_MAX);
                for (uint64_t i = 0; i < this->size(); i++)
                {
                    r[i] = this->sa(i);
                }
                return r;
            }
            /**
             * @brief Return the inverse suffix array as a vector.
             */
            std::vector<uint64_t> to_isa() const
            {
                assert(this->bwt != nullptr);
                std::vector<uint64_t> r;
                r.resize(this->size(), UINT64_MAX);
                for (uint64_t i = 0; i < this->size(); i++)
                {
                    r[i] = this->isa(i);
                }
                return r;
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Update operations
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /**
             * @brief Swap operation
             */
            void swap(DynamicSampledSA &item)
            {
                this->dp.swap(item.dp);
                this->sample_marks_on_text.swap(item.sample_marks_on_text);
                this->sample_marks_on_sa.swap(item.sample_marks_on_sa);
                std::swap(this->sampling_interval, item.sampling_interval);
                std::swap(this->bwt, item.bwt);
            }

            /**
             * @brief Clear all the elements in this instance
             */
            void clear()
            {
                this->dp.clear();
                this->sample_marks_on_text.clear();
                this->sample_marks_on_sa.clear();
                this->dp.insert(0, 0);
                this->sample_marks_on_text.push_back(1);
                this->sample_marks_on_sa.push_back(1);
            }
            /**
             * @brief Move the bit \p SA[j] to a given position \p j' in \p SA
             * @note O(log n) time
             */
            void move_update(int64_t j, int64_t j_prime)
            {
                bool b = this->sample_marks_on_sa.at(j);
                bool b_prime = this->sample_marks_on_sa.at(j_prime);

                if (b)
                {
                    uint64_t rank = this->sample_marks_on_sa.one_based_rank1(j);
                    uint64_t rank_prime = this->sample_marks_on_sa.one_based_rank1(j_prime);

                    if (b_prime)
                    {
                        this->dp.move_pi_index(rank, rank_prime);
                    }
                    else
                    {
                        this->move_pi_index2(rank, rank_prime);
                    }
                }
                this->move_sa_bit(j, j_prime);
            }


            /**
             * @brief Mark SA[t] as a sampled value and update this instance accordingly
             * @note O(log n) time
             */
            void update_sample_marks(uint64_t t)
            {
                if (t > 0)
                {
                    uint64_t rank = this->sample_marks_on_text.one_based_rank1(t);
                    this->update_sample_marks_sub(rank);
                }
                else
                {
                    bool b = this->sample_marks_on_text[0];
                    if (!b)
                    {
                        uint64_t sa_pos0 = this->isa(0);
                        uint64_t sample_sa_rank0 = this->sample_marks_on_sa.one_based_rank1(sa_pos0);
                        this->dp.insert(sample_sa_rank0, 0);
                        this->sample_marks_on_text.set_bit(0, true);
                        this->sample_marks_on_sa.set_bit(sa_pos0, true);
                    }
                }
                this->verify();
            }
            /**
             * @brief Deletes SA[x] from SA and ISA[y] from ISA, where SA[x] = y
             * @note O(log n) time
             */
            void update_for_deletion(int64_t removed_sa_index_x, uint64_t removed_text_index_y)
            {

                bool b = this->sample_marks_on_sa.at(removed_sa_index_x);
                this->sample_marks_on_text.remove(removed_text_index_y);
                this->sample_marks_on_sa.remove(removed_sa_index_x);
                if (b)
                {
                    uint64_t rank = this->sample_marks_on_sa.one_based_rank1(removed_sa_index_x);
                    this->dp.erase(rank);
                }
            }

            /**
             * @brief Insert x and y into SA and ISA as SA[x] = y and ISA[y] = x, respectively
             * @note O(log n) time
             */
            void update_for_insertion(int64_t inserted_sa_index_x, int64_t inserted_text_position_y)
            {
                this->sample_marks_on_text.insert(inserted_text_position_y, false);
                this->sample_marks_on_sa.insert(inserted_sa_index_x, false);
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
                std::vector<std::string> log1 = this->dp.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->sample_marks_on_text.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log3 = this->sample_marks_on_sa.get_memory_usage_info(message_paragraph + 1);

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=DynamicSampledSA: " + std::to_string(this->size_in_bytes()) + " bytes =");
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
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicSampledSA):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Sampling interval: " << this->get_sampling_interval() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The number of sampled sa-values: " << this->get_sampled_suffix_array_size() << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "The density of sampled sa-values: " << (this->sample_marks_on_text.size() / this->get_sampled_suffix_array_size()) << std::endl;

                this->dp.print_statistics(message_paragraph + 1);
                this->sample_marks_on_text.print_statistics(message_paragraph + 1);
                this->sample_marks_on_sa.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            /**
             * @brief Prints the content of this instance.
             * @param message_paragraph The message paragraph.
             */
            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicSampledSA):" << std::endl;
                this->dp.print_content(message_paragraph + 1);
                this->sample_marks_on_text.print_content("sample_marks_on_text", message_paragraph + 1);
                this->sample_marks_on_sa.print_content("sample_marks_on_sa", message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            /**
             * @brief Prints information about this instance.
             */
            void print_info() const
            {
                std::string s1 = this->sample_marks_on_text.to_string();
                std::string s2 = this->sample_marks_on_sa.to_string();

                std::vector<uint64_t> pi = this->dp.to_pi_vector();
                std::vector<uint64_t> inv_pi = this->dp.to_inverse_pi_vector();
                std::string s3 = stool::ConverterToString::to_integer_string(pi);
                std::string s4 = stool::ConverterToString::to_integer_string(inv_pi);

                std::vector<uint64_t> _sampled_sa;
                for (uint64_t i = 0; i < pi.size(); i++)
                {
                    _sampled_sa.push_back(this->sampled_sa(i));
                }
                std::string s5 = stool::ConverterToString::to_integer_string(_sampled_sa);

                std::cout << "======== DynamicSampledSA =======" << std::endl;
                std::cout << "text bits: \t" << s1 << std::endl;
                std::cout << "sa bits: \t" << s2 << std::endl;
                std::cout << "Sampled SA: " << s5 << std::endl;
                std::cout << "pi: \t \t" << s3 << std::endl;
                std::cout << "pi^{-1}: \t " << s4 << std::endl;
                std::cout << "==================================" << std::endl;
            }


            /**
             * @brief Verify the internal consistency of this data structure.
             */
            void verify() const
            {
                uint64_t p1 = this->sample_marks_on_text.count_c(true);
                uint64_t p2 = this->sample_marks_on_sa.count_c(true);
                if (p1 != p2)
                {
                    assert(false);
                }

                if (p1 != this->dp.size())
                {
                    assert(false);
                }
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Load, save, and builder functions
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Builds a DynamicSampledSA from BISA and BWT, where BISA is the backward iterator of ISA
             */
            static DynamicSampledSA build(const stool::bwt::BackwardISA<stool::bwt::LFDataStructure> &BISA, DynamicBWT *_bwt, uint64_t sampling_interval = DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = _bwt->size();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic Sampled Suffix Array from BWT... " << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicSampledSA r;
                r.set_BWT(_bwt);
                r.set_sampling_interval(sampling_interval);

                assert(r.bwt != nullptr);
                // r.sample_marks_on_text.clear();
                // r.sample_marks_on_sa.clear();

                {
                    std::vector<bool> _sample_marks_on_text;
                    std::vector<bool> _sample_marks_on_sa;
                    std::vector<uint64_t> output_sampled_sa_rank;
                    std::vector<uint64_t> output_sampled_sa;

                    DynamicSampledSA::build_sampled_sa_and_bits(BISA, text_size, sampling_interval, _sample_marks_on_text, _sample_marks_on_sa, output_sampled_sa_rank, output_sampled_sa, stool::Message::increment_paragraph_level(message_paragraph));

                    auto tmp1 = stool::bptree::SimpleDynamicBitSequence::build(_sample_marks_on_text);
                    auto tmp2 = stool::bptree::SimpleDynamicBitSequence::build(_sample_marks_on_sa);

                    r.dp.clear();
                    r.sample_marks_on_text.swap(tmp1);
                    r.sample_marks_on_sa.swap(tmp2);
                    r.dp.build(output_sampled_sa_rank.begin(), output_sampled_sa_rank.end(), output_sampled_sa_rank.size(), stool::Message::increment_paragraph_level(message_paragraph));
                }

                // r.dp.clear();
                // DynamicSampledSA::build_sample_sa(BISA, text_size, sampling_interval, output_sampled_sa_rank, output_sampled_sa, stool::Message::increment_paragraph_level(message_paragraph));
                // r.dp.build(output_sampled_sa_rank.begin(), output_sampled_sa_rank.end(), output_sampled_sa_rank.size(), stool::Message::increment_paragraph_level(message_paragraph));

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
             * @brief Builds a DynamicSampledSA from ISA and BWT
             */
            static DynamicSampledSA build(const std::vector<uint64_t> &isa, DynamicBWT *bwt, uint64_t sampling_interval = DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = isa.size();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Dynamic Sampled Suffix Array from BWT and ISA... " << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                DynamicSampledSA r;
                r.set_BWT(bwt);
                r.set_sampling_interval(sampling_interval);

                assert(r.bwt != nullptr);
                // r.sample_marks_on_text.clear();
                // r.sample_marks_on_sa.clear();

                {
                    std::vector<bool> _sample_marks_on_text;
                    std::vector<bool> _sample_marks_on_sa;
                    DynamicSampledSA::build_bits(isa, sampling_interval, _sample_marks_on_text, _sample_marks_on_sa, stool::Message::increment_paragraph_level(message_paragraph));

                    auto tmp1 = stool::bptree::SimpleDynamicBitSequence::build(_sample_marks_on_text);
                    auto tmp2 = stool::bptree::SimpleDynamicBitSequence::build(_sample_marks_on_sa);

                    r.sample_marks_on_text.swap(tmp1);
                    r.sample_marks_on_sa.swap(tmp2);
                }

                std::vector<uint64_t> output_sampled_sa_rank;
                std::vector<uint64_t> output_sampled_sa;
                r.dp.clear();
                DynamicSampledSA::build_sample_sa(isa, sampling_interval, output_sampled_sa_rank, output_sampled_sa, stool::Message::increment_paragraph_level(message_paragraph));
                r.dp.build(output_sampled_sa_rank.begin(), output_sampled_sa_rank.end(), output_sampled_sa_rank.size(), stool::Message::increment_paragraph_level(message_paragraph));

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
             * @brief Save the given instance \p item to a file stream \p os
             */
            static void store_to_file(DynamicSampledSA &item, std::ofstream &os)
            {
                stool::bptree::DynamicPermutation::store_to_file(item.dp, os);
                stool::bptree::SimpleDynamicBitSequence::store_to_file(item.sample_marks_on_text, os);
                stool::bptree::SimpleDynamicBitSequence::store_to_file(item.sample_marks_on_sa, os);
                os.write(reinterpret_cast<const char *>(&item.sampling_interval), sizeof(item.sampling_interval));
            }
            /**
             * @brief Return the DynamicSampledSA instance loaded from a file stream \p ifs
             */
            static DynamicSampledSA load_from_file(std::ifstream &ifs, DynamicBWT *bwt)
            {
                auto tmp1 = stool::bptree::DynamicPermutation::load_from_file(ifs);
                auto tmp2 = stool::bptree::SimpleDynamicBitSequence::load_from_file(ifs);
                auto tmp3 = stool::bptree::SimpleDynamicBitSequence::load_from_file(ifs);
                uint64_t tmp4 = 0;
                ifs.read(reinterpret_cast<char *>(&tmp4), sizeof(uint64_t));
                DynamicSampledSA r;
                r.dp.swap(tmp1);
                r.sample_marks_on_text.swap(tmp2);
                r.sample_marks_on_sa.swap(tmp3);
                std::swap(r.sampling_interval, tmp4);
                r.bwt = bwt;
                return r;
            }

            //@}

        private:
            /**
             * @brief Builds sampled SA and bits from BWT.
             * @param BISA The BackwardISA object.
             * @param text_size The size of the text.
             * @param sampling_interval The sampling interval.
             * @param sample_marks_on_text The sample marks on text.
             * @param sample_marks_on_sa The sample marks on SA.
             * @param output_sampled_sa_rank The output sampled SA rank.
             * @param output_sampled_sa The output sampled SA.
             * @param message_paragraph The message paragraph.
             */
            static void build_sampled_sa_and_bits(const stool::bwt::BackwardISA<stool::bwt::LFDataStructure> &BISA, uint64_t text_size, uint64_t sampling_interval, std::vector<bool> &sample_marks_on_text, std::vector<bool> &sample_marks_on_sa,
                                                  std::vector<uint64_t> &output_sampled_sa_rank, std::vector<uint64_t> &output_sampled_sa,
                                                  int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing arrays for Dynamic Sampled SA from BWT... " << std::endl;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t output_size = 0;
                if (text_size <= 2)
                {
                    output_size = text_size;
                }
                else
                {
                    output_size = ((text_size - 2) / sampling_interval) + 2;
                }

                sample_marks_on_text.resize(text_size, false);
                sample_marks_on_sa.resize(text_size, false);

                std::vector<uint64_t> tmp;
                tmp.resize(output_size, UINT64_MAX);

                uint64_t message_counter = 10000000;
                uint64_t processed_text_length = 0;

                output_sampled_sa.resize(output_size, UINT64_MAX);
                {
                    std::vector<uint64_t> tmp2;
                    tmp2.resize(output_size, UINT64_MAX);

                    int64_t tpos = text_size - 1;
                    int64_t sampling_counter = 0;
                    int64_t p = output_size - 1;
                    for (auto it : BISA)
                    {
                        message_counter++;
                        processed_text_length++;
                        if (message_paragraph >= 0 && message_counter > 10000000)
                        {

                            std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Processing...(1/3) [" << (processed_text_length / 1000000) << "/" << (text_size / 1000000) << "MB] \r" << std::flush;
                            message_counter = 0;
                        }

                        if (sampling_counter == 0 || tpos == 0)
                        {
                            sample_marks_on_text[tpos] = true;
                            sample_marks_on_sa[it] = true;

                            tmp[p] = tpos;
                            output_sampled_sa[p] = p;
                            tmp2[p] = it;
                            sampling_counter = sampling_interval - 1;
                            p--;
                        }
                        else
                        {
                            sampling_counter--;
                        }
                        tpos--;
                    }
                    if (message_paragraph >= 0)
                    {
                        std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Processed(1/3) [" << (processed_text_length / 1000000) << "/" << (text_size / 1000000) << "MB] \r" << std::endl;
                    }

                    if (message_paragraph >= 0 && text_size > 0)
                    {
                        std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Processing...(2/3) " << std::flush;
                    }

                    int64_t x = text_size - 1;
                    for (int64_t i = output_size - 1; i >= 0; i--)
                    {
                        tmp[i] = x >= 0 ? x : 0;
                        output_sampled_sa[i] = i;
                        x -= sampling_interval;
                    }
                    std::sort(output_sampled_sa.begin(), output_sampled_sa.end(), [&](const uint64_t &lhs, const uint64_t &rhs)
                              { return tmp2[lhs] < tmp2[rhs]; });
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output_sampled_sa[i] = tmp[output_sampled_sa[i]];
                    }
                    if (message_paragraph >= 0 && text_size > 0)
                    {
                        std::cout << std::endl;

                        std::cout << "[END]" << std::endl;
                    }
                }

                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Processing...(3/3) " << std::flush;
                }

                output_sampled_sa_rank.resize(output_size, UINT64_MAX);
                {
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        tmp[i] = i;
                    }
                    std::sort(tmp.begin(), tmp.end(), [&](const uint64_t &lhs, const uint64_t &rhs)
                              { return output_sampled_sa[lhs] < output_sampled_sa[rhs]; });
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output_sampled_sa_rank[tmp[i]] = i;
                    }
                }
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << "[END]" << std::endl;
                }

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
             * @brief Builds bits from BWT.
             * @param BISA The BackwardISA object.
             * @param text_size The size of the text.
             * @param sampling_interval The sampling interval.
             * @param sample_marks_on_text The sample marks on text.
             * @param sample_marks_on_sa The sample marks on SA.
             * @param message_paragraph The message paragraph.
             */
            static void build_bits(const stool::bwt::BackwardISA<stool::bwt::LFDataStructure> &BISA, uint64_t text_size, uint64_t sampling_interval, std::vector<bool> &sample_marks_on_text, std::vector<bool> &sample_marks_on_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {

                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing bits for Dynamic Sampled SA from BWT... " << std::flush;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                sample_marks_on_text.resize(text_size, false);
                sample_marks_on_sa.resize(text_size, false);

                int64_t tpos = text_size - 1;
                int64_t sampling_counter = 0;
                for (auto it : BISA)
                {
                    if (sampling_counter == 0 || tpos == 0)
                    {
                        sample_marks_on_text[tpos] = true;
                        sample_marks_on_sa[it] = true;
                        sampling_counter = sampling_interval - 1;
                    }
                    else
                    {
                        sampling_counter--;
                    }
                    tpos--;
                }

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }
            /**
             * @brief Builds sample SA from ISA.
             * @param isa The ISA vector.
             * @param sampling_interval The sampling interval.
             * @param output_sampled_sa_rank The output sampled SA rank.
             * @param output_sampled_sa The output sampled SA.
             * @param message_paragraph The message paragraph.
             */
            static void build_sample_sa(const std::vector<uint64_t> &isa, uint64_t sampling_interval, std::vector<uint64_t> &output_sampled_sa_rank, std::vector<uint64_t> &output_sampled_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = isa.size();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Sampled Suffix Array... " << std::flush;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t output_size = 0;
                if (text_size <= 2)
                {
                    output_size = text_size;
                }
                else
                {
                    output_size = ((text_size - 2) / sampling_interval) + 2;
                }

                std::vector<uint64_t> tmp;
                tmp.resize(output_size, UINT64_MAX);

                output_sampled_sa.resize(output_size, UINT64_MAX);
                {

                    int64_t x = text_size - 1;
                    for (int64_t i = output_size - 1; i >= 0; i--)
                    {
                        tmp[i] = x >= 0 ? x : 0;
                        output_sampled_sa[i] = i;
                        x -= sampling_interval;
                    }
                    std::sort(output_sampled_sa.begin(), output_sampled_sa.end(), [&](const uint64_t &lhs, const uint64_t &rhs)
                              { return isa[tmp[lhs]] < isa[tmp[rhs]]; });
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output_sampled_sa[i] = tmp[output_sampled_sa[i]];
                    }
                }

                output_sampled_sa_rank.resize(output_size, UINT64_MAX);
                {
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        tmp[i] = i;
                    }
                    std::sort(tmp.begin(), tmp.end(), [&](const uint64_t &lhs, const uint64_t &rhs)
                              { return output_sampled_sa[lhs] < output_sampled_sa[rhs]; });
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output_sampled_sa_rank[tmp[i]] = i;
                    }
                }

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }
            /**
             * @brief Builds bits from ISA.
             * @param isa The ISA vector.
             * @param sampling_interval The sampling interval.
             * @param sample_marks_on_text The sample marks on text.
             * @param sample_marks_on_sa The sample marks on SA.
             * @param message_paragraph The message paragraph.
             */
            static void build_bits(const std::vector<uint64_t> &isa, uint64_t sampling_interval, std::vector<bool> &sample_marks_on_text, std::vector<bool> &sample_marks_on_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                uint64_t text_size = isa.size();
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing bits for Dynamic Sampled SA from ISA... " << std::flush;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t output_size = 0;
                if (text_size <= 2)
                {
                    output_size = text_size;
                }
                else
                {
                    output_size = ((text_size - 2) / sampling_interval) + 2;
                }

                sample_marks_on_text.resize(text_size, false);
                sample_marks_on_sa.resize(text_size, false);

                int64_t x = text_size - 1;
                for (int64_t i = output_size - 1; i >= 0; i--)
                {
                    uint64_t p = x >= 0 ? x : 0;
                    sample_marks_on_text[p] = true;
                    sample_marks_on_sa[isa[p]] = true;
                    x -= sampling_interval;
                }

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }

            /**
             * @brief Builds sample SA from BWT.
             * @param BISA The BackwardISA object.
             * @param text_size The size of the text.
             * @param sampling_interval The sampling interval.
             * @param output_sampled_sa_rank The output sampled SA rank.
             * @param output_sampled_sa The output sampled SA.
             * @param message_paragraph The message paragraph.
             */
            static void build_sample_sa(const stool::bwt::BackwardISA<stool::bwt::LFDataStructure> &BISA, uint64_t text_size, uint64_t sampling_interval, std::vector<uint64_t> &output_sampled_sa_rank, std::vector<uint64_t> &output_sampled_sa, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                if (message_paragraph >= 0 && text_size > 0)
                {
                    std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Constructing Sampled Suffix Array... " << std::flush;
                }
                std::chrono::system_clock::time_point st1, st2;
                st1 = std::chrono::system_clock::now();

                uint64_t output_size = 0;
                if (text_size <= 2)
                {
                    output_size = text_size;
                }
                else
                {
                    output_size = ((text_size - 2) / sampling_interval) + 2;
                }

                std::vector<uint64_t> tmp;
                tmp.resize(output_size, UINT64_MAX);

                output_sampled_sa.resize(output_size, UINT64_MAX);
                {
                    std::vector<uint64_t> tmp2;
                    tmp2.resize(output_size, UINT64_MAX);

                    int64_t tpos = text_size - 1;
                    int64_t sampling_counter = 0;
                    int64_t p = output_size - 1;
                    for (auto it : BISA)
                    {
                        if (sampling_counter == 0 || tpos == 0)
                        {
                            tmp[p] = tpos;
                            output_sampled_sa[p] = p;
                            tmp2[p] = it;
                            sampling_counter = sampling_interval - 1;
                            p--;
                        }
                        else
                        {
                            sampling_counter--;
                        }
                        tpos--;
                    }

                    int64_t x = text_size - 1;
                    for (int64_t i = output_size - 1; i >= 0; i--)
                    {
                        tmp[i] = x >= 0 ? x : 0;
                        output_sampled_sa[i] = i;
                        x -= sampling_interval;
                    }
                    std::sort(output_sampled_sa.begin(), output_sampled_sa.end(), [&](const uint64_t &lhs, const uint64_t &rhs)
                              { return tmp2[lhs] < tmp2[rhs]; });
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output_sampled_sa[i] = tmp[output_sampled_sa[i]];
                    }
                }

                output_sampled_sa_rank.resize(output_size, UINT64_MAX);
                {
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        tmp[i] = i;
                    }
                    std::sort(tmp.begin(), tmp.end(), [&](const uint64_t &lhs, const uint64_t &rhs)
                              { return output_sampled_sa[lhs] < output_sampled_sa[rhs]; });
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output_sampled_sa_rank[tmp[i]] = i;
                    }
                }

                st2 = std::chrono::system_clock::now();

                if (message_paragraph >= 0 && text_size > 0)
                {
                    uint64_t sec_time = std::chrono::duration_cast<std::chrono::seconds>(st2 - st1).count();
                    uint64_t ms_time = std::chrono::duration_cast<std::chrono::milliseconds>(st2 - st1).count();
                    uint64_t per_time = ((double)ms_time / (double)text_size) * 1000000;

                    std::cout << "[END] Elapsed Time: " << sec_time << " sec (" << per_time << " ms/MB)" << std::endl;
                }
            }

        private:
            /**
             * @brief Returns the SA value at rank and distance.
             * @param rank The rank.
             * @param dist The distance.
             * @return The SA value.
             */
            int64_t sa(uint64_t rank, uint64_t dist) const
            {
                assert(this->bwt != nullptr);
                assert(rank < this->dp.size());

                uint64_t p = this->sampled_sa(rank) + dist;
                uint64_t _size = this->size();
                if (p >= _size)
                {
                    p -= _size;
                }
                return p;
            }

            /**
             * @brief Return the pair (x, y) of (i) the number \p x of sampled SA values in SA[0..LF^{d}(i)-1] and (ii) the smallest value \p d such that SA[LF^{d}(i)] is a sampled SA value.
             * @note O(d (log n + LF)) time, where LF is the time complexity of the LF function, and d <= k
             */
            std::pair<uint64_t, uint64_t> sample_predecessor_on_sa(uint64_t i) const
            {
                assert(this->bwt != nullptr);

                uint64_t predecessor_rank = 0;
                uint64_t dist = 0;

                bool b = this->sample_marks_on_sa[i];
                uint64_t sa_pos = i;
                while (!b)
                {

                    assert(sa_pos < this->size());

                    sa_pos = this->bwt->LF(sa_pos);
                    dist++;
                    b = this->sample_marks_on_sa[sa_pos];

                    assert(dist <= this->size());
                }
                predecessor_rank = this->sample_marks_on_sa.one_based_rank1(sa_pos);

                assert(predecessor_rank < this->dp.size());

                return std::pair<uint64_t, uint64_t>(predecessor_rank, dist);
            }

            /**
             * @brief Updates sample marks for a given text position rank.
             * @param t_pos_rank The text position rank.
             */
            void update_sample_marks_sub(uint64_t t_pos_rank)
            {
                int64_t current_t_pos_rank = t_pos_rank;
                uint64_t t_pos = this->sample_marks_on_text.select1(current_t_pos_rank);

                uint64_t dist_min_threshold = this->sampling_interval / 2;
                uint64_t dist_max_threshold = this->sampling_interval + dist_min_threshold;

                while (current_t_pos_rank > 0)
                {
                    uint64_t next_t_pos = this->sample_marks_on_text.select1(current_t_pos_rank - 1);
                    uint64_t dist = t_pos - next_t_pos;

                    if (dist < dist_min_threshold)
                    {
                        uint64_t next_sample_sa_rank = this->dp.inverse(current_t_pos_rank - 1);
                        uint64_t next_sa_pos = this->sample_marks_on_sa.select1(next_sample_sa_rank);
                        assert(this->sample_marks_on_text[next_t_pos]);
                        assert(this->sample_marks_on_sa[next_sa_pos]);

                        this->sample_marks_on_text.set_bit(next_t_pos, false);
                        this->sample_marks_on_sa.set_bit(next_sa_pos, false);
                        this->dp.erase(next_sample_sa_rank);
                        current_t_pos_rank--;
                    }
                    else if (dist > dist_max_threshold)
                    {
                        uint64_t mid_t_pos = t_pos - this->sampling_interval;
                        uint64_t mid_sa_pos = this->isa(mid_t_pos);
                        assert(!this->sample_marks_on_sa[mid_sa_pos]);
                        uint64_t mid_sample_sa_rank = this->sample_marks_on_sa.one_based_rank1(mid_sa_pos);
                        int64_t mid_t_pos_rank = current_t_pos_rank;
                        this->dp.insert(mid_sample_sa_rank, mid_t_pos_rank);
                        assert(!this->sample_marks_on_text[mid_t_pos]);
                        assert(!this->sample_marks_on_sa[mid_sa_pos]);
                        this->sample_marks_on_text.set_bit(mid_t_pos, true);
                        this->sample_marks_on_sa.set_bit(mid_sa_pos, true);

                        t_pos = mid_t_pos;
                    }
                    else
                    {
                        break;
                    }
                }
                if (current_t_pos_rank == 0 && t_pos > 0)
                {
                    uint64_t sa_pos0 = this->isa(0);
                    uint64_t sample_sa_rank0 = this->sample_marks_on_sa.one_based_rank1(sa_pos0);

                    this->dp.insert(sample_sa_rank0, 0);
                    assert(!this->sample_marks_on_text[0]);
                    this->sample_marks_on_text.set_bit(0, true);
                    assert(!this->sample_marks_on_sa[sa_pos0]);
                    this->sample_marks_on_sa.set_bit(sa_pos0, true);
                }
            }
            /**
             * @brief Moves the pi index from one rank to another.
             * @param from_rank The source rank.
             * @param to_rank The destination rank.
             */
            void move_pi_index2(int64_t from_rank, int64_t to_rank)
            {
                int64_t inverse_pi_index = this->dp.access(from_rank);
                if (from_rank < to_rank)
                {
                    this->dp.insert(to_rank, inverse_pi_index);
                    this->dp.erase(from_rank);
                }
                else if (from_rank > to_rank)
                {
                    this->dp.erase(from_rank);
                    this->dp.insert(to_rank, inverse_pi_index);
                }
            }
            /**
             * @brief Move the bit \p B[from] to a given position \p to in \p B
             */
            void move_sa_bit(uint64_t from, uint64_t to)
            {
                bool b = this->sample_marks_on_sa[from];
                if (from < to)
                {
                    this->sample_marks_on_sa.insert(to + 1, b);
                    this->sample_marks_on_sa.remove(from);
                }
                else if (from > to)
                {
                    this->sample_marks_on_sa.remove(from);
                    this->sample_marks_on_sa.insert(to, b);
                }
            }
            /**
             * @brief Sets the sampling interval \p k.
             */
            void set_sampling_interval(uint64_t _k)
            {
                if (this->sampling_interval != _k)
                {
                    this->sampling_interval = _k;
                }
            }
        };

    }
}

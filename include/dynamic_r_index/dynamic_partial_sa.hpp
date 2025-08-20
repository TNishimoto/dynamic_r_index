#pragma once
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        ////////////////////////////////////////////////////////////////////////////////
        /// @class      DynamicPartialSA
        /// @brief      A dynamic data structure for managing sampled suffix array values
        /// @details    Memory: $O(r)$ words for $r$ sampled suffix array values.
        ///
        ////////////////////////////////////////////////////////////////////////////////
        class DynamicPartialSA
        {
        public:
            stool::bptree::DynamicPermutation pom;
            stool::bptree::SimpleDynamicPrefixSum sampled_isa_gap_vector;

            uint64_t _text_size = 0;

            DynamicPartialSA()
            {
                this->clear();
            }
            DynamicPartialSA &operator=(const DynamicPartialSA &) = delete;
            DynamicPartialSA(DynamicPartialSA &&) noexcept = default;
            DynamicPartialSA &operator=(DynamicPartialSA &&) noexcept = default;

            void print_statistics(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Statistics(DynamicFMIndex):" << std::endl;
                this->pom.print_statistics(message_paragraph + 1);
                this->sampled_isa_gap_vector.print_statistics(message_paragraph + 1);
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }
            void print_content(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                auto sa_values = this->get_sampled_sa_values();
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "Content(DynamicFMIndex):" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << std::flush;
                stool::DebugPrinter::print_integers(sa_values, "sampled_sa_values");
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "[END]" << std::endl;
            }

            static void save(DynamicPartialSA &item, std::ofstream &os)
            {
                stool::bptree::DynamicPermutation::save(item.pom, os);
                stool::bptree::SimpleDynamicPrefixSum::save(item.sampled_isa_gap_vector, os);

                os.write(reinterpret_cast<const char *>(&item._text_size), sizeof(uint64_t));
            }
            static DynamicPartialSA build_from_data(std::ifstream &ifs)
            {
                stool::bptree::DynamicPermutation tmp_pom = stool::bptree::DynamicPermutation::build_from_data(ifs);
                stool::bptree::SimpleDynamicPrefixSum tmp_sampled_isa_gap_vector = stool::bptree::SimpleDynamicPrefixSum::build_from_data(ifs);

                uint64_t _text_size = 0;
                ifs.read(reinterpret_cast<char *>(&_text_size), sizeof(uint64_t));

                DynamicPartialSA r;
                r.pom.swap(tmp_pom);
                r.sampled_isa_gap_vector.swap(tmp_sampled_isa_gap_vector);
                std::swap(r._text_size, _text_size);

                return r;
            }

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Initializers
            ///   @param degree The degree of the B-tree used in this data structure
            ///   Initializers
            ////////////////////////////////////////////////////////////////////////////////
            //@{
            /*
            void set_degree(uint64_t degree)
            {
                //this->pom.set_degree(degree);
                this->clear();
            }
            */
            /**
             * @brief Build this data structure for a string of length 1.
             */
            void build_for_single_character()
            {
                this->pom.insert(0, 0);
                this->_text_size = 1;
            }

            stool::bptree::DynamicPermutation &get_dynamic_permutation()
            {
                return this->pom;
            }
            stool::bptree::SimpleDynamicPrefixSum &get_spsi()
            {
                return this->sampled_isa_gap_vector;
            }
            /**
             * @brief Initialize this data structure.
             */
            void clear()
            {
                this->pom.clear();
                this->sampled_isa_gap_vector.clear();
                this->_text_size = 1;

                this->pom.insert(0, 0);
                this->sampled_isa_gap_vector.push_back(0);

            }
            void swap(DynamicPartialSA &item)
            {
                this->pom.swap(item.pom);
                this->sampled_isa_gap_vector.swap(item.sampled_isa_gap_vector);
                std::swap(this->_text_size, item._text_size);

            }

            /**
             * @brief Set the size of the input text
             * @param new_text_size The new size of the input text
             */
            void set_text_size(uint64_t new_text_size)
            {
                this->_text_size = new_text_size;
            }

            /**
             * @brief Build this data structure
             * @param sampled_sa_indexes The sampled suffix array values indexed by this data structure
             * @param new_text_size The new size of the input text
             */
            static DynamicPartialSA build_from_sampled_sa_indexes(const std::vector<uint64_t> &sampled_sa_indexes, uint64_t new_text_size, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                DynamicPartialSA r;

                r._text_size = new_text_size;
                r.sampled_isa_gap_vector.clear();
                r.pom.clear();

                // PackedSPSIWrapper::clear(this->sampled_isa_gap_vector);

                std::vector<uint64_t> idx_vec;
                idx_vec.resize(sampled_sa_indexes.size(), UINT64_MAX);
                for (int64_t i = 0; i < (int64_t)sampled_sa_indexes.size(); i++)
                {
                    idx_vec[i] = i;
                }
                std::sort(idx_vec.begin(), idx_vec.end(), [&](uint64_t lhs, const uint64_t rhs)
                          {
                    if(sampled_sa_indexes[lhs] != sampled_sa_indexes[rhs]){
                        return sampled_sa_indexes[lhs] < sampled_sa_indexes[rhs];
                    }else{
                        return lhs < rhs;
                    } });

                if (idx_vec.size() > 0)
                {
                    r.sampled_isa_gap_vector.push_back(sampled_sa_indexes[idx_vec[0]]);

                }

                for (int64_t i = 1; i < (int64_t)idx_vec.size(); i++)
                {
                    int64_t idx1 = idx_vec[i];
                    int64_t idx2 = idx_vec[i - 1];
                    r.sampled_isa_gap_vector.push_back(sampled_sa_indexes[idx1] - sampled_sa_indexes[idx2]);

                }

                std::vector<uint64_t> ranked_samp_sa;
                ranked_samp_sa.resize(sampled_sa_indexes.size(), UINT64_MAX);

                for (int64_t i = 0; i < (int64_t)sampled_sa_indexes.size(); i++)
                {
                    ranked_samp_sa[idx_vec[i]] = i;
                }

                r.pom.clear();
                r.pom.build(ranked_samp_sa.begin(), ranked_samp_sa.end(), ranked_samp_sa.size(), stool::Message::add_message_paragraph(message_paragraph));
                return r;
            }
            void build(const std::vector<uint64_t> &sampled_sa_indexes, uint64_t new_text_size, int message_paragraph = stool::Message::SHOW_MESSAGE)
            {
                this->_text_size = new_text_size;
                this->sampled_isa_gap_vector.clear();

                // PackedSPSIWrapper::clear(this->sampled_isa_gap_vector);

                std::vector<uint64_t> idx_vec;
                idx_vec.resize(sampled_sa_indexes.size(), UINT64_MAX);
                for (int64_t i = 0; i < (int64_t)sampled_sa_indexes.size(); i++)
                {
                    idx_vec[i] = i;
                }
                std::sort(idx_vec.begin(), idx_vec.end(), [&](uint64_t lhs, const uint64_t rhs)
                          {
                    if(sampled_sa_indexes[lhs] != sampled_sa_indexes[rhs]){
                        return sampled_sa_indexes[lhs] < sampled_sa_indexes[rhs];
                    }else{
                        return lhs < rhs;
                    } });

                if (idx_vec.size() > 0)
                {
                    sampled_isa_gap_vector.push_back(sampled_sa_indexes[idx_vec[0]]);

                }

                for (int64_t i = 1; i < (int64_t)idx_vec.size(); i++)
                {
                    int64_t idx1 = idx_vec[i];
                    int64_t idx2 = idx_vec[i - 1];
                    sampled_isa_gap_vector.push_back(sampled_sa_indexes[idx1] - sampled_sa_indexes[idx2]);

                }
                // PackedSPSIWrapper::push_dummy_values(this->sampled_isa_gap_vector);
                // assert(PackedSPSIWrapper::verify(this->sampled_isa_gap_vector, true));

                /*
                if (idx_vec.size() > 0)
                {
                    sampled_isa_gap_vector.push_back(this->_text_size - sampled_sa_indexes[idx_vec[idx_vec.size() - 1]]);
                }
                */

                std::vector<uint64_t> ranked_samp_sa;
                ranked_samp_sa.resize(sampled_sa_indexes.size(), UINT64_MAX);

                for (int64_t i = 0; i < (int64_t)sampled_sa_indexes.size(); i++)
                {
                    ranked_samp_sa[idx_vec[i]] = i;
                }

                /*
                stool::bptree::DynamicPermutation tmp_dp;
                tmp_dp.build(ranked_samp_sa.begin(), ranked_samp_sa.end(), ranked_samp_sa.size(), stool::Message::add_message_paragraph(message_paragraph));
                this->pom.swap(tmp_dp);
                */

                this->pom.clear();
                this->pom.build(ranked_samp_sa.begin(), ranked_samp_sa.end(), ranked_samp_sa.size(), stool::Message::add_message_paragraph(message_paragraph));
            }
            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Const Methods
            ///   The const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            int64_t get_sampled_isa_index(int64_t sampled_sa_index) const
            {
                assert(sampled_sa_index < this->size());

                return this->pom.access(sampled_sa_index);
            }
            int64_t get_sampled_isa_value(int64_t sampled_sa_index) const
            {
                assert(sampled_sa_index < this->size());

                return this->sampled_isa_gap_vector.psum(sampled_sa_index);
            }

            int64_t get_sampled_sa_index(int64_t sampled_isa_index) const
            {
                assert(sampled_isa_index < this->size());
                return this->pom.inverse(sampled_isa_index);
            }

            int64_t circular_successor_index_query_on_sampled_isa(int64_t isa_value) const
            {
                int64_t p = this->sampled_isa_gap_vector.successor_index(isa_value);
                if (p == -1)
                {
                    return 0;
                }
                else
                {
                    return p;
                }
            }
            int64_t circular_predecessor_index_query_on_sampled_isa(uint64_t i) const
            {
                int64_t size = this->size();
                int64_t idx = this->predecessor_index_query_on_sampled_isa(i);
                idx = idx != -1 ? idx : size - 1;
                return idx;
            }

            std::pair<uint64_t, uint64_t> nearest_search_by_text_position(uint64_t text_position) const
            {
                int64_t idx1 = this->circular_successor_index_query_on_sampled_isa(text_position);
                int64_t idx2 = idx1 > 0 ? idx1 - 1 : this->sampled_isa_gap_vector.size() - 1;

                uint64_t t1 = this->get_sampled_isa_value(idx1);
                uint64_t t2 = this->get_sampled_isa_value(idx2);

                uint64_t dist1 = t1 <= text_position ? text_position - t1 : t1 - text_position;
                uint64_t dist2 = t2 <= text_position ? text_position - t2 : t2 - text_position;
                if (dist1 <= dist2)
                {
                    return std::pair<uint64_t, uint64_t>(idx1, t1);
                }
                else
                {
                    return std::pair<uint64_t, uint64_t>(idx2, t2);
                }
            }

            int64_t get_sampled_sa_value(int64_t sampled_sa_index) const
            {
                assert(sampled_sa_index < this->size());
                int64_t isa_idx = this->pom.access(sampled_sa_index);
                return this->sampled_isa_gap_vector.psum(isa_idx);
            }
            int64_t predecessor_index_query_on_sampled_isa(int64_t isa_value) const
            {
                int64_t p = this->sampled_isa_gap_vector.predecessor_index(isa_value);

                // int64_t p = PackedSPSIWrapper::predecessor_index(this->sampled_isa_gap_vector, isa_value, true);
                assert(p < this->size());
                return p;
            }
            int64_t successor_index_query_on_sampled_isa(int64_t isa_value) const
            {
                int64_t p = this->sampled_isa_gap_vector.successor_index(isa_value);

                // int64_t p = PackedSPSIWrapper::successor_index(this->sampled_isa_gap_vector, isa_value, true);
                assert(p < this->size());
                return p;
            }

            bool verify() const
            {
                this->pom.verify();

                
                return true;
                // return PackedSPSIWrapper::verify(this->sampled_isa_gap_vector, true);
            }

            int64_t size() const
            {
                return this->sampled_isa_gap_vector.size();
                // return PackedSPSIWrapper::modified_size(this->sampled_isa_gap_vector);
            }

            void print() const
            {
                std::vector<uint64_t> gap_vector;
                std::vector<uint64_t> sa_vector;
                sa_vector.resize(this->size(), UINT64_MAX);
                uint64_t vec_size = this->sampled_isa_gap_vector.size();

                // uint64_t vec_size = PackedSPSIWrapper::modified_size(this->sampled_isa_gap_vector);

                for (int64_t i = 0; i < (int64_t)vec_size; i++)
                {
                    gap_vector.push_back(this->sampled_isa_gap_vector.at(i));
                    if (i < this->size())
                    {
                        int64_t idx = this->get_sampled_sa_index(i);
                        sa_vector[idx] = this->get_sampled_sa_value(idx);
                    }
                }
                stool::Printer::print("isa_gap_vector", gap_vector);
                stool::Printer::print("sampled_sa_vector", sa_vector);
                this->pom.print();
            }
            std::vector<uint64_t> get_sampled_sa_values() const
            {
                std::vector<uint64_t> sa;
                sa.resize(this->size(), UINT64_MAX);
                for (int64_t i = 0; i < this->size(); i++)
                {
                    sa[i] = this->get_sampled_sa_value(i);
                }
                return sa;
            }
            void print_detailed_info() const
            {
                this->pom.print();
                std::cout << this->sampled_isa_gap_vector.to_string() << std::endl;
            }

            //@}

            ////////////////////////////////////////////////////////////////////////////////
            ///   @name Non-const Methods
            ///   The non-const methods of this class.
            ////////////////////////////////////////////////////////////////////////////////
            //@{

            /**
             * @brief Insert a character into the input text at a position and appropriately shift the sampled suffix array values.
             * @param new_position A character is inserted into the input text at this position
             */
            void extend_text(int64_t new_position)
            {

                int64_t idx = this->sampled_isa_gap_vector.successor_index(new_position);

                if (idx != -1)
                {
                    this->sampled_isa_gap_vector.increment(idx, 1);
                }
                this->_text_size++;

                assert(this->verify());
            }

            /**
             * @brief Remove the character at a position from the input text and appropriately shift the sampled suffix array values.
             * @param removed_position The character at this position is removed from the input text
             */
            void shrink_text(int64_t removed_position)
            {
                int64_t idx = this->sampled_isa_gap_vector.successor_index(removed_position);

                if (idx != -1)
                {
                    uint64_t len = this->sampled_isa_gap_vector.at(idx);
                    if (len == 0)
                    {
                        throw std::logic_error("Error: shrink_text");
                    }
                    this->sampled_isa_gap_vector.decrement(idx, 1);

                }
                this->_text_size--;

                assert(this->verify());
            }

            /**
             * @brief Add a new sampled suffix array value into this data structure
             * @param new_sa_index the new suffix array value is inserted into this data structure at this position
             * @param new_sa_value the new suffix array value
             */
            void insert(int64_t new_sa_index, int64_t new_sa_value)
            {
                assert(new_sa_index <= (int64_t)this->pom.size());

                int64_t idx = this->sampled_isa_gap_vector.successor_index(new_sa_value);

                if (idx == -1)
                {
                    idx = this->sampled_isa_gap_vector.size();
                    assert(idx <= (int64_t)this->pom.size());

                    int64_t diff = new_sa_value - this->sampled_isa_gap_vector.psum();

                    this->sampled_isa_gap_vector.insert(idx, diff);

                    this->pom.insert(new_sa_index, idx);
                }
                else
                {
                    assert(idx <= (int64_t)this->pom.size());

                    int64_t v = this->sampled_isa_gap_vector.psum(idx);
                    int64_t diff = this->sampled_isa_gap_vector.at(idx);
                    int64_t diff2 = v - new_sa_value;
                    int64_t diff1 = diff - diff2;
                    this->sampled_isa_gap_vector.remove(idx);
                    this->sampled_isa_gap_vector.insert(idx, diff1);
                    this->sampled_isa_gap_vector.insert(idx + 1, diff2);

                    this->pom.insert(new_sa_index, idx);
                }

                assert(this->verify());
            }

            /**
             * @brief Remove the sampled suffix array value at the selected index from this data structure
             * @param sa_index The index of the removed sampled suffix array value
             */
            void remove(int64_t sa_index)
            {
                assert(this->verify());
                int64_t isa_index = this->get_sampled_isa_index(sa_index);
                if (isa_index + 1 < this->size())
                {
                    int64_t diff1 = this->sampled_isa_gap_vector.at(isa_index);
                    int64_t diff2 = this->sampled_isa_gap_vector.at(isa_index + 1);
                    this->sampled_isa_gap_vector.remove(isa_index + 1);
                    this->sampled_isa_gap_vector.remove(isa_index);

                    this->sampled_isa_gap_vector.insert(isa_index, diff1 + diff2);

                }
                else
                {
                    this->sampled_isa_gap_vector.remove(isa_index);

                }

                this->pom.erase(sa_index);
                assert(this->verify());
            }

            /**
             * @brief Remove the sampled suffix array value at the selected index with a new suffix array value
             * @param sa_index The index of the replaced sampled suffix array value
             * @param new_sa_value The new sampled suffix array value at the selected index
             */
            void replace(SampledSAIndex sa_index, SAValue new_sa_value)
            {
                this->remove(sa_index);
                this->insert(sa_index, new_sa_value);

                assert(this->verify());
            }
            uint64_t size_in_bytes() const
            {
                return this->pom.size_in_bytes() + this->sampled_isa_gap_vector.size_in_bytes();
            }
            std::vector<std::string> get_memory_usage_info(int message_paragraph = stool::Message::SHOW_MESSAGE) const
            {
                std::vector<std::string> log1 = this->pom.get_memory_usage_info(message_paragraph + 1);
                std::vector<std::string> log2 = this->sampled_isa_gap_vector.get_memory_usage_info(message_paragraph + 1);

                uint64_t total_bytes = this->size_in_bytes();
                uint64_t run_count = this->sampled_isa_gap_vector.size();
                uint64_t byte_per_run = total_bytes / run_count;

                std::vector<std::string> r;
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + "=Dynamic Sampled SA: " + std::to_string(total_bytes) + " bytes (" + std::to_string(byte_per_run) + " bytes per run in BWT) =");
                r.push_back(stool::Message::get_paragraph_string(message_paragraph) + " The number of runs: " + std::to_string(run_count));

                for (std::string &s : log1)
                {
                    r.push_back(" " + s);
                }
                for (std::string &s : log2)
                {
                    r.push_back(" " + s);
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

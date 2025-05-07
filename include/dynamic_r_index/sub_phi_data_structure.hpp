#pragma once
#include "../dynamic_fm_index/dynamic_bwt.hpp"
#include "../dynamic_fm_index/dynamic_isa.hpp"
#include "./dynamic_rlbwt_helper.hpp"
#include "./dynamic_phi.hpp"
#include "./sampling_sa_builder.hpp"
#include "./rlbwt_builder.hpp"

#include "./extended_spsi.hpp"

namespace stool
{
    namespace dynamic_r_index
    {

        class SubPhiDataStructure
        {
            std::map<uint64_t, uint64_t> phi_mapper;
            std::map<uint64_t, uint64_t> inverse_phi_mapper;

            int64_t first_inserted_character_position = -1;
            int64_t last_removed_character_position = -1;
            uint64_t current_inserted_or_removed_character_count = 0;
            uint64_t max_inserted_or_removed_character_count = 0;
            bool is_insertion_mode = false;

            static inline constexpr std::pair<uint64_t, uint64_t> NOT_FOUND = std::pair<uint64_t, uint64_t>(UINT64_MAX, UINT64_MAX);

        public:
            SubPhiDataStructure(uint64_t first_inserted_or_removed_character_position, uint64_t inserted_or_removed_string_length, bool _is_insertion_mode)
            {
                this->is_insertion_mode = _is_insertion_mode;
                this->max_inserted_or_removed_character_count = inserted_or_removed_string_length;

                if (_is_insertion_mode)
                {
                    this->first_inserted_character_position = first_inserted_or_removed_character_position;
                }
                else
                {
                    this->last_removed_character_position = first_inserted_or_removed_character_position + inserted_or_removed_string_length - 1;
                }
            }

        private:
            /*
            int64_t get_next_inserted_position() const
            {
                if (this->is_insertion_mode)
                {
                    return (this->first_inserted_character_position + this->max_inserted_or_removed_character_count - 1) - this->current_inserted_or_removed_character_count;
                }
                else
                {
                    return -1;
                }
            }
            */
            int64_t get_next_inserted_position_on_final_text() const
            {
                if (this->is_insertion_mode)
                {
                    return (this->first_inserted_character_position + this->max_inserted_or_removed_character_count - 1) - this->current_inserted_or_removed_character_count;
                }
                else
                {
                    return -1;
                }
            }

            int64_t get_next_removed_position() const
            {
                if (!this->is_insertion_mode)
                {
                    return this->last_removed_character_position - (int64_t)this->current_inserted_or_removed_character_count;
                }
                else
                {
                    return -1;
                }
            }

            static std::pair<uint64_t, uint64_t> predecessor_on_map(uint64_t key, const std::map<uint64_t, uint64_t> &mapper)
            {
                if (mapper.size() == 0)
                {
                    return NOT_FOUND;
                }
                auto it = mapper.lower_bound(key);

                // v 以下の要素を探す
                if (it == mapper.begin())
                {
                    if (it->first > key)
                    {
                        return NOT_FOUND;
                    }
                    else
                    {
                        return std::pair<uint64_t, uint64_t>(it->first, it->second);
                    }
                }
                else
                {
                    if (it == mapper.end() || it->first > key)
                    {
                        --it;
                    }
                    return std::pair<uint64_t, uint64_t>(it->first, it->second);
                }
            }
            uint64_t get_sa_value_of_initial_text(uint64_t sa_value) const
            {
                if (!this->is_insertion_mode)
                {
                    if ((int64_t)sa_value <= this->get_next_removed_position())
                    {
                        return sa_value;
                    }
                    else
                    {
                        return sa_value + this->current_inserted_or_removed_character_count;
                    }
                }
                else
                {
                    throw std::runtime_error("get_sa_value_of_initial_text");
                }
            }
            
            uint64_t get_sa_value_of_final_text(uint64_t sa_value) const
            {
                if (this->is_insertion_mode)
                {
                    if ((int64_t)sa_value < this->first_inserted_character_position)
                    {
                        return sa_value;
                    }
                    else
                    {
                        uint64_t diff = this->max_inserted_or_removed_character_count - this->current_inserted_or_removed_character_count;
                        return sa_value + diff;
                    }
                }
                else
                {
                    throw std::runtime_error("get_sa_value_of_final_text");
                }
            }
            
            uint64_t get_modified_sa_value(uint64_t sa_value) const
            {
                if (this->is_insertion_mode)
                {
                    return this->get_sa_value_of_final_text(sa_value);
                }
                else
                {
                    return this->get_sa_value_of_initial_text(sa_value);
                }
            }
            uint64_t get_current_sa_value(uint64_t modified_sa_value) const
            {
                if (this->is_insertion_mode)
                {
                    if (this->get_next_inserted_position_on_final_text() <= (int64_t)modified_sa_value)
                    {
                        uint64_t dist = this->max_inserted_or_removed_character_count - this->current_inserted_or_removed_character_count;
                        return modified_sa_value - dist;
                    }
                    else
                    {
                        return modified_sa_value;
                    }
                }
                else
                {
                    if (this->get_next_removed_position() < (int64_t)modified_sa_value)
                    {
                        uint64_t dist = this->current_inserted_or_removed_character_count;
                        return modified_sa_value - dist;
                    }
                    else
                    {
                        return modified_sa_value;
                    }
                }
            }

            void insert_sa_value_pair(uint64_t upper_sa_value, uint64_t lower_sa_value)
            {
                uint64_t modified_lower_sa_value = this->get_modified_sa_value(lower_sa_value);
                uint64_t modified_upper_sa_value = this->get_modified_sa_value(upper_sa_value);
                this->phi_mapper[modified_lower_sa_value] = modified_upper_sa_value;
                this->inverse_phi_mapper[modified_upper_sa_value] = modified_lower_sa_value;
            }
            void remove_sa_value_pair(uint64_t upper_sa_value, uint64_t lower_sa_value)
            {
                uint64_t modified_lower_sa_value = this->get_modified_sa_value(lower_sa_value);
                uint64_t modified_upper_sa_value = this->get_modified_sa_value(upper_sa_value);
                this->phi_mapper.erase(modified_lower_sa_value);
                this->inverse_phi_mapper.erase(modified_upper_sa_value);
            }

        public:
            void remove_sa_value(uint64_t upper_sa_value, uint64_t center_sa_value, uint64_t lower_sa_value)
            {

                this->remove_sa_value_pair(upper_sa_value, center_sa_value);
                this->remove_sa_value_pair(center_sa_value, lower_sa_value);
                this->insert_sa_value_pair(upper_sa_value, lower_sa_value);

                assert(this->phi_mapper.size() == this->inverse_phi_mapper.size());
            }
            void insert_sa_value(uint64_t upper_sa_value, uint64_t center_sa_value, uint64_t lower_sa_value)
            {


                this->remove_sa_value_pair(upper_sa_value, lower_sa_value);

                this->insert_sa_value_pair(upper_sa_value, center_sa_value);
                this->insert_sa_value_pair(center_sa_value, lower_sa_value);

                assert(this->phi_mapper.size() == this->inverse_phi_mapper.size());
            }

            void extend_text([[maybe_unused]] uint64_t new_position)
            {
                if (this->is_insertion_mode)
                {
                    this->current_inserted_or_removed_character_count++;
                }
                else
                {
                    throw std::runtime_error("Error:extend_text");
                }
            }

            void shrink_text([[maybe_unused]] uint64_t remove_position)
            {
                if (!this->is_insertion_mode)
                {
                    this->current_inserted_or_removed_character_count++;
                }
                else
                {
                    throw std::runtime_error("Error:shrink_text");
                }

            }

            uint64_t inverse_phi(uint64_t sa_value, const DynamicPhi &phi_ds) const
            {
                uint64_t text_size = phi_ds.text_size();
                std::pair<uint64_t, uint64_t> result = predecessor_on_map(this->get_modified_sa_value(sa_value), this->inverse_phi_mapper);

                if (result == NOT_FOUND)
                {
                    uint64_t result = phi_ds.inverse_phi(sa_value);
                    assert(result < text_size);
                    return result;
                }
                else
                {
                    // const DynamicPartialSA *sampled_first_sa = phi_ds.get_pointer_of_first_sa_values_of_BWT_run();
                    const DynamicPartialSA *sampled_last_sa = phi_ds.get_pointer_of_last_sa_values_of_BWT_run();

                    int64_t idx = sampled_last_sa->circular_predecessor_index_query_on_sampled_isa(sa_value);
                    uint64_t found_sa_value1 = this->get_current_sa_value(result.first);
                    int64_t found_sa_value = sampled_last_sa->get_sampled_isa_value(idx);

                    uint64_t dist1 = sa_value >= found_sa_value1 ? sa_value - found_sa_value1 : (sa_value + (text_size - found_sa_value1));
                    uint64_t dist2 = sa_value >= (uint64_t)found_sa_value ? sa_value - found_sa_value : (sa_value + (text_size - found_sa_value));


                    if (dist1 <= dist2)
                    {
                        uint64_t answer = dist1 + this->get_current_sa_value(result.second);
                        assert(answer < text_size);

                        return answer;
                    }
                    else
                    {
                        return phi_ds.inverse_phi(sa_value);
                    }
                }
            }

            uint64_t phi(uint64_t sa_value, const DynamicPhi &phi_ds) const
            {
                std::pair<uint64_t, uint64_t> result = predecessor_on_map(this->get_modified_sa_value(sa_value), this->phi_mapper);
                if (result == NOT_FOUND)
                {
                    return phi_ds.phi(sa_value);
                }
                else
                {
                    uint64_t text_size = phi_ds.text_size();
                    const DynamicPartialSA *sampled_first_sa = phi_ds.get_pointer_of_first_sa_values_of_BWT_run();
                    // const DynamicPartialSA *sampled_last_sa = phi_ds.get_pointer_of_last_sa_values_of_BWT_run();

                    int64_t idx = sampled_first_sa->circular_predecessor_index_query_on_sampled_isa(sa_value);
                    uint64_t found_sa_value1 = this->get_current_sa_value(result.first);
                    int64_t found_sa_value = sampled_first_sa->get_sampled_isa_value(idx);

                    uint64_t dist1 = sa_value >= found_sa_value1 ? sa_value - found_sa_value1 : (sa_value + (text_size - found_sa_value1));
                    uint64_t dist2 = sa_value >= (uint64_t)found_sa_value ? sa_value - found_sa_value : (sa_value + (text_size - found_sa_value));

                    if (dist1 <= dist2)
                    {
                        return dist1 + this->get_current_sa_value(result.second);
                    }
                    else
                    {
                        return phi_ds.phi(sa_value);
                    }
                }
            }
            std::vector<uint64_t> get_sa(const DynamicPhi &phi_ds) const
            {

                std::vector<uint64_t> sa;
                const DynamicPartialSA *sampled_first_sa = phi_ds.get_pointer_of_first_sa_values_of_BWT_run();
                int64_t sa_size = phi_ds.text_size();
                sa.resize(sa_size, UINT64_MAX);
                if (sa_size == 0)
                {
                    return sa;
                }

                uint64_t sa_value = sampled_first_sa->get_sampled_sa_value(0);

                sa[0] = sa_value;

                for (int64_t i = 1; i < sa_size; i++)
                {
                    sa_value = this->inverse_phi(sa_value, phi_ds);
                    sa[i] = sa_value;
                }

                return sa;
            }
            void verify(const DynamicPhi &phi_ds) const
            {
                std::vector<uint64_t> sa = this->get_sa(phi_ds);
                DynamicRLBWT::verify_permutation(sa, "SA");
            }

            void print(int message_paragraph = 0) const
            {
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "====== PhiSub ======" << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "Phi: " << std::flush;

                for (auto &it : this->phi_mapper)
                {
                    std::cout << "[" << this->get_current_sa_value(it.first) << ", " << this->get_current_sa_value(it.second) << "], " << std::flush;
                }
                std::cout << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph + 1) << "InvPhi: " << std::flush;

                for (auto &it : this->inverse_phi_mapper)
                {
                    std::cout << "[" << this->get_current_sa_value(it.first) << ", " << this->get_current_sa_value(it.second) << "], " << std::flush;
                }
                std::cout << std::endl;
                std::cout << stool::Message::get_paragraph_string(message_paragraph) << "====== PhiSub ======" << std::endl;
            }
        };
    }
}

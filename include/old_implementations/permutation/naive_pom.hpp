#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <list>
#include <iterator>
#include <algorithm>

namespace stool
{
    namespace old_implementations
    {
        class NaivePOM
        {
        public:
            std::vector<uint64_t> pi_list;
            std::vector<uint64_t> inverse_pi_list;
            std::vector<int64_t> value_list;

            int64_t size()
            {
                return this->pi_list.size();
            }

            void insert(int64_t pi_index, int64_t inverse_pi_index)
            {
                uint64_t size = this->pi_list.size();
                for (size_t i = 0; i < this->pi_list.size(); i++)
                {
                    if ((int64_t)this->pi_list[i] >= inverse_pi_index)
                    {
                        this->pi_list[i]++;
                    }
                }
                this->pi_list.insert(this->pi_list.begin() + pi_index, inverse_pi_index);
                this->value_list.insert(this->value_list.begin() + pi_index, size);

                for (size_t i = 0; i < this->inverse_pi_list.size(); i++)
                {
                    if ((int64_t)this->inverse_pi_list[i] >= pi_index)
                    {
                        this->inverse_pi_list[i]++;
                    }
                }
                this->inverse_pi_list.insert(this->inverse_pi_list.begin() + inverse_pi_index, pi_index);
            }
            void erase(int64_t pi_index)
            {
                int64_t inverse_pi_index = this->pi_list[pi_index];
                for (size_t i = 0; i < this->pi_list.size(); i++)
                {
                    if ((int64_t)this->pi_list[i] > inverse_pi_index)
                    {
                        this->pi_list[i]--;
                    }
                }
                this->pi_list.erase(this->pi_list.begin() + pi_index);
                this->value_list.erase(this->value_list.begin() + pi_index);

                for (size_t i = 0; i < this->inverse_pi_list.size(); i++)
                {
                    if ((int64_t)this->inverse_pi_list[i] > pi_index)
                    {
                        this->inverse_pi_list[i]--;
                    }
                }
                this->inverse_pi_list.erase(this->inverse_pi_list.begin() + inverse_pi_index);
            }
        };
    }
}

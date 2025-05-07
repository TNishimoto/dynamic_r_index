#pragma once
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        class ExtendedSPSI
        {
            //dyn::packed_spsi spsi;
            stool::bptree::VLCDequeDynamicPrefixSum spsi;

            public:

            void set_psum_value(uint64_t value)
            {
                uint64_t size = spsi.size();
                if (size > 0)
                {
                    uint64_t sum = spsi.psum();
                    if (sum < value)
                    {
                        spsi.push_back(value - sum);
                    }
                    else
                    {
                        uint64_t p = spsi.search(value);
                        uint64_t psum = spsi.psum(p);
                        if (psum > value)
                        {
                            uint64_t x = spsi.at(p);
                            uint64_t prev = psum - x;
                            uint64_t diff1 = value - prev;
                            uint64_t diff2 = psum - value;
                            spsi.remove(p);
                            spsi.insert(p, diff1);
                            spsi.insert(p + 1, diff2);
                        }
                    }
                }
                else
                {
                    spsi.push_back(value);
                }
                // uint64_t i = spsi.psum()
            }
            bool remove_psum_value(uint64_t value)
            {
                uint64_t size = spsi.size();
                if (size == 0)
                    return false;
                uint64_t sum = spsi.psum();
                if (sum < value)
                {
                    return false;
                }
                else
                {
                    uint64_t p = spsi.search(value);
                    uint64_t psum = spsi.psum(p);
                    if (psum == value)
                    {
                        if (p + 1 < size)
                        {
                            spsi.increment(p + 1, spsi.at(p));
                            spsi.remove(p);
                        }
                        else
                        {
                            spsi.remove(p);
                        }
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }

                // uint64_t i = spsi.psum()
            }

            void extend(uint64_t new_position)
            {
                uint64_t size = spsi.size();
                if (size == 0)
                    return;
                uint64_t sum = spsi.psum();
                if (new_position <= sum)
                {
                    uint64_t p = spsi.search(new_position);
                    spsi.increment(p, 1);
                }
            }
            void shrink(uint64_t old_position)
            {
                this->remove_psum_value(old_position);

                uint64_t size = spsi.size();
                if (size == 0)
                    return;
                uint64_t sum = spsi.psum();
                if (sum < old_position)
                    return;

                uint64_t p = spsi.search(old_position);
                spsi.decrement(p, 1);
            }

            uint64_t psum() const {
                return this->spsi.psum();
            }
            uint64_t at(uint64_t x) const {
                return this->spsi.at(x);
            }
            uint64_t size() const{
                return this->spsi.size();
            }
        };
    }
}

#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"
#include "include/dynamic_r_index_test.hpp"
//

#include <filesystem>

class TemporaryBWT
{
public:
    static uint64_t LF(const std::vector<uint8_t> &bwt, uint64_t i)
    {
        uint64_t x = 0;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            if (bwt[j] < bwt[i])
            {
                x++;
            }
            else if (bwt[j] == bwt[i] && j <= i)
            {
                x++;
            }
        }
        return x - 1;
    }
    static uint64_t special_LF(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c, uint64_t replaced_sa_index)
    {
        if (c < bwt[i] || ((c == bwt[i]) && (replaced_sa_index <= i)))
        {
            return LF(bwt, i) + 1;
        }
        else
        {
            return LF(bwt, i);
        }
    }
    static uint64_t LF_for_move_phase(const std::vector<uint8_t> &bwt, uint64_t i, uint64_t from, uint64_t to)
    {
        uint8_t c = bwt[to];
        if (c == bwt[i])
        {
            if (from < to)
            {
                if (from <= i && i < to)
                {
                    return LF(bwt, i) + 1;
                }
                else
                {
                    return LF(bwt, i);
                }
            }
            else
            {
                if (to < i && i <= from)
                {
                    return LF(bwt, i) - 1;
                }
                else
                {
                    return LF(bwt, i);
                }
            }
        }
        else
        {
            return LF(bwt, i);
        }
    }

    static uint64_t LF_for_insertion_phase(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c, uint64_t replaced_sa_index, uint64_t inserted_sa_index)
    {
        int64_t b1 = c < bwt[i] || ((c == bwt[i]) && (replaced_sa_index <= i)) ? 1 : 0;
        assert(inserted_sa_index < bwt.size());
        int64_t b2 = bwt[inserted_sa_index] < bwt[i] || ((bwt[inserted_sa_index] == bwt[i]) && (inserted_sa_index <= i)) ? -1 : 0;
        int64_t r = ((int64_t)LF(bwt, i)) + b1 + b2;
        assert(r >= 0);

        return r;
    }

    static uint64_t LF_for_insertion_last_phase(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c, uint64_t replaced_sa_index, uint64_t inserted_sa_index)
    {
        int64_t b1 = c < bwt[i] || ((c == bwt[i]) && (replaced_sa_index <= i)) ? 1 : 0;
        assert(inserted_sa_index < bwt.size());
        int64_t b2 = bwt[inserted_sa_index] < bwt[i] || ((bwt[inserted_sa_index] == bwt[i]) && (inserted_sa_index <= i)) ? -1 : 0;
        int64_t r = ((int64_t)LF(bwt, i)) + b1 + b2;
        assert(r >= 0);

        return r;
    }
    static int64_t compute_special_rank(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t old_char, uint64_t current_replaced_sa_index)
    {

        int64_t b1 = old_char < bwt[i] || ((old_char == bwt[i]) && (current_replaced_sa_index <= i)) ? 1 : 0;
        int64_t r = ((int64_t)LF(bwt, i)) + b1;
        return r;
    };
    static std::vector<uint64_t> compute_s_array_rank_for_insertion2(const std::vector<uint8_t> &bwt, uint8_t old_char, uint64_t current_replaced_sa_index)
    {
        std::vector<uint64_t> s_rank_array;

        std::vector<uint8_t> bwt2;
        for (uint64_t i = 0; i < bwt.size(); i++)
        {
            if (i == current_replaced_sa_index)
            {
                bwt2.push_back(old_char);
            }
            bwt2.push_back(bwt[i]);
        }
        for (uint64_t j = 0; j < bwt2.size(); j++)
        {
            uint64_t x = lexCount(bwt2, bwt2[j]) + rank(bwt2, j, bwt2[j]) - 1;
            s_rank_array.push_back(x);
        }
        return s_rank_array;
    }
    static std::vector<uint64_t> compute_inverse_s_array_rank_for_insertion2(const std::vector<uint8_t> &bwt, uint8_t old_char, uint64_t current_replaced_sa_index)
    {
        std::vector<uint8_t> bwt2;
        for (uint64_t i = 0; i < bwt.size(); i++)
        {
            if (i == current_replaced_sa_index)
            {
                bwt2.push_back(old_char);
            }
            bwt2.push_back(bwt[i]);
        }

        std::vector<uint64_t> inv_s_rank_array;
        inv_s_rank_array.resize(bwt2.size(), UINT64_MAX);

        for (uint64_t j = 0; j < bwt2.size(); j++)
        {
            uint64_t s_rank = lexCount(bwt2, bwt2[j]) + rank(bwt2, j, bwt2[j]) - 1;
            inv_s_rank_array[s_rank] = j;
        }
        return inv_s_rank_array;
    };

    static std::vector<uint64_t> compute_s_array_rank_for_insertion(const std::vector<uint8_t> &bwt, uint8_t old_char, uint64_t current_replaced_sa_index)
    {
        std::vector<uint64_t> s_rank_array;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            s_rank_array.push_back(compute_special_rank(bwt, j, old_char, current_replaced_sa_index));
        }
        return s_rank_array;
    };
    static std::vector<uint64_t> compute_s_array_rank_for_insertion2(const std::vector<uint8_t> &bwt2)
    {

        std::vector<uint64_t> s_rank_array;
        for (uint64_t j = 0; j < bwt2.size(); j++)
        {
            s_rank_array.push_back(lexCount(bwt2, bwt2[j]) + rank(bwt2, j, bwt2[j]) - 1);
        }
        return s_rank_array;
    };

    static std::vector<uint64_t> compute_inverse_s_array_rank_for_insertion(const std::vector<uint8_t> &bwt, uint8_t old_char, uint64_t current_replaced_sa_index)
    {
        std::vector<uint64_t> s_rank_array;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            s_rank_array.push_back(compute_special_rank(bwt, j, old_char, current_replaced_sa_index));
        }
        std::vector<uint64_t> inv_s_rank_array;
        inv_s_rank_array.resize(bwt.size() + 1, UINT64_MAX);
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            inv_s_rank_array[s_rank_array[j]] = j;
        }

        return inv_s_rank_array;
    };

    static uint64_t LF_for_replace_phase(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c, uint64_t replaced_sa_index)
    {
        uint64_t x = 0;
        uint8_t yc = i != replaced_sa_index ? bwt[i] : c;

        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            uint8_t xc = j != replaced_sa_index ? bwt[j] : c;
            if (xc < yc)
            {
                x++;
            }
            else if (xc == yc && j <= i)
            {
                x++;
            }
        }

        return x - 1;
    }
    static int64_t get_largest_char_index(const std::vector<uint8_t> &bwt, uint8_t c)
    {
        int64_t x = -1;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            if (bwt[j] < c)
            {
                if (x == -1)
                {
                    x = j;
                }
                else
                {
                    if (bwt[x] < bwt[j])
                    {
                        x = j;
                    }
                    else if (bwt[x] == bwt[j])
                    {
                        x = j;
                    }
                }
            }
        }
        return x;
    }
    static uint64_t lexCount(const std::vector<uint8_t> &bwt, uint8_t c)
    {
        uint64_t x = 0;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            if (bwt[j] < c)
            {
                x++;
            }
        }
        return x;
    }

    static uint64_t rank(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c)
    {
        uint64_t x = 0;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            if (bwt[j] == c && j <= i)
            {
                x++;
            }
        }
        return x;
    }
    static int64_t select(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c)
    {
        uint64_t x = 0;
        for (uint64_t j = 0; j < bwt.size(); j++)
        {
            if (bwt[j] == c)
            {
                x++;
            }
            if (x == i)
            {
                return j;
            }
        }
        throw std::runtime_error("select is not found");
    }

    static uint64_t srank(const std::vector<uint8_t> &bwt, uint64_t i){
        return lexCount(bwt, bwt[i]) + rank(bwt, i, bwt[i]) - 1;
    }
};

class TemporaryRIndex
{
public:
    std::vector<uint8_t> bwt;
    std::vector<uint64_t> sa;

    TemporaryRIndex()
    {
    }
    TemporaryRIndex(std::vector<uint8_t> _bwt, std::vector<uint64_t> _sa)
    {
        for (uint8_t c : _bwt)
        {
            this->bwt.push_back(c);
        }
        for (uint64_t i : _sa)
        {
            this->sa.push_back(i);
        }
    }
    std::vector<uint64_t> construct_suffix_array_for_insertion(uint8_t old_char, uint64_t current_replaced_sa_index, uint64_t inserted_string_length, uint64_t original_text_length, uint64_t last_inserted_index, uint64_t lambda)
    {
        uint64_t lambda_prev = original_text_length + inserted_string_length - 1;
        if (lambda > 0)
        {
            lambda_prev = lambda - 1;
        }
        std::vector<uint8_t> bwt2;
        for (uint64_t i = 0; i < bwt.size(); i++)
        {
            if (i == current_replaced_sa_index)
            {
                bwt2.push_back(old_char);
            }
            if (i != last_inserted_index)
            {
                bwt2.push_back(bwt[i]);
            }
            else
            {
                bwt2.push_back(old_char);
            }
        }
        std::vector<uint64_t> sa2;
        for (uint64_t i = 0; i < sa.size(); i++)
        {
            if (i == current_replaced_sa_index)
            {
                sa2.push_back(lambda);
            }
            sa2.push_back(sa[i]);
        }

        std::vector<uint64_t> s_rank_array = TemporaryBWT::compute_s_array_rank_for_insertion2(bwt2);
        std::vector<uint64_t> r;
        r.resize(bwt2.size(), UINT64_MAX);

        stool::DebugPrinter::print_characters(bwt2, "bwt2");

        stool::DebugPrinter::print_integers(s_rank_array, "s_rank_array");
        stool::DebugPrinter::print_integers(sa2, "sa2");

        uint64_t last_inserted_index2 = last_inserted_index;
        if (current_replaced_sa_index <= last_inserted_index)
        {
            last_inserted_index2++;
        }

        for (uint64_t i = 0; i < s_rank_array.size(); i++)
        {
            uint64_t s_rank = s_rank_array[i];

            if (i == current_replaced_sa_index)
            {
                r[s_rank] = lambda_prev;
            }
            else
            {
                [[maybe_unused]]uint64_t j = i;

                /*
                if(bwt2[last_inserted_index2] < bwt2[i] ||  (bwt2[last_inserted_index2] == bwt2[i] && last_inserted_index2 <= i)){
                    s_rank--;
                }
                */

                /*
                if(old_char < bwt2[i] ||  (old_char == bwt2[i] && last_inserted_index < i)){
                    s_rank--;
                }
                */

                /*
                if(current_replaced_sa_index <= i){
                    j--;
                }
                */

                if (s_rank < r.size())
                {
                    if (i == last_inserted_index2)
                    {
                        r[s_rank] = UINT64_MAX - 1;
                    }
                    else
                    {
                        if (sa2[i] > 0)
                        {
                            r[s_rank] = sa2[i] - 1;
                        }
                        else
                        {
                            r[s_rank] = original_text_length + inserted_string_length - 1;
                        }
                    }
                }
            }
        }
        std::vector<uint64_t> r2;
        for (uint64_t i = 0; i < r.size(); i++)
        {
            if (r[i] != UINT64_MAX - 1)
            {
                r2.push_back(r[i]);
            }
        }

        return r2;
    }

    /*
    uint64_t LF(uint64_t i)
    {
        uint64_t x = 0;
        for (uint64_t j = 0; j < this->sa.size(); j++)
        {
            if (this->bwt[j] < this->bwt[i])
            {
                x++;
            }
            else if (this->bwt[j] == this->bwt[i] && j <= i)
            {
                x++;
            }
        }
        return x - 1;
    }
    */

    uint64_t LF_phi(uint64_t i)
    {
        uint64_t max = *std::max_element(std::begin(this->sa), std::end(this->sa));
        uint64_t rank_c = TemporaryBWT::rank(this->bwt, i, this->bwt[i]);
        if (rank_c > 1)
        {
            uint64_t px = TemporaryBWT::select(this->bwt, rank_c - 1, this->bwt[i]);
            return this->sa[px] - 1 > 0 ? this->sa[px] - 1 : max;
        }
        else
        {
            int64_t px = TemporaryBWT::get_largest_char_index(this->bwt, this->bwt[i]);
            assert(px >= 0);
            return this->sa[px] - 1 > 0 ? this->sa[px] - 1 : max;
        }
    }
    /*
    uint64_t isa_phi(uint64_t i){
        uint64_t max = *std::max_element(std::begin(this->sa), std::end(this->sa));
        uint64_t sa_value = this->sa[i] != 0 ? this->sa[i] - 1 : max;

        uint64_t x = 0;
        for(uint64_t j = 0; j < this->sa.size(); j++){
            if(this->sa[j] == i){
                return j;
            }
        }
        throw std::runtime_error("LF_phi is not found");
    }
    */

    uint64_t expect_LF(uint64_t i)
    {
        uint64_t max = *std::max_element(std::begin(this->sa), std::end(this->sa));
        uint64_t v = this->sa[i] != 0 ? this->sa[i] - 1 : max;

        int64_t x = -1;
        uint64_t max_index = 0;
        for (uint64_t j = 0; j < this->sa.size(); j++)
        {
            if (max == this->sa[j])
            {
                max_index = j;
            }
            if (x == -1 && this->sa[j] <= v)
            {
                x = j;
            }
            else if (x != -1 && this->sa[x] < this->sa[j] && this->sa[j] <= v)
            {
                x = j;
            }
        }
        if (x != -1)
        {
            return x;
        }
        else
        {
            return max_index;
        }
    }

    std::vector<uint64_t> accessSA_by_LF_for_insertion(uint8_t old_char, uint64_t current_replaced_sa_index, [[maybe_unused]]uint64_t insertion_pos, [[maybe_unused]]uint64_t max_sa_value)
    {

        std::vector<uint64_t> s_rank_array = TemporaryBWT::compute_s_array_rank_for_insertion(this->bwt, old_char, current_replaced_sa_index);
        std::vector<uint64_t> inv_s_rank_array = TemporaryBWT::compute_inverse_s_array_rank_for_insertion(this->bwt, old_char, current_replaced_sa_index);

        stool::DebugPrinter::print_integers(s_rank_array, "s_rank");
        stool::DebugPrinter::print_integers(inv_s_rank_array, "inv_s_rank");

        std::vector<uint64_t> result;
        result.resize(this->sa.size(), UINT64_MAX);
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            if (inv_s_rank_array[i] != UINT64_MAX)
            {
                result[inv_s_rank_array[i]] = i;
            }
        }
        return result;
    }

    int64_t compute_LF_phi_for_insertion2(uint64_t i, uint8_t old_char, uint64_t current_replaced_sa_index, uint64_t insertion_pos, uint64_t max_sa_value)
    {
        std::vector<uint64_t> s_rank_array = TemporaryBWT::compute_s_array_rank_for_insertion2(this->bwt, old_char, current_replaced_sa_index);
        std::vector<uint64_t> inv_s_rank_array = TemporaryBWT::compute_inverse_s_array_rank_for_insertion2(this->bwt, old_char, current_replaced_sa_index);

        std::vector<uint64_t> sa2;
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            if (i == current_replaced_sa_index)
            {
                sa2.push_back(insertion_pos);
            }
            sa2.push_back(this->sa[i]);
        }

        uint64_t ip = i >= current_replaced_sa_index ? i + 1 : i;

        [[maybe_unused]]uint64_t replace_rank = s_rank_array[current_replaced_sa_index];
        assert(s_rank_array[ip] < s_rank_array.size());
        uint64_t prev_rank = s_rank_array[ip] == 0 ? this->sa.size() : (s_rank_array[ip] - 1);


            assert(prev_rank < inv_s_rank_array.size());
            assert(inv_s_rank_array[prev_rank] < sa2.size());

        if (sa2[inv_s_rank_array[prev_rank]] > 0)
        {
            return sa2[inv_s_rank_array[prev_rank]] - 1;
        }
        else
        {
            return max_sa_value;
        }
    }

    int64_t compute_LF_phi_for_insertion(uint64_t i, uint8_t old_char, uint64_t current_replaced_sa_index, uint64_t insertion_pos, uint64_t max_sa_value)
    {
        std::vector<uint64_t> s_rank_array = TemporaryBWT::compute_s_array_rank_for_insertion(this->bwt, old_char, current_replaced_sa_index);
        std::vector<uint64_t> inv_s_rank_array = TemporaryBWT::compute_inverse_s_array_rank_for_insertion(this->bwt, old_char, current_replaced_sa_index);

        // std::cout << "Prev: char: " << (char)old_char << "/ current_replaced_sa_index: " << current_replaced_sa_index << std::endl;
        // stool::DebugPrinter::print_integers(s_rank_array, "s_rank");

        uint64_t prev_rank = s_rank_array[i] == 0 ? this->sa.size() : s_rank_array[i] - 1;
        if (inv_s_rank_array[prev_rank] != UINT64_MAX)
        {

            if (this->sa[inv_s_rank_array[prev_rank]] > 0)
            {
                return this->sa[inv_s_rank_array[prev_rank]] - 1;
            }
            else
            {
                return max_sa_value;
            }
        }
        else
        {
            if (insertion_pos > 0)
            {
                return insertion_pos - 1;
            }
            else
            {
                return max_sa_value;
            }
        }
    };
    int64_t compute_LF_inverse_phi_for_insertion(uint64_t i, uint8_t old_char, uint64_t current_replaced_sa_index, uint64_t insertion_pos, uint64_t max_sa_value)
    {
        std::vector<uint64_t> s_rank_array = TemporaryBWT::compute_s_array_rank_for_insertion(this->bwt, old_char, current_replaced_sa_index);
        std::vector<uint64_t> inv_s_rank_array = TemporaryBWT::compute_inverse_s_array_rank_for_insertion(this->bwt, old_char, current_replaced_sa_index);

        uint64_t next_rank = s_rank_array[i] < this->sa.size() ? s_rank_array[i] + 1 : 0;
        if (inv_s_rank_array[next_rank] != UINT64_MAX)
        {
            if (this->sa[inv_s_rank_array[next_rank]] > 0)
            {
                return this->sa[inv_s_rank_array[next_rank]] - 1;
            }
            else
            {
                return max_sa_value;
            }
        }
        else
        {
            if (insertion_pos > 0)
            {
                return insertion_pos - 1;
            }
            else
            {
                return max_sa_value;
            }
        }
    };
    int64_t compute_LF_phi_for_move(uint64_t i)
    {
        uint64_t max = *std::max_element(std::begin(this->sa), std::end(this->sa));
        std::vector<uint64_t> s_rank_array;
        for (uint64_t j = 0; j < this->sa.size(); j++)
        {
            s_rank_array.push_back(TemporaryBWT::LF(this->bwt, j));
        }
        std::vector<uint64_t> inv_s_rank_array;
        inv_s_rank_array.resize(this->sa.size() + 1, UINT64_MAX);
        for (uint64_t j = 0; j < this->sa.size(); j++)
        {
            inv_s_rank_array[s_rank_array[j]] = j;
        }
        uint64_t prev_rank = s_rank_array[i] == 0 ? this->sa.size() - 1 : s_rank_array[i] - 1;
        if (this->sa[inv_s_rank_array[prev_rank]] > 0)
        {
            return this->sa[inv_s_rank_array[prev_rank]] - 1;
        }
        else
        {
            return max;
        }
    };
    int64_t compute_LF_inverse_phi_for_move(uint64_t i)
    {
        uint64_t max = *std::max_element(std::begin(this->sa), std::end(this->sa));
        std::vector<uint64_t> s_rank_array;
        for (uint64_t j = 0; j < this->sa.size(); j++)
        {
            s_rank_array.push_back(TemporaryBWT::LF(this->bwt, j));
        }
        std::vector<uint64_t> inv_s_rank_array;
        inv_s_rank_array.resize(this->sa.size() + 1, UINT64_MAX);
        for (uint64_t j = 0; j < this->sa.size(); j++)
        {
            inv_s_rank_array[s_rank_array[j]] = j;
        }
        uint64_t next_rank = s_rank_array[i] + 1 < this->sa.size() ? s_rank_array[i] + 1 : 0;
        if (this->sa[inv_s_rank_array[next_rank]] > 0)
        {
            return this->sa[inv_s_rank_array[next_rank]] - 1;
        }
        else
        {
            return max;
        }
    };

    void check_LF_for_default()
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            assert(TemporaryBWT::LF(this->bwt, i) == this->expect_LF(i));
        }
    }
    void check_LF_for_replace_phase(uint8_t c, uint64_t replaced_sa_index)
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            // std::cout << i << ": " << this->sa[i] << "->" << this->sa[this->LF_for_replace_phase(i, c, replaced_sa_index)] << "/@" << this->sa[this->expect_LF(i)] << "/" << (char)c << "/" << replaced_sa_index << std::endl;

            assert(TemporaryBWT::LF_for_replace_phase(this->bwt, i, c, replaced_sa_index) == this->expect_LF(i));
        }
    }
    void check_LF_for_insertion_phase(uint8_t c, uint64_t replaced_sa_index, uint64_t inserted_sa_value, uint64_t inserted_sa_index)
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            if (this->sa[i] != inserted_sa_value)
            {
                uint64_t r = TemporaryBWT::LF_for_insertion_phase(this->bwt, i, c, replaced_sa_index, inserted_sa_index);
                assert(r == this->expect_LF(i));
            }
        }
    }
    void check_LF_for_insertion_last_phase(uint8_t c, uint64_t replaced_sa_index, [[maybe_unused]]uint64_t inserted_sa_value, uint64_t inserted_sa_index)
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            uint8_t c2 = this->bwt[i];
            uint64_t r = TemporaryBWT::LF(this->bwt, i);

            if (c != c2)
            {
                assert(r == this->expect_LF(i));
            }
            else if (i != inserted_sa_index)
            {
                int64_t b1 = i >= inserted_sa_index ? -1 : 0;
                int64_t b2 = replaced_sa_index <= i ? 1 : 0;
                int64_t r2 = r + b1 + b2;
                assert(r2 == (int64_t)this->expect_LF(i));
            }
        }
    }

    void check_LF_for_move_phase(uint64_t from, uint64_t to)
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            if (i != to)
            {
                uint64_t r = TemporaryBWT::LF_for_move_phase(this->bwt, i, from, to);
                std::cout << i << ": " << this->sa[i] << "->" << this->sa[r] << "/@" << this->sa[this->expect_LF(i)] << "/skip1:" << to << std::endl;
                assert(r == this->expect_LF(i));
            }
        }
    }

    /*
    TemporaryRIndex copy(){
        TemporaryRIndex tmp;
        for(uint8_t c : this->bwt){
            tmp.bwt.push_back(c);
        }
        for(uint64_t i : this->sa){
            tmp.sa.push_back(i);
        }
        return tmp;
    }
    */

    TemporaryRIndex expand(int64_t v, int64_t added_value, uint64_t new_index, uint8_t new_char)
    {
        TemporaryRIndex tmp(this->bwt, this->sa);
        for (uint64_t i = 0; i < tmp.sa.size(); i++)
        {
            if ((int64_t)tmp.sa[i] >= v)
            {
                tmp.sa[i] += added_value;
            }
        }
        tmp.bwt[new_index] = new_char;
        return tmp;
    }

    TemporaryRIndex r_insert(int64_t new_index, uint8_t new_char, uint64_t new_value)
    {
        TemporaryRIndex tmp;
        for (int64_t i = 0; i < (int64_t)this->sa.size(); i++)
        {
            if (i == new_index)
            {
                tmp.sa.push_back(new_value);
            }
            tmp.sa.push_back(this->sa[i]);
        }

        for (int64_t i = 0; i < (int64_t)this->bwt.size(); i++)
        {
            if (i == new_index)
            {
                tmp.bwt.push_back(new_char);
            }
            tmp.bwt.push_back(this->bwt[i]);
        }
        if ((int64_t)this->sa.size() == new_index)
        {
            tmp.sa.push_back(new_value);
            tmp.bwt.push_back(new_char);
        }
        assert(this->bwt.size() + 1 == tmp.bwt.size());
        assert(this->sa.size() + 1 == tmp.sa.size());

        return tmp;
    }

    TemporaryRIndex r_delete(int64_t delete_index)
    {
        TemporaryRIndex tmp;
        for (int64_t i = 0; i < (int64_t)this->sa.size(); i++)
        {
            if (i != delete_index)
            {
                tmp.sa.push_back(this->sa[i]);
            }
        }

        for (int64_t i = 0; i < (int64_t)this->bwt.size(); i++)
        {
            if (i != delete_index)
            {
                tmp.bwt.push_back(this->bwt[i]);
            }
        }
        return tmp;
    }

    TemporaryRIndex r_move(int64_t delete_index, int64_t move_index)
    {
        uint64_t c = this->bwt[delete_index];
        uint64_t v = this->sa[delete_index];

        TemporaryRIndex tmp = this->r_delete(delete_index);
        TemporaryRIndex tmp2 = tmp.r_insert(move_index, c, v);

        return tmp2;
    }

    void print()
    {
        std::cout << "TemporaryRIndex = {" << std::endl;
        std::cout << "\t";
        stool::DebugPrinter::print_characters(this->bwt, "bwt");
        std::cout << "\t";
        stool::DebugPrinter::print_integers(this->sa, "sa");
    }

    std::vector<uint64_t> construct_next_SA_for_move([[maybe_unused]]uint8_t old_char, [[maybe_unused]]uint64_t current_replaced_sa_index, [[maybe_unused]]uint64_t insertion_pos, uint64_t max_sa_value)
    {
        std::vector<uint64_t> result;
        result.resize(this->sa.size(), UINT64_MAX);

        for(uint64_t i = 0; i < this->sa.size(); i++){
            uint64_t v = this->sa[i];
            uint64_t w = TemporaryBWT::srank(this->bwt, i);
            if(v != 0){
                result[w] = v- 1;
            }
            else{
                result[w] = max_sa_value;
            }
        }
        return result;
    }


    std::vector<std::pair<uint64_t, uint64_t>> compute_LF_phi_for_insertion_all(uint8_t old_char, uint64_t current_replaced_sa_index, uint64_t insertion_pos, uint64_t max_sa_value)
    {

        std::vector<std::pair<uint64_t, uint64_t>> result;

        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            uint64_t v = this->sa[i];
            uint64_t w = this->compute_LF_phi_for_insertion2(i, old_char, current_replaced_sa_index, insertion_pos, max_sa_value);
            if (v != 0)
            {
                result.push_back(std::make_pair(v - 1, w));
            }
            else
            {
                result.push_back(std::make_pair(max_sa_value, w));
            }
        }
        return result;
    }
    std::vector<std::pair<uint64_t, uint64_t>> compute_inverse_LF_phi_for_insertion_all(uint8_t old_char, uint64_t current_replaced_sa_index, uint64_t insertion_pos, uint64_t max_sa_value)
    {

        std::vector<std::pair<uint64_t, uint64_t>> result;

        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            uint64_t v = this->sa[i];
            uint64_t w = this->compute_LF_inverse_phi_for_insertion(i, old_char, current_replaced_sa_index, insertion_pos, max_sa_value);
            if (v != 0)
            {
                result.push_back(std::make_pair(v - 1, w));
            }
            else
            {
                result.push_back(std::make_pair(max_sa_value, w));
            }
        }
        return result;
    }

    uint64_t phi_function(uint64_t value)
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            if (this->sa[i] == value)
            {
                if (i > 0)
                {
                    return this->sa[i - 1];
                }
                else
                {
                    return this->sa[this->sa.size() - 1];
                }
            }
        }
        return UINT64_MAX;
    }
    uint64_t inverse_phi_function(uint64_t value)
    {
        for (uint64_t i = 0; i < this->sa.size(); i++)
        {
            if (this->sa[i] == value)
            {
                if (i + 1 < this->sa.size())
                {
                    return this->sa[i + 1];
                }
                else
                {
                    return this->sa[0];
                }
            }
        }
        return UINT64_MAX;
    }
};
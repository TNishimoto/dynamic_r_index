#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "stool/include/lib.hpp"
#include "libdivsufsort/sa.hpp"
#include "./dynamic_fm_index_test.hpp"
#include "./dynamic_r_index_test.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        class NaiveOperations{
            public:
            static int64_t inverse_phi(const std::vector<uint64_t> &sa, int64_t x){
                for(int64_t j = 0; j < (int64_t)sa.size(); j++){
                    if((int64_t)sa[j] == x){
                        if(j + 1 < (int64_t)sa.size()){
                            return sa[j + 1];
                        }else{
                            return sa[0];
                        }
                    }
                }
                throw std::runtime_error("Error: inverse_phi");
            }
            static int64_t phi(const std::vector<uint64_t> &sa, int64_t x){
                for(int64_t j = 0; j < (int64_t)sa.size(); j++){
                    if((int64_t)sa[j] == x){
                        if(j > 0){
                            return sa[j - 1];
                        }else{
                            return sa[sa.size() - 1];
                        }
                    }
                }
                throw std::runtime_error("Error: phi");
            }


            static std::string get_circular_string(const std::string &text, uint64_t starting_pos)
            {
                assert(starting_pos < text.size());
                return text.substr(starting_pos) + text.substr(0, starting_pos);
            }
            static uint64_t rank(const std::string &text, uint64_t i, uint8_t c)
            {
                uint64_t count = 0;
                for (uint64_t j = 0; j <= i; j++)
                {
                    if (text[j] == c)
                    {
                        count++;
                    }
                }
                return count;
            }
            static uint64_t lex_count(const std::string &text, uint8_t c)
            {
                uint64_t count = 0;
                for (auto c1 : text)
                {
                    if (c1 < c)
                    {
                        count++;
                    }
                }
                return count;
            }

            static std::vector<std::pair<uint8_t, uint64_t>> get_RLE(const std::string &bwt){
                std::vector<std::pair<uint8_t, uint64_t>> RLE;
                uint64_t count = 1;
                uint8_t c = bwt[0];

                for(uint64_t i = 1; i < bwt.size(); i++){
                    if(bwt[i-1] != bwt[i]){
                        RLE.push_back(std::make_pair(c, count));
                        count = 1;
                        c = bwt[i];
                    }else{
                        count++;
                    }
                }
                if(count > 0){
                    RLE.push_back(std::make_pair(c, count));

                }
                return RLE;
            }
            static int64_t get_rle_index_by_position(const std::vector<std::pair<uint8_t, uint64_t>> &rle, uint64_t position){
                uint64_t p = 0;
                for(uint64_t i = 0; i < rle.size(); i++){
                    if(p <= position && position < p + rle[i].second){
                        return i;
                    }
                    p += rle[i].second;
                }
                return -1;
            }
            static int64_t get_starting_position(const std::vector<std::pair<uint8_t, uint64_t>> &rle, uint64_t rle_index){
                uint64_t p = 0;
                for(uint64_t i = 0; i < rle_index; i++){
                    p += rle[i].second;
                }
                return p;
            }
            

            static std::vector<uint64_t> get_SA_s(const std::string &bwt, const std::vector<uint64_t> &sa){
                std::vector<uint64_t> SA_s;
                for(uint64_t i = 0; i < sa.size(); i++){
                    if(i == 0 || bwt[i-1] != bwt[i]){
                        SA_s.push_back(sa[i]);
                    }
                }
                return SA_s;
            }
            static std::vector<uint64_t> get_SA_e(const std::string &bwt, const std::vector<uint64_t> &sa){
                std::vector<uint64_t> SA_e;
                for(uint64_t i = 0; i < sa.size(); i++){
                    if(i == sa.size() - 1 || bwt[i] != bwt[i+1]){
                        SA_e.push_back(sa[i]);
                    }
                }
                return SA_e;
            }

            static uint64_t get_array_pos_plus(const std::vector<uint64_t> &array, int64_t i){
                assert(i >= 0 && i < (int64_t)array.size());
                if (i == (int64_t)array.size() - 1)
                {
                    return array[0];
                }
                else
                {
                    return array[i + 1];
                }
            }
            static uint64_t get_array_pos_minus(const std::vector<uint64_t> &array, int64_t i){
                assert(i >= 0 && i < (int64_t)array.size());
                if (i == 0)
                {
                    return array[array.size() - 1];
                }
                else
                {
                    return array[i - 1];
                }
            }


            static uint8_t get_c_succ(uint8_t c_v, const std::string &bwt)
            {
                uint16_t c_succ = UINT16_MAX;
                for (uint64_t i = 0; i < bwt.size(); i++)
                {
                    if (bwt[i] > c_v && (uint16_t)bwt[i] < c_succ)
                    {
                        c_succ = bwt[i];
                    }
                }
                if (c_succ == UINT16_MAX)
                {
                    c_succ = '$';
                }

                return c_succ;
            }
            static uint8_t get_c_prev(uint8_t c_v, const std::string &bwt)
            {
                int16_t c_prev = -1;
                int16_t c_max = -1;
                for (uint64_t i = 0; i < bwt.size(); i++)
                {
                    if (bwt[i] < c_v && (uint16_t)bwt[i] > c_prev)
                    {
                        c_prev = bwt[i];
                    }
                    if(bwt[i] > c_max)
                    {
                        c_max = bwt[i];
                    }
                }
                if (c_prev == -1)
                {
                    c_prev = c_max;
                }

                return c_prev;
            }

            static int64_t compute_sa_e_at_max_V(std::vector<std::pair<uint8_t, uint64_t>> &rle, std::vector<uint64_t> &SA_e, int64_t v)
            {
                int64_t s = -1;


                for (int64_t i = v-1; i >= 0; i--)
                {
                    if (rle[i].first == rle[v].first)
                    {
                        s = SA_e[i];
                        break;
                    }
                }
                return s;
            }
            static int64_t compute_sa_e_at_max_V_prime(std::vector<std::pair<uint8_t, uint64_t>> &rle, std::vector<uint64_t> &SA_e, uint8_t c_prev)
            {
                int64_t s = -1;
                for (int64_t i = (int64_t)rle.size() - 1; i >= 0; i--)
                {
                    if (rle[i].first == c_prev)
                    {
                        s = SA_e[i];
                        break;
                    }
                }
                return s;
            }


            static int64_t compute_sa_s_at_min_U(std::vector<std::pair<uint8_t, uint64_t>> &rle, std::vector<uint64_t> &SA_s, int64_t v)
            {
                int64_t s = -1;

                for (int64_t i = v + 1; i < (int64_t)rle.size(); i++)
                {
                    if (rle[i].first == rle[v].first)
                    {
                        s = SA_s[i];
                        break;
                    }
                }
                return s;
            }
            static int64_t compute_sa_s_at_min_U_prime(std::vector<std::pair<uint8_t, uint64_t>> &rle, std::vector<uint64_t> &SA_s, uint8_t c_succ)
            {
                int64_t s = -1;
                for (int64_t i = 0; i < (int64_t)rle.size(); i++)
                {
                    if (rle[i].first == c_succ)
                    {
                        s = SA_s[i];
                        break;
                    }
                }
                return s;
            }
            static uint8_t get_previous_character(const std::string &text, uint64_t i){
                if(i == 0){
                    return text[text.size() - 1];
                }
                else{
                    return text[i - 1];
                }
            }
            static std::vector<uint64_t> create_suffix_array_from_conceptual_matrix(const std::vector<std::string> &conceptual_matrix, uint8_t end_marker) {
                std::vector<uint64_t> suffix_array;
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    suffix_array.push_back(access_suffix_array(conceptual_matrix, i, end_marker));
                }
                return suffix_array;
            }
            static std::vector<uint64_t> create_inverse_suffix_array_from_conceptual_matrix(const std::vector<std::string> &conceptual_matrix, uint8_t end_marker)
            {
                std::vector<uint64_t> suffix_array = NaiveOperations::create_suffix_array_from_conceptual_matrix(conceptual_matrix, end_marker);
                uint64_t max = *std::max_element(suffix_array.begin(), suffix_array.end());
                std::vector<uint64_t> isa;
                isa.resize(max + 1, UINT64_MAX);
                for (uint64_t i = 0; i < suffix_array.size(); i++)
                {
                    isa[suffix_array[i]] = i;
                }
                return isa;
            }
            static uint64_t access_inverse_suffix_array(const std::vector<std::string> &conceptual_matrix,uint64_t i, uint8_t end_marker)
            {
                uint64_t endmarker_pos = conceptual_matrix[i].size() - 1;
                for (uint64_t j = 0; j < conceptual_matrix[i].size(); j++)
                {
                    if (conceptual_matrix[i][j] == end_marker)
                    {
                        endmarker_pos = j;
                        break;
                    }
                }
                return conceptual_matrix[i].size() - (endmarker_pos + 1);
            }
            static std::string construct_bwt_from_conceptual_matrix(const std::vector<std::string> &conceptual_matrix)
            {
                std::string bwt;
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    bwt.push_back(conceptual_matrix[i][conceptual_matrix[i].size() - 1]);
                }
                return bwt;
            }



            static uint64_t access_suffix_array(const std::vector<std::string> &conceptual_matrix, uint64_t i, uint8_t end_marker)
            {
                uint64_t endmarker_pos = conceptual_matrix[i].size() - 1;
                for (uint64_t j = 0; j < conceptual_matrix[i].size(); j++)
                {
                    if (conceptual_matrix[i][j] == end_marker)
                    {
                        endmarker_pos = j;
                        break;
                    }
                }
                return conceptual_matrix[i].size() - (endmarker_pos + 1);
            }
            static uint64_t access_suffix_array_for_deletion(const std::vector<std::string> &conceptual_matrix, uint64_t i, uint8_t end_marker, uint64_t old_text_size, uint64_t special_gap)
            {
                uint64_t endmarker_pos = conceptual_matrix[i].size() - 1;
                for (uint64_t j = 0; j < conceptual_matrix[i].size(); j++)
                {
                    if (conceptual_matrix[i][j] == end_marker)
                    {
                        endmarker_pos = j;
                        break;
                    }
                }
                if (conceptual_matrix[i].size() == old_text_size)
                {
                    return special_gap + (conceptual_matrix[i].size() - (endmarker_pos + 1));
                }
                else
                {
                    return conceptual_matrix[i].size() - (endmarker_pos + 1);
                }

            }
            static int64_t value_with_index(int64_t value, bool index_begin_with_1 = false)
            {
                return index_begin_with_1 ? value + 1 : value;
            }
            static void sort_conceptual_matrix(std::vector<std::string> &conceptual_matrix)
            {
                std::sort(conceptual_matrix.begin(), conceptual_matrix.end());
            }


            static int64_t compute_next_SA_in_dynamic_LF_order2(int64_t x, int64_t sa_x_minus, const std::string &bwt, const std::vector<uint64_t> &sa)
            {
                auto rle = NaiveOperations::get_RLE(bwt);
                int64_t v = NaiveOperations::get_rle_index_by_position(rle, x);
                uint8_t c_v = rle[v].first;

                assert(v != -1 && v < (int64_t)rle.size());
                int64_t t_v = NaiveOperations::get_starting_position(rle, v);
                //int64_t ell_v = rle[v].second;

                auto SA_e = NaiveOperations::get_SA_e(bwt, sa);
                uint8_t c_prev = NaiveOperations::get_c_prev(c_v, bwt);
                int64_t max_V = NaiveOperations::compute_sa_e_at_max_V(rle, SA_e, v);
                int64_t max_V_prime = NaiveOperations::compute_sa_e_at_max_V_prime(rle, SA_e, c_prev);

                int64_t SA_j_h = 0;

                if (x != t_v)
                {
                    SA_j_h = sa_x_minus;
                }
                else if (x == t_v  && max_V != -1)
                {
                    SA_j_h = max_V;
                }
                else
                {
                    assert(max_V_prime != -1);
                    SA_j_h = max_V_prime;
                }
                return SA_j_h;

            }

            static int64_t compute_next_SA_in_dynamic_LF_order(int64_t x, int64_t sa_x_plus, const std::string &bwt, const std::vector<uint64_t> &sa)
            {
                auto rle = NaiveOperations::get_RLE(bwt);
                int64_t v = NaiveOperations::get_rle_index_by_position(rle, x);
                uint8_t c_v = rle[v].first;

                assert(v != -1 && v < (int64_t)rle.size());
                int64_t t_v = NaiveOperations::get_starting_position(rle, v);
                int64_t ell_v = rle[v].second;
                auto SA_s = NaiveOperations::get_SA_s(bwt, sa);
                uint8_t c_succ = NaiveOperations::get_c_succ(c_v, bwt);
                int64_t min_U = NaiveOperations::compute_sa_s_at_min_U(rle, SA_s, v);
                int64_t min_U_prime = NaiveOperations::compute_sa_s_at_min_U_prime(rle, SA_s, c_succ);

                int64_t SA_j_h = 0;

                if (x != t_v + ell_v - 1)
                {
                    SA_j_h = sa_x_plus;
                }
                else if (x == t_v + ell_v - 1 && min_U != -1)
                {
                    SA_j_h = min_U;
                }
                else
                {
                    assert(min_U_prime != -1);
                    SA_j_h = min_U_prime;
                }

                

                return SA_j_h;
            }



        };
    }
}

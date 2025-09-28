#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "stool/include/stool.hpp"
#include "libdivsufsort/sa.hpp"
#include "./dynamic_fm_index_test.hpp"
#include "./dynamic_r_index_test.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        class NaiveOperations{
            public:
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

            static uint64_t get_SA_plus(const std::vector<uint64_t> &sa, int64_t i){
                assert(i >= 0 && i < (int64_t)sa.size());
                if (i == (int64_t)sa.size() - 1)
                {
                    return sa[0];
                }
                else
                {
                    return sa[i + 1];
                }
            }
            static uint64_t get_SA_minus(const std::vector<uint64_t> &sa, int64_t i){
                assert(i >= 0 && i < (int64_t)sa.size());
                if (i == 0)
                {
                    return sa[sa.size() - 1];
                }
                else
                {
                    return sa[i - 1];
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

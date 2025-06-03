#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "stool/include/stool.hpp"
#include "libdivsufsort/sa.hpp"

namespace stool
{
    namespace fm_index_test
    {
        class NaiveDynamicStringForBWT
        {
        public:
            std::vector<uint8_t> text;
            uint8_t end_marker = '$';

            void initialzie(uint8_t _end_marker = '$')
            {
                this->end_marker = _end_marker;
                this->text.push_back(_end_marker);
            }
            void initialzie(const std::vector<uint8_t> &text_with_end_marker)
            {
                this->end_marker = text_with_end_marker[text_with_end_marker.size() - 1];
                this->text.clear();
                for (auto c : text_with_end_marker)
                {
                    this->text.push_back(c);
                }
            }

            void insert_char(uint64_t pos, int64_t character)
            {
                if (character <= this->end_marker)
                {
                    throw std::runtime_error("Error: NaiveDynamicStringForBWT");
                }
                this->text.insert(this->text.begin() + pos, character);
            }
            void insert_string(uint64_t pos, std::vector<uint8_t> &pattern)
            {
                std::vector<uint8_t> _text;
                for (uint64_t i = 0; i < pos; i++)
                {
                    _text.push_back(text[i]);
                }
                for (uint8_t c : pattern)
                {
                    _text.push_back(c);
                }
                for (uint64_t i = pos; i < text.size(); i++)
                {
                    _text.push_back(text[i]);
                }
                this->text.swap(_text);
            }
            void delete_char(uint64_t pos)
            {
                this->text.erase(this->text.begin() + pos);
            }
            void delete_string(uint64_t pos, uint64_t len)
            {
                for (uint64_t i = 0; i < len; i++)
                {
                    this->text.erase(this->text.begin() + pos);
                }
            }

            std::string to_str() const
            {
                std::string s;
                for (auto it : this->text)
                {
                    s.push_back(it);
                }
                return s;
            }

            std::vector<uint64_t> create_suffix_array()
            {
                return libdivsufsort::construct_suffix_array(this->text, stool::Message::NO_MESSAGE);
            }
            std::vector<uint64_t> create_inverse_suffix_array()
            {
                std::vector<uint64_t> sa = this->create_suffix_array();
                return stool::construct_ISA(this->text, sa, stool::Message::NO_MESSAGE);
            }
            std::vector<uint8_t> create_bwt()
            {
                std::vector<uint64_t> sa = this->create_suffix_array();
                std::vector<uint8_t> bwt;
                for (uint64_t i = 0; i < sa.size(); i++)
                {
                    if (sa[i] > 0)
                    {
                        bwt.push_back(this->text[sa[i] - 1]);
                    }
                    else
                    {
                        bwt.push_back(this->text[this->text.size() - 1]);
                    }
                }
                return bwt;
            }
            void construct_SA_and_LF_array_for_deletion(uint64_t delete_pos, uint64_t delete_len, std::vector<std::vector<uint64_t>> &output_sa_arrays, std::vector<std::vector<uint64_t>> &output_lf_arrays, std::vector<uint64_t> &output_idx_array)
            {
                std::vector<uint64_t> sa = this->create_suffix_array();

                output_sa_arrays.push_back(sa);

                output_idx_array.push_back(this->create_inverse_suffix_array()[delete_pos + delete_len - 1]);

                // stool::DebugPrinter::print_integers(sa, "sa");

                for (uint64_t i = 0; i < delete_len; i++)
                {
                    std::vector<uint64_t> tmp_sa;
                    for (uint64_t j = 0; j < sa.size(); j++)
                    {
                        uint64_t lower = delete_pos + delete_len - i - 1;
                        uint64_t upper = delete_pos + delete_len - 1;
                        uint64_t b_pointer = lower > 0 ? lower - 1 : this->size() - 1;
                        if (sa[j] < lower || sa[j] > upper)
                        {
                            tmp_sa.push_back(sa[j]);
                        }
                        if (sa[j] == b_pointer)
                        {
                            output_idx_array.push_back(tmp_sa.size() - 1);
                        }
                    }
                    auto tmp_lf = NaiveDynamicStringForBWT::construct_dynamic_LF_array(output_sa_arrays[output_sa_arrays.size() - 1], tmp_sa);
                    output_sa_arrays.push_back(tmp_sa);
                    output_lf_arrays.push_back(tmp_lf);
                }
            }

            void construct_SA_arrays_for_deletion(uint64_t delete_pos, uint64_t delete_len, std::vector<std::vector<uint64_t>> &output_sa_arrays)
            {
                std::vector<uint64_t> sa = this->create_suffix_array();

                output_sa_arrays.push_back(sa);

                // stool::DebugPrinter::print_integers(sa, "sa");

                for (uint64_t i = 0; i < delete_len; i++)
                {
                    std::vector<uint64_t> tmp_sa;
                    for (uint64_t j = 0; j < sa.size(); j++)
                    {
                        uint64_t lower = delete_pos + delete_len - i - 1;
                        uint64_t upper = delete_pos + delete_len - 1;
                        //uint64_t b_pointer = lower > 0 ? lower - 1 : this->size() - 1;
                        if (sa[j] < lower || sa[j] > upper)
                        {
                            if (sa[j] > upper)
                            {
                                tmp_sa.push_back(sa[j] - (upper - lower + 1));
                            }
                            else
                            {
                                tmp_sa.push_back(sa[j]);
                            }
                        }
                    }
                    output_sa_arrays.push_back(tmp_sa);
                }
            }

            static std::vector<uint64_t> construct_dynamic_LF_array(const std::vector<uint64_t> &sa1, const std::vector<uint64_t> &sa2)
            {
                uint64_t max = 0;
                for (uint64_t i = 0; i < sa1.size(); i++)
                {
                    if (sa1[i] > max)
                    {
                        max = sa1[i];
                    }
                }
                std::vector<uint64_t> isa;
                isa.resize(max + 1, UINT64_MAX);
                for (uint64_t i = 0; i < sa2.size(); i++)
                {
                    isa[sa2[i]] = i;
                }

                std::vector<uint64_t> lf_array;
                lf_array.resize(sa1.size(), UINT64_MAX);

                for (uint64_t i = 0; i < sa1.size(); i++)
                {
                    for (int64_t j = sa1[i]; j >= 0; j--)
                    {
                        if (j > 0)
                        {
                            if (isa[j - 1] != UINT64_MAX)
                            {
                                lf_array[i] = isa[j - 1];
                                break;
                            }
                        }
                        else
                        {
                            lf_array[i] = isa[max];
                            break;
                        }
                    }
                }

                return lf_array;
            }

            std::string create_bwt_str()
            {
                std::string s;
                std::vector<uint8_t> bwt = this->create_bwt();

                for (auto it : bwt)
                {
                    s.push_back(it);
                }
                return s;
            }

            int64_t size()
            {
                return this->text.size();
            }
        };
    }
}

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

            void insert_char(uint64_t pos, int64_t character)
            {
                if(character <= this->end_marker){
                    throw std::runtime_error("Error: NaiveDynamicStringForBWT");
                }
                this->text.insert(this->text.begin() + pos, character);
            }
            void insert_string(uint64_t pos, std::vector<uint8_t> &pattern)
            {
                std::vector<uint8_t> _text;
                for(uint64_t i = 0; i < pos; i++){
                    _text.push_back(text[i]);
                }
                for(uint8_t c : pattern){
                    _text.push_back(c);
                }
                for(uint64_t i = pos; i < text.size(); i++){
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

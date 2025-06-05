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
        class DynamicRIndexSnapShotForDeletion
        {
        public:
            std::string text;
            std::string updated_text;
            uint64_t deletion_pos;
            uint64_t deletion_length;

            std::vector<std::string> conceptual_matrix;
            uint8_t end_marker = '$';

            std::vector<uint64_t> sa;
            std::vector<uint64_t> isa;
            std::string bwt;

            uint64_t t = 0;

            /*
            std::string get_nbwt() const
            {
                std::string bwt2 = this->bwt;

                bwt2[this->isa[this->deletion_pos]] = this->text[this->deletion_pos + this->deletion_length - 1];
                return bwt2;
            }
            */

            uint8_t get_old_character() const
            {
                return this->text[this->deletion_pos + this->deletion_length - 1];
            }
            uint8_t get_new_character() const
            {
                if (this->deletion_pos == 0)
                {
                    return this->text[this->text.size() - 1];
                }
                else
                {
                    return this->text[this->deletion_pos - 1];
                }
            }

            uint64_t access_suffix_array(uint64_t i) const
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
                if (conceptual_matrix[i].size() == this->text.size())
                {
                    return 10000 + (conceptual_matrix[i].size() - (endmarker_pos + 1));
                }
                else
                {
                    return conceptual_matrix[i].size() - (endmarker_pos + 1);
                }
            }

            std::vector<uint64_t> construct_dynamic_LF_array(const std::vector<uint64_t> &next_isa) const
            {
                // std::vector<uint64_t> suffix_array = this->create_suffix_array();

                std::vector<uint64_t> lf_array;
                lf_array.resize(this->sa.size(), UINT64_MAX);
                for (uint64_t i = 0; i < this->sa.size(); i++)
                {
                    uint64_t p = this->sa[i];
                    uint64_t q = UINT64_MAX;

                    if (p == 10000)
                    {
                            q = this->updated_text.size() - 1;
                        /*
                        if (this->deletion_pos == 0 && this->t > this->deletion_pos && this->t <= this->deletion_pos + this->deletion_length)
                        {
                            q = UINT64_MAX;
                        }
                        else
                        {
                        }
                        */
                    }
                    else
                    {
                        q = p - 1;
                    }

                    if (q == UINT64_MAX)
                    {
                        lf_array[i] = UINT64_MAX;
                    }
                    else if (next_isa[q] != UINT64_MAX)
                    {
                        lf_array[i] = next_isa[q];
                    }
                    else
                    {
                        lf_array[i] = UINT64_MAX;
                    }
                }
                return lf_array;
            }

            uint64_t dynamic_LF(uint64_t i) const
            {
                int64_t lf = rank(this->bwt, i, this->bwt[i]) + lex_count(this->bwt, this->bwt[i]) - 1;

                if (this->t > this->deletion_pos && this->t <= this->deletion_pos + this->deletion_length)
                {
                    uint64_t replace_pos = this->isa[this->deletion_pos];

                    uint8_t new_char = this->get_new_character();

                    if (i == replace_pos)
                    {
                        return UINT64_MAX;
                    }
                    else
                    {

                        uint8_t c = this->bwt[i];

                        if (c > new_char || (i > replace_pos && c == new_char))
                        {
                            return lf > 0 ? lf -1 : this->conceptual_matrix.size() - 2;
                        }else{
                            return lf;
                        }        
                    }
                }
                else
                {
                    return lf;
                }
            }

            void verify_dynamic_LF(const DynamicRIndexSnapShotForDeletion &next) const
            {
                std::vector<uint64_t> next_isa = next.isa;
                std::vector<uint64_t> correct_array = construct_dynamic_LF_array(next_isa);
                // std::string bwt = construct_bwt();
                std::vector<uint64_t> test_array;

                for (uint64_t i = 0; i < this->bwt.size(); i++)
                {
                    test_array.push_back(this->dynamic_LF(i));
                }

                try
                {
                    stool::equal_check("dynamic LF check", correct_array, test_array);
                }
                catch (const std::exception &e)
                {
                    std::cout << "Error: " << e.what() << std::endl;

                    std::cout << "t = " << t << std::endl;
                    std::cout << "deletion_pos = " << deletion_pos << std::endl;
                    std::cout << "deletion_length = " << deletion_length << std::endl;
                    std::cout << "text = " << text << std::endl;
                    std::cout << "updated_text = " << updated_text << std::endl;

                    this->print_conceptual_matrix();
                    next.print_conceptual_matrix();

                    stool::DebugPrinter::print_integers(this->sa, "current suffix_array");
                    stool::DebugPrinter::print_integers(next.sa, "next suffix_array");

                    stool::DebugPrinter::print_integers(correct_array, "correct_array");
                    stool::DebugPrinter::print_integers(test_array, "test_array");
                    throw -1;
                }
            }

        private:
            std::vector<uint64_t> create_suffix_array() const
            {
                std::vector<uint64_t> suffix_array;
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    suffix_array.push_back(access_suffix_array(i));
                }
                return suffix_array;
            }
            std::vector<uint64_t> create_inverse_suffix_array() const
            {
                std::vector<uint64_t> suffix_array = this->create_suffix_array();
                uint64_t max = *std::max_element(suffix_array.begin(), suffix_array.end());
                std::vector<uint64_t> isa;
                isa.resize(max + 1, UINT64_MAX);
                for (uint64_t i = 0; i < suffix_array.size(); i++)
                {
                    isa[suffix_array[i]] = i;
                }
                return isa;
            }
            std::string construct_bwt() const
            {
                std::string bwt;
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    bwt.push_back(conceptual_matrix[i][conceptual_matrix[i].size() - 1]);
                }
                return bwt;
            }
            /*
            uint64_t access_inverse_suffix_array(uint64_t i) const
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
            */

        public:
            void build(const std::string &_text, uint64_t _deletion_pos, uint64_t &_deletion_length)
            {
                text = std::string(_text.begin(), _text.end());
                deletion_pos = _deletion_pos;
                deletion_length = _deletion_length;

                this->updated_text = text.substr(0, deletion_pos) + text.substr(deletion_pos + deletion_length);

                for (uint64_t i = 0; i < text.size(); i++)
                {
                    conceptual_matrix.push_back(get_circular_string(text, i));
                }
                this->sort_conceptual_matrix();
                t = this->text.size();

                sa = create_suffix_array();
                isa = create_inverse_suffix_array();
                bwt = construct_bwt();
            }

            static std::string get_circular_string(const std::string &text, uint64_t starting_pos)
            {
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

            int64_t get_starting_position_of_new_circular_string(uint64_t t) const
            {
                if (t < this->deletion_pos)
                {
                    return t;
                }
                else if (this->deletion_pos <= t && t < this->deletion_pos + this->deletion_length)
                {
                    return -1;
                }
                else
                {
                    return t - this->deletion_length;
                }
            }

            void update()
            {
                this->t--;
                std::string old_circular_string = get_circular_string(this->text, t);
                int64_t p = this->get_starting_position_of_new_circular_string(this->t);

                if (p != -1)
                {
                    std::string new_circular_string = get_circular_string(this->updated_text, p);

                    for (uint64_t i = 0; i < this->conceptual_matrix.size(); i++)
                    {
                        if (conceptual_matrix[i] == old_circular_string)
                        {
                            conceptual_matrix[i] = new_circular_string;
                            break;
                        }
                    }
                }
                else
                {
                    uint64_t x = UINT64_MAX;
                    for (uint64_t i = 0; i < this->conceptual_matrix.size(); i++)
                    {
                        if (conceptual_matrix[i] == old_circular_string)
                        {
                            x = i;
                            break;
                        }
                    }
                    conceptual_matrix.erase(conceptual_matrix.begin() + x);
                }
                this->sort_conceptual_matrix();

                sa = create_suffix_array();
                isa = create_inverse_suffix_array();
                bwt = construct_bwt();
            }

            void sort_conceptual_matrix()
            {
                std::sort(conceptual_matrix.begin(), conceptual_matrix.end());
            }

            bool is_stop() const
            {
                return t == 0;
            }

            const DynamicRIndexSnapShotForDeletion copy()
            {
                DynamicRIndexSnapShotForDeletion snap_shot;
                snap_shot.text = text;
                snap_shot.updated_text = updated_text;
                snap_shot.deletion_length = deletion_length;
                snap_shot.deletion_pos = deletion_pos;
                snap_shot.conceptual_matrix = conceptual_matrix;
                snap_shot.t = t;
                return snap_shot;
            }

            static std::vector<DynamicRIndexSnapShotForDeletion> get_all_snap_shots(std::string text, uint64_t deletion_pos, uint64_t deletion_length)
            {
                std::vector<DynamicRIndexSnapShotForDeletion> snap_shots;
                DynamicRIndexSnapShotForDeletion snap_shot;
                snap_shot.build(text, deletion_pos, deletion_length);
                snap_shots.push_back(snap_shot);

                while (!snap_shots[snap_shots.size() - 1].is_stop())
                {
                    snap_shots.push_back(snap_shots[snap_shots.size() - 1].copy());
                    snap_shots[snap_shots.size() - 1].update();
                }

                return snap_shots;
            }

            void print_conceptual_matrix() const
            {
                std::cout << this->t << "--------------------------------" << std::endl;
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    std::cout << conceptual_matrix[i] << std::endl;
                }
                std::cout << "--------------------------------" << std::endl;
            }

            static void deletion_test(std::string text, uint64_t deletion_pos, uint64_t deletion_length)
            {
                std::vector<DynamicRIndexSnapShotForDeletion> snap_shots = DynamicRIndexSnapShotForDeletion::get_all_snap_shots(text, deletion_pos, deletion_length);

                // std::cout << "insertion range = [" << insertion_pos << ", " << (insertion_pos + inserted_string.size() - 1) << "]" << std::endl;

                /*
                for (uint64_t i = 0; i < snap_shots.size(); i++)
                {
                     snap_shots[i].print_conceptual_matrix();
                     stool::DebugPrinter::print_integers(snap_shots[i].sa, "suffix_array");
                     std::cout << "bwt: " << snap_shots[i].bwt << std::endl;
                }
                */

                for (uint64_t i = 0; i < snap_shots.size() - 1; i++)
                {
                    // snap_shots[i].print_conceptual_matrix();

                    snap_shots[i].verify_dynamic_LF(snap_shots[i + 1]);
                }

                // std::cout << std::endl;
            }

            static void deletion_test(uint64_t text_size, uint64_t deletion_length, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = fm_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, '$');

                // dfmi.initialize(alphabet_with_end_marker);

                std::vector<uint8_t> _text = r_index_test::DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);

                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, text_size - deletion_length - 1);
                uint64_t deletion_pos = get_rand_uni_int(mt64);

                std::string text = std::string(_text.begin(), _text.end());

                DynamicRIndexSnapShotForDeletion::deletion_test(text, deletion_pos, deletion_length);
            }
        };

    }
}

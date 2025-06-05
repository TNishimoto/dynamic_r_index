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
        class DynamicRIndexSnapShotForInsertion
        {
        public:
            std::string text;
            std::string updated_text;
            std::string inserted_string;
            uint64_t insertion_pos;

            std::vector<std::string> conceptual_matrix;
            uint8_t end_marker = '$';

            std::vector<uint64_t> sa;
            std::vector<uint64_t> isa;
            std::string bwt;

            int64_t t = 0;

            uint8_t get_old_character() const
            {
                if (this->insertion_pos == 0)
                {
                    return this->text[this->text.size() - 1];
                }
                else
                {
                    return this->text[this->insertion_pos - 1];
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
                return conceptual_matrix[i].size() - (endmarker_pos + 1);
            }

            std::vector<uint64_t> construct_dynamic_LF_array(const std::vector<uint64_t> &next_isa) const
            {
                //std::vector<uint64_t> suffix_array = this->create_suffix_array();

                std::vector<uint64_t> lf_array;
                lf_array.resize(this->sa.size(), UINT64_MAX);
                for (uint64_t i = 0; i < this->sa.size(); i++)
                {
                    uint64_t p = this->sa[i];
                    uint64_t q = p > 0 ? p - 1 : this->updated_text.size() - 1;

                    if (next_isa[q] != UINT64_MAX)
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

            /*
            static uint64_t LF_for_insertion_phase(const std::vector<uint8_t> &bwt, uint64_t i, uint8_t c, uint64_t replaced_sa_index, uint64_t inserted_sa_index)
            {
                int64_t b1 = c < bwt[i] || ((c == bwt[i]) && (replaced_sa_index <= i)) ? 1 : 0;
                assert(inserted_sa_index < bwt.size());
                int64_t b2 = bwt[inserted_sa_index] < bwt[i] || ((bwt[inserted_sa_index] == bwt[i]) && (inserted_sa_index <= i)) ? -1 : 0;
                int64_t r = ((int64_t)LF(bwt, i)) + b1 + b2;
                assert(r >= 0);

                return r;
            }
            */
            uint64_t dynamic_LF(uint64_t i) const
            {
                //std::string bwt = construct_bwt();
               // std::vector<uint64_t> isa = create_inverse_suffix_array();
                uint64_t lf = rank(this->bwt, i, this->bwt[i]) + lex_count(this->bwt, this->bwt[i]) - 1;
                if (this->t > this->insertion_pos && this->t <= this->insertion_pos + this->inserted_string.size())
                {
                    uint8_t old_char = this->get_old_character();
                    uint64_t positionToReplace = this->isa[this->insertion_pos + this->inserted_string.size()];
                    // uint64_t positionToReplace = isa[this->t];

                    if (old_char < this->bwt[i])
                    {
                        return lf + 1;
                    }
                    else if (positionToReplace <= i && this->bwt[i] == old_char)
                    {
                        return lf + 1;
                    }
                    else
                    {
                        return lf;
                    }
                }
                else
                {
                    return lf;
                }
            }

            void verify_dynamic_LF(const DynamicRIndexSnapShotForInsertion &next) const
            {
                std::vector<uint64_t> next_isa = next.create_inverse_suffix_array();
                std::vector<uint64_t> correct_array = construct_dynamic_LF_array(next_isa);
                //std::string bwt = construct_bwt();
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
                    std::cout << "insertion_pos = " << insertion_pos << std::endl;
                    std::cout << "inserted_string = " << inserted_string << std::endl;
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

        public:
            void build(const std::string &_text, uint64_t _insertion_pos, const std::string &_inserted_string)
            {
                text = std::string(_text.begin(), _text.end());
                inserted_string = std::string(_inserted_string.begin(), _inserted_string.end());
                insertion_pos = _insertion_pos;

                this->updated_text = text.substr(0, insertion_pos) + inserted_string + text.substr(insertion_pos);

                for (uint64_t i = 0; i < text.size(); i++)
                {
                    conceptual_matrix.push_back(get_circular_string(text, i));
                }
                this->sort_conceptual_matrix();
                t = this->updated_text.size();

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

            int64_t get_starting_position_of_old_circular_string(uint64_t t) const
            {
                if (t < this->insertion_pos)
                {
                    return t;
                }
                else if (this->insertion_pos <= t && t < this->insertion_pos + this->inserted_string.size())
                {
                    return -1;
                }
                else
                {
                    return t - this->inserted_string.size();
                }
            }

            void update()
            {
                this->t--;
                int64_t p = this->get_starting_position_of_old_circular_string(this->t);
                std::string new_circular_string = get_circular_string(this->updated_text, this->t);

                if (p != -1)
                {
                    std::string old_circular_string = get_circular_string(this->text, p);
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
                    conceptual_matrix.push_back(new_circular_string);
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

            const DynamicRIndexSnapShotForInsertion copy()
            {
                DynamicRIndexSnapShotForInsertion snap_shot;
                snap_shot.text = text;
                snap_shot.updated_text = updated_text;
                snap_shot.inserted_string = inserted_string;
                snap_shot.insertion_pos = insertion_pos;
                snap_shot.conceptual_matrix = conceptual_matrix;
                snap_shot.t = t;
                return snap_shot;
            }

            static std::vector<DynamicRIndexSnapShotForInsertion> get_all_snap_shots(std::string text, uint64_t insertion_pos, std::string inserted_string)
            {
                std::vector<DynamicRIndexSnapShotForInsertion> snap_shots;
                DynamicRIndexSnapShotForInsertion snap_shot;
                snap_shot.build(text, insertion_pos, inserted_string);
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

            static void insertion_test(std::string text, uint64_t insertion_pos, std::string inserted_string)
            {
                std::vector<DynamicRIndexSnapShotForInsertion> snap_shots = DynamicRIndexSnapShotForInsertion::get_all_snap_shots(text, insertion_pos, inserted_string);

                // std::cout << "insertion range = [" << insertion_pos << ", " << (insertion_pos + inserted_string.size() - 1) << "]" << std::endl;

                for (uint64_t i = 0; i < snap_shots.size() - 1; i++)
                {
                    // snap_shots[i].print_conceptual_matrix();

                    snap_shots[i].verify_dynamic_LF(snap_shots[i + 1]);
                }
                // std::cout << std::endl;
            }

            static void insertion_test(uint64_t text_size, uint64_t insertion_length, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = fm_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, '$');

                // dfmi.initialize(alphabet_with_end_marker);

                std::vector<uint8_t> _text = r_index_test::DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);
                std::vector<uint8_t> _pattern = r_index_test::DynamicRIndexTest::create_text(insertion_length + 1, chars, alphabet_with_end_marker[0], mt64);
                _pattern.pop_back();

                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, text_size - 1);
                uint64_t insertion_pos = get_rand_uni_int(mt64);

                std::string text = std::string(_text.begin(), _text.end());
                std::string pattern = std::string(_pattern.begin(), _pattern.end());

                DynamicRIndexSnapShotForInsertion::insertion_test(text, insertion_pos, pattern);
            }
        };

    }
}

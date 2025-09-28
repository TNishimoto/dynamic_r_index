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
            enum ModeForDeletion
            {
                Preprocessing = 1,
                CharDeletion = 2,
                PostPreprocessing = 3,
                Finished = 4
            };

        public:
            std::string text;
            std::string updated_text;
            int64_t deletion_pos;
            int64_t deletion_length;

            std::vector<std::string> conceptual_matrix;
            uint8_t end_marker = '$';

            std::vector<uint64_t> sa;
            std::vector<uint64_t> isa;
            std::string bwt;

            int64_t j = 0;

            uint64_t SPECIAL_GAP = 10000;

            /*
            std::string get_nbwt() const
            {
                std::string bwt2 = this->bwt;

                bwt2[this->isa[this->deletion_pos]] = this->text[this->deletion_pos + this->deletion_length - 1];
                return bwt2;
            }
            */
            ModeForDeletion get_mode() const
            {
                int64_t high_p = this->deletion_pos + this->deletion_length;
                if (this->j > high_p)
                {
                    return Preprocessing;
                }
                else if (this->j >= this->deletion_pos + 1 && this->j <= high_p)
                {
                    return CharDeletion;
                }
                else if (this->j > 0)
                {
                    return PostPreprocessing;
                }
                else
                {
                    return Finished;
                }
            }

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
                    return SPECIAL_GAP + (conceptual_matrix[i].size() - (endmarker_pos + 1));
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

                    if (p == SPECIAL_GAP)
                    {
                        q = this->updated_text.size() - 1;
                        /*
                        if (this->deletion_pos == 0 && this->j > this->deletion_pos && this->j <= this->deletion_pos + this->deletion_length)
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

                if (this->j > this->deletion_pos && this->j <= this->deletion_pos + this->deletion_length)
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
                            return lf > 0 ? lf - 1 : this->conceptual_matrix.size() - 2;
                        }
                        else
                        {
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
                    stool::EqualChecker::equal_check(correct_array, test_array, "dynamic LF check");
                }
                catch (const std::exception &e)
                {
                    std::cout << "Error: " << e.what() << std::endl;

                    std::cout << "j = " << this->j << std::endl;
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
                this->j = this->text.size();

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

            int64_t get_starting_position_of_new_circular_string(int64_t t) const
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
                this->j--;
                std::string old_circular_string = get_circular_string(this->text, this->j);
                int64_t p = this->get_starting_position_of_new_circular_string(this->j);

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
                return this->j == 0;
            }

            const DynamicRIndexSnapShotForDeletion copy() const
            {
                DynamicRIndexSnapShotForDeletion snap_shot;
                snap_shot.text = text;
                snap_shot.updated_text = updated_text;
                snap_shot.deletion_length = deletion_length;
                snap_shot.deletion_pos = deletion_pos;
                snap_shot.conceptual_matrix = conceptual_matrix;
                snap_shot.j = j;
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
            /*
            int64_t compute_naive_x() const
            {
                if (this->j != 0)
                {
                    uint64_t p = SPECIAL_GAP + this->j - 1;
                    for (uint64_t i = 0; i < this->conceptual_matrix.size(); i++)
                    {
                        if (this->sa[i] == p)
                        {
                            return i;
                        }
                    }
                    return -1;
                }
                else
                {
                    return -1;
                }
            }
            int64_t compute_naive_y() const
            {
                ModeForDeletion mode = this->get_mode();
                std::cout << "mode = " << mode << std::endl;
                if (mode == CharDeletion || mode == Finished)
                {
                    return -1;
                }
                else
                {

                    uint64_t value = 0;
                    if (mode == PostPreprocessing)
                    {
                        value = this->j - 1;
                    }
                    else
                    {
                        value = this->j - 1 - this->deletion_length;
                    }

                    auto copy = this->copy();
                    copy.update();
                    assert(copy.isa[value] != UINT64_MAX);
                    return copy.isa[value];
                }
            }
            */
            int64_t compute_x(DynamicRIndexSnapShotForDeletion *prev) const
            {
                int64_t j_max = (int64_t)this->text.size();
                ModeForDeletion mode = this->get_mode();
                if (this->j == j_max)
                {
                    return 0;
                }
                else if (mode == Finished)
                {
                    return -1;
                }
                else
                {
                    assert(prev != nullptr);
                    int64_t prev_x = prev->naive_compute_x();
                    ModeForDeletion prev_mode = prev->get_mode();
                    char prev_x_char = prev->bwt[prev_x];

                    uint64_t lf = NaiveOperations::rank(prev->bwt, prev_x, prev_x_char) + NaiveOperations::lex_count(prev->bwt, prev_x_char) - 1;

                    if (prev_mode == CharDeletion)
                    {
                        int64_t prev_q = prev->isa[this->deletion_pos];
                        int64_t prev_q_char = prev->bwt[prev_q];

                        bool b = prev_q_char < prev_x_char || (prev_q_char == prev_x_char && prev_q <= prev_x);

                        return b ? lf - 1 : lf;
                    }
                    else
                    {
                        return lf;
                    }
                }
            }
            int64_t compute_y(DynamicRIndexSnapShotForDeletion *prev) const
            {
                ModeForDeletion mode = this->get_mode();
                if (mode == Preprocessing)
                {
                    return this->naive_compute_x();
                }
                else if (mode == Finished || mode == CharDeletion)
                {
                    return -1;
                }
                else if (this->j == this->deletion_pos)
                {
                    int64_t q = this->isa[this->deletion_pos];
                    uint64_t lf = NaiveOperations::rank(this->bwt, q, this->bwt[q]) + NaiveOperations::lex_count(this->bwt, this->bwt[q]) - 1;
                    return lf;
                }
                else
                {
                    int64_t prev_y = prev->naive_compute_y();
                    assert(prev_y != -1);
                    assert(prev_y < (int64_t)this->bwt.size());
                    uint64_t lf = NaiveOperations::rank(this->bwt, prev_y, this->bwt[prev_y]) + NaiveOperations::lex_count(this->bwt, this->bwt[prev_y]) - 1;
                    return lf;
                }
            }

            void print_conceptual_matrix(bool index_begin_with_1 = false) const
            {
                std::vector<uint64_t> sa = this->create_suffix_array();
                int64_t naive_x = this->naive_compute_x();
                ModeForDeletion mode = this->get_mode();

                std::cout << "---- Information ------------------------" << std::endl;
                if (this->j != 0)
                {
                    std::cout << "i \tSA_{" << this->j << "}\tCM_{" << this->j << "}\t\tL_{" << this->j << "}\tLF_{" << this->j << "}" << std::endl;
                }
                else
                {
                    std::cout << "i \tSA_{" << this->j << "}\tCM_{" << this->j << "}\t\tL_{" << this->j << "}" << std::endl;
                }
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    int64_t color_mode = 0;
                    if ((int64_t)i == naive_x)
                    {
                        color_mode = 2;
                    }
                    else if (mode == Preprocessing && (int64_t)this->sa[i] == (this->j - this->deletion_length))
                    {
                        color_mode = 1;
                    }
                    else if (mode == PostPreprocessing && (int64_t)this->sa[i] == this->j)
                    {
                        color_mode = 1;
                    }
                    // std::cout << sa[i] << "\t" << conceptual_matrix[i] << std::endl;
                    print_conceptual_matrix(i, this->sa[i], conceptual_matrix[i], index_begin_with_1, color_mode);
                }
                std::cout << "--------------------------------" << std::endl;

                /*
                std::cout << this->j << "--------------------------------" << std::endl;
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    std::cout << sa[i] << "\t" << conceptual_matrix[i] << std::endl;
                }
                std::cout << "--------------------------------" << std::endl;
                */
            }
            void print_conceptual_matrix(uint64_t i, int64_t sa_v, std::string circular_shift, bool index_begin_with_1 = false, int64_t color_mode = 0) const
            {

                if (color_mode == 1)
                {
                    std::cout << "\x1b[43m";
                }
                else if (color_mode == 2)
                {
                    std::cout << "\x1b[47m";
                }

                int64_t modified_sa_v = sa_v >= (int64_t)SPECIAL_GAP ? (sa_v - SPECIAL_GAP) : sa_v;
                uint64_t _i = i;
                if (index_begin_with_1)
                {
                    _i++;
                    modified_sa_v++;
                }

                std::string modified_sa_v_str = sa_v >= (int64_t)SPECIAL_GAP ? (std::to_string(modified_sa_v) + "*") : std::to_string(modified_sa_v);

                if (this->j != 0)
                {
                    int64_t lf = this->dynamic_LF(i) == UINT64_MAX ? -1 : this->dynamic_LF(i);
                    if (index_begin_with_1)
                    {
                        lf++;
                    }
                    std::cout << _i << "\t" << modified_sa_v_str << "\t" << circular_shift << "\t\t" << this->bwt[i] << "\t" << lf;
                }
                else
                {
                    std::cout << _i << "\t" << modified_sa_v_str << "\t" << circular_shift << "\t\t" << this->bwt[i];
                }

                if (color_mode > 0)
                {

                    std::cout << "\x1b[49m" << std::flush;
                }
                std::cout << std::endl;
            }

            int64_t naive_compute_x() const
            {
                ModeForDeletion mode = this->get_mode();
                if (mode == Finished)
                {
                    return -1;
                }
                else
                {
                    return this->isa[this->j - 1 + SPECIAL_GAP];
                }
            }
            int64_t naive_compute_y() const
            {

                ModeForDeletion mode = this->get_mode();
                if (mode == Finished || mode == CharDeletion)
                {
                    return -1;
                }
                else
                {

                    uint64_t value = 0;
                    if (mode == PostPreprocessing)
                    {
                        value = this->j - 1;
                    }
                    else
                    {
                        value = this->j - 1 - this->deletion_length;
                    }

                    auto copy = this->copy();
                    copy.update();
                    return copy.isa[value];
                }
            }

            void verify_RLE_update(DynamicRIndexSnapShotForDeletion *prev) const
            {
                int64_t _naive_x = this->naive_compute_x();
                int64_t _x = this->compute_x(prev);
                ModeForDeletion mode = this->get_mode();

                if (_naive_x != _x)
                {
                    std::cout << "naive_x = " << _naive_x << ", x = " << _x << ", mode = " << mode << std::endl;
                    std::cout << "j = " << this->j << std::endl;
                    std::cout << "deletion_pos = " << this->deletion_pos << std::endl;
                    std::cout << "deletion_length = " << this->deletion_length << std::endl;
                    std::cout << "text = " << this->text << std::endl;
                    std::cout << "updated_text = " << this->updated_text << std::endl;
                    throw std::runtime_error("Error: naive_x != x");
                }
                int64_t _naive_y = this->naive_compute_y();
                int64_t _y = this->compute_y(prev);

                if (_naive_y != _y)
                {
                    std::cout << "naive_y = " << _naive_y << ", y = " << _y << ", mode = " << mode << std::endl;
                    throw std::runtime_error("Error: naive_y != y");
                }
            }
            int64_t compute_SA_j_h_for_SA1_plus(int64_t forbidden_sa_value) const
            {

                int64_t x = this->naive_compute_x();
                int64_t sa_x_plus = NaiveOperations::get_SA_plus(this->sa, x);
                int64_t SA_j_h_candidate = NaiveOperations::compute_next_SA_in_dynamic_LF_order(x, sa_x_plus, this->bwt, this->sa);
                assert(SA_j_h_candidate != -1);
                if (SA_j_h_candidate == forbidden_sa_value)
                {

                    int64_t forbidden_sa_value_pos = this->isa[forbidden_sa_value];
                    int64_t sa_forbidden_sa_value_pos_plus = NaiveOperations::get_SA_plus(this->sa, forbidden_sa_value_pos);
                    int64_t SA_j_h_candidate_for_forbidden_sa_value_pos = NaiveOperations::compute_next_SA_in_dynamic_LF_order(forbidden_sa_value_pos, sa_forbidden_sa_value_pos_plus, this->bwt, this->sa);
                    return SA_j_h_candidate_for_forbidden_sa_value_pos;
                }
                else
                {
                    return SA_j_h_candidate;
                }
            }
            int64_t compute_SA_j_h_for_SA2_plus(int64_t y_plus) const
            {

                int64_t sa_yp_plus = NaiveOperations::get_SA_plus(this->sa, y_plus);
                int64_t SA_j_h_candidate = NaiveOperations::compute_next_SA_in_dynamic_LF_order(y_plus, sa_yp_plus, this->bwt, this->sa);
                assert(SA_j_h_candidate != -1);
                return SA_j_h_candidate;
            }

            int64_t compute_next_SA1_plus() const
            {
                ModeForDeletion mode = this->get_mode();

                int64_t SA_j_h = -1;
                if (mode == CharDeletion)
                {
                    int64_t forbitten_sa_value = this->deletion_pos;
                    SA_j_h = this->compute_SA_j_h_for_SA1_plus(forbitten_sa_value);
                }
                else
                {
                    SA_j_h = this->compute_SA_j_h_for_SA1_plus(-1);
                }

                if (SA_j_h == (int64_t)SPECIAL_GAP)
                {
                    return this->updated_text.size() - 1;
                }
                else if (SA_j_h > 0)
                {
                    return SA_j_h - 1;
                }
                else
                {
                    return this->updated_text.size() - 1;
                }
            }
            int64_t compute_next_SA2_plus(int64_t y_plus, ModeForDeletion prev_mode, int64_t naive_answer) const
            {
                if (prev_mode == CharDeletion)
                {
                    return naive_answer;
                }
                else
                {
                    int64_t sa_yp_plus = NaiveOperations::get_SA_plus(this->sa, y_plus);
                    int64_t SA_j_h = NaiveOperations::compute_next_SA_in_dynamic_LF_order(y_plus, sa_yp_plus, this->bwt, this->sa);

                    if (SA_j_h == (int64_t)SPECIAL_GAP)
                    {
                        return this->updated_text.size() - 1;
                    }
                    else if (SA_j_h > 0)
                    {
                        return SA_j_h - 1;
                    }
                    else
                    {
                        return this->updated_text.size() - 1;
                    }
                }
            }
            void verify_SA_update(DynamicRIndexSnapShotForDeletion *prev) const
            {
                ModeForDeletion mode = this->get_mode();

                if (mode == CharDeletion || mode == PostPreprocessing)
                {
                    int64_t _x = this->naive_compute_x();
                    assert(_x != -1);

                    int64_t _naive_SA1_plus = NaiveOperations::get_SA_plus(this->sa, _x);
                    /*
                    if(_naive_SA1_plus > SPECIAL_GAP){
                        _naive_SA1_plus = _naive_SA1_plus - SPECIAL_GAP;
                    }
                    */

                    int64_t _SA1_plus = prev->compute_next_SA1_plus();
                    if (_naive_SA1_plus != _SA1_plus)
                    {
                        std::cout << "naive_SA1_plus = " << _naive_SA1_plus << ", SA1_plus = " << _SA1_plus << std::endl;
                        throw std::runtime_error("Error: naive_SA1_plus != SA1_plus");
                    }
                }

                if (mode == PostPreprocessing)
                {
                    int64_t _y = this->naive_compute_y();
                    assert(_y != -1);
                    auto next_snapshot = this->copy();
                    next_snapshot.update();

                    int64_t _naive_SA2_plus = NaiveOperations::get_SA_plus(next_snapshot.sa, _y);
                    int64_t y_plus = prev->naive_compute_y();

                    int64_t _SA2_plus = this->compute_next_SA2_plus(y_plus, prev->get_mode(), _naive_SA2_plus);
                    if (_naive_SA2_plus != _SA2_plus)
                    {
                        next_snapshot.print_conceptual_matrix();

                        std::cout << "naive_SA2_plus = " << _naive_SA2_plus << ", SA2_plus = " << _SA2_plus << std::endl;
                        throw std::runtime_error("Error: naive_SA2_plus != SA2_plus");
                    }
                }
            }

            static void deletion_test(std::string text, uint64_t deletion_pos, uint64_t deletion_length, bool view = false)
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

                for (uint64_t i = 0; i < snap_shots.size(); i++)
                {
                    if (view)
                    {
                        std::cout << "j = " << snap_shots[i].j << std::endl;
                        std::cout << "new_char = " << (char)snap_shots[i].get_new_character() << std::endl;
                        /*
                        std::cout << "replace_pos = " << snap_shots[i].isa[snap_shots[i].deletion_pos] << std::endl;
                        std::vector<uint64_t> next_isa = snap_shots[i + 1].isa;
                        std::vector<uint64_t> lf_array = snap_shots[i].construct_dynamic_LF_array(next_isa);
                        std::cout << "bwt: " << snap_shots[i].bwt << std::endl;
                        stool::DebugPrinter::print_integers(snap_shots[i].sa, "sa");
                        stool::DebugPrinter::print_integers(lf_array, "lf_array");
                        */

                        snap_shots[i].print_conceptual_matrix();

                        std::cout << std::endl;
                    }

                    stool::dynamic_r_index::DynamicRIndexSnapShotForDeletion *prev = i == 0 ? nullptr : &snap_shots[i - 1];

                    if (i < snap_shots.size() - 1)
                    {
                        snap_shots[i].verify_RLE_update(prev);
                        snap_shots[i].verify_dynamic_LF(snap_shots[i + 1]);
                        snap_shots[i].verify_SA_update(prev);
                    }
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
            static void deletion_test(uint64_t text_size, uint64_t deletion_length, uint8_t alphabet_type, uint64_t number_of_trials, uint64_t seed)
            {
                std::cout << "Test started." << std::endl;
                for (uint64_t i = 0; i < number_of_trials; i++)
                {
                    std::cout << "+" << std::flush;
                    deletion_test(text_size, deletion_length, alphabet_type, seed++);
                }
                std::cout << "[DONE]" << std::endl;
                std::cout << std::endl;
            }
        };

    }
}

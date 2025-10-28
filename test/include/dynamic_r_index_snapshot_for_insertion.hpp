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
#include "./naive_operations.hpp"
namespace stool
{
    namespace dynamic_r_index
    {

        class DynamicRIndexSnapShotForInsertion
        {
            enum ModeForInsertion
            {
                None = 0,
                Preprocessing = 1,
                CharInsertion = 2,
                PostPreprocessing = 3,
                Finished = 4
            };

        public:
            std::string text;
            std::string updated_text;
            std::string inserted_string;
            int64_t insertion_pos;

            std::vector<std::string> conceptual_matrix;
            uint8_t end_marker = '$';

            std::vector<uint64_t> sa;
            std::vector<uint64_t> isa;
            std::string bwt;

            int64_t j = 0;

        public:
            ModeForInsertion get_mode() const
            {
                int64_t high_p = this->insertion_pos + this->inserted_string.size();
                if (this->j > high_p)
                {
                    return Preprocessing;
                }
                else if (this->j >= this->insertion_pos + 1 && this->j <= high_p)
                {
                    return CharInsertion;
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
            bool is_first_preprocessing_mode() const
            {
                return this->j == (int64_t)this->updated_text.size();
            }
            bool is_first_postprocessing_mode() const
            {
                return this->j == this->insertion_pos;
            }

            bool is_last_preprocessing_mode() const
            {
                int64_t high_p = this->insertion_pos + this->inserted_string.size();
                return this->j == high_p + 1;
            }
            bool is_last_insertion_mode() const
            {
                return this->j == this->insertion_pos + 1;
            }

            int64_t get_starting_position_of_old_circular_string(int64_t t) const
            {
                if (t < this->insertion_pos)
                {
                    return t;
                }
                else if (this->insertion_pos <= t && t < this->insertion_pos + (int64_t)this->inserted_string.size())
                {
                    return -1;
                }
                else
                {
                    return t - this->inserted_string.size();
                }
            }

            bool is_stop() const
            {
                return this->j == 0;
            }

            // X and Y value methods

            int64_t compute_isa_i_minus(DynamicRIndexSnapShotForInsertion *prev) const
            {
                // ModeForInsertion mode = this->get_mode();
                ModeForInsertion prev_mode = (ModeForInsertion)None;

                if (prev_mode == None)
                {
                    return this->naive_compute_isa_i_minus();
                }
                else if (prev_mode == Preprocessing)
                {
                    return prev->naive_compute_isa_i_minus();
                }
                else if (prev_mode == CharInsertion)
                {
                    int64_t naive_isa_i_minus = prev->naive_compute_isa_i_minus();
                    int64_t prev_y = prev->naive_compute_y();
                    if (prev_y <= naive_isa_i_minus)
                    {
                        return naive_isa_i_minus + 1;
                    }
                    else
                    {
                        return naive_isa_i_minus;
                    }
                }
                else
                {
                    throw std::runtime_error("Error: mode == Preprocessing");
                }
            }
            int64_t compute_isa_p(DynamicRIndexSnapShotForInsertion *prev) const
            {
                ModeForInsertion prev_mode = (ModeForInsertion)None;

                if (prev_mode == None)
                {
                    return this->naive_compute_isa_p();
                }
                else if (prev_mode == Preprocessing)
                {
                    return prev->naive_compute_isa_p();
                }
                else if (prev_mode == CharInsertion)
                {
                    int64_t naive_isa_p = prev->naive_compute_isa_p();
                    int64_t prev_y = prev->naive_compute_y();
                    if (prev_y <= naive_isa_p)
                    {
                        return naive_isa_p + 1;
                    }
                    else
                    {
                        return naive_isa_p;
                    }
                }
                else
                {
                    throw std::runtime_error("Error: mode == Preprocessing");
                }
            }

            int64_t compute_x(DynamicRIndexSnapShotForInsertion *prev) const
            {
                int64_t j_max = (int64_t)this->text.size() + (int64_t)this->inserted_string.size();
                ModeForInsertion mode = this->get_mode();
                ModeForInsertion prev_mode = (ModeForInsertion)None;
                if (prev != nullptr)
                {
                    prev_mode = prev->get_mode();
                }

                if (mode == CharInsertion || mode == Finished)
                {
                    throw std::runtime_error("Error: mode == CharInsertion || mode == Finished");
                }
                else if (this->j == j_max)
                {
                    return 0;
                }
                else if (mode == PostPreprocessing && prev_mode == CharInsertion)
                {
                    return this->compute_isa_i_minus(prev);
                }
                else
                {
                    assert(prev != nullptr);
                    int64_t prev_x = prev->naive_compute_x();
                    uint64_t lf = NaiveOperations::rank(prev->bwt, prev_x, prev->bwt[prev_x]) + NaiveOperations::lex_count(prev->bwt, prev->bwt[prev_x]) - 1;
                    return lf;
                }
            }

            int64_t compute_y(DynamicRIndexSnapShotForInsertion *prev) const
            {
                ModeForInsertion mode = this->get_mode();

                if (mode == Finished)
                {
                    throw std::runtime_error("Error: mode == Finished");
                }
                else if (mode == Preprocessing)
                {
                    return this->compute_x(prev);
                }
                else if (mode == CharInsertion)
                {
                    int64_t p = this->insertion_pos + this->inserted_string.size();
                    int64_t q = this->isa[p];

                    int64_t prev_y = prev->naive_compute_y();
                    uint64_t lf = NaiveOperations::rank(this->bwt, prev_y, this->bwt[prev_y]) + NaiveOperations::lex_count(this->bwt, this->bwt[prev_y]) - 1;
                    bool b = this->text[this->insertion_pos - 1] < this->bwt[prev_y] || (this->text[this->insertion_pos - 1] == this->bwt[prev_y] && q <= prev_y);
                    if (b)
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
                    int64_t prev_y = prev->naive_compute_y();
                    uint64_t lf = NaiveOperations::rank(this->bwt, prev_y, this->bwt[prev_y]) + NaiveOperations::lex_count(this->bwt, this->bwt[prev_y]) - 1;
                    return lf;
                }
            }

            // SA value methods

            int64_t compute_next_SA1_plus(int64_t inverse_phi_i, int64_t inverse_phi_i_plus, const DynamicRIndexSnapShotForInsertion &next) const
            {
                ModeForInsertion next_mode = next.get_mode();

                std::cout << "inverse_phi_i = " << inverse_phi_i << ", inverse_phi_i_plus = " << inverse_phi_i_plus << "/" << "i = " << this->insertion_pos << "/" << next_mode << std::endl;


                // uint64_t high_p = this->insertion_pos + this->inserted_string.size();
                // int64_t y = this->naive_compute_y();

                if (next_mode == Preprocessing)
                {
                    if(next.is_last_preprocessing_mode()){
                        return inverse_phi_i >= (int64_t)(this->insertion_pos + this->inserted_string.size()) ? inverse_phi_i + this->inserted_string.size() : inverse_phi_i;
                    }else{
                        throw std::runtime_error("Error: mode == Preprocessing");
                    }
                }
                else if (next.is_first_postprocessing_mode())
                {
                    return inverse_phi_i_plus >= (int64_t)this->insertion_pos ? inverse_phi_i_plus + this->inserted_string.size(): inverse_phi_i_plus;
                }
                else
                {
                    int64_t x = this->naive_compute_x();
                    assert(x != -1 && x < (int64_t)this->sa.size());
                    int64_t sa_x_plus = NaiveOperations::get_array_pos_plus(this->sa, x);
                    int64_t SA_j_h = NaiveOperations::compute_next_SA_in_dynamic_LF_order(x, sa_x_plus, this->bwt, this->sa);
                    if (SA_j_h > 0)
                    {
                        return SA_j_h - 1;
                    }
                    else
                    {
                        return this->updated_text.size() - 1;
                    }
                }
            }
            int64_t compute_next_SA1_minus(int64_t naive_answer, const DynamicRIndexSnapShotForInsertion &next) const
            {
                ModeForInsertion next_mode = next.get_mode();

                // uint64_t high_p = this->insertion_pos + this->inserted_string.size();
                // int64_t y = this->naive_compute_y();
                if (next_mode == Preprocessing)
                {
                    if(next.is_last_preprocessing_mode()){
                        return naive_answer;
                    }else{
                        throw std::runtime_error("Error: mode == Preprocessing");
                    }
                }
                else if (next.is_first_postprocessing_mode())
                {
                    return naive_answer;
                }
                else
                {
                    int64_t x = this->naive_compute_x();
                    int64_t sa_x_minus = NaiveOperations::get_array_pos_minus(this->sa, x);
                    int64_t SA_j_h = NaiveOperations::compute_next_SA_in_dynamic_LF_order2(x, sa_x_minus, this->bwt, this->sa);
                    if (SA_j_h > 0)
                    {
                        return SA_j_h - 1;
                    }
                    else
                    {
                        return this->updated_text.size() - 1;
                    }
                }
            }
            int64_t compute_next_SA2_plus(int64_t y_plus) const
            {
                ModeForInsertion mode = this->get_mode();

                // uint64_t high_p = this->insertion_pos + this->inserted_string.size();
                int64_t y = this->naive_compute_y();

                if (mode == Preprocessing)
                {
                    if (this->is_last_preprocessing_mode())
                    {
                        int64_t x = this->naive_compute_x();
                        int64_t _naive_SA1_plus = NaiveOperations::get_array_pos_plus(this->sa, x);
                        return _naive_SA1_plus;
                    }
                    else
                    {
                        throw std::runtime_error("Error: mode == Preprocessing");
                    }
                }
                else if (mode == CharInsertion && y < (int64_t)this->sa.size() && (int64_t)this->sa[y] == this->insertion_pos - 1)
                {
                    return this->insertion_pos - 1;
                }
                else if (mode == CharInsertion && y == (int64_t)this->sa.size() && (int64_t)this->sa[0] == this->insertion_pos - 1)
                {
                    return this->insertion_pos - 1;
                }
                else
                {
                    int64_t sa_yp_plus = NaiveOperations::get_array_pos_plus(this->sa, y_plus);
                    int64_t SA_j_g = NaiveOperations::compute_next_SA_in_dynamic_LF_order(y_plus, sa_yp_plus, this->bwt, this->sa);
                    if (SA_j_g > 0)
                    {
                        return SA_j_g - 1;
                    }
                    else
                    {
                        return this->updated_text.size() - 1;
                    }
                }
            }
            int64_t compute_next_SA2_minus(int64_t y_plus) const
            {
                ModeForInsertion mode = this->get_mode();

                // uint64_t high_p = this->insertion_pos + this->inserted_string.size();
                int64_t y = this->naive_compute_y();

                if (mode == Preprocessing)
                {
                    if (this->is_last_preprocessing_mode())
                    {
                        int64_t x = this->naive_compute_x();
                        int64_t _naive_SA1_minus = NaiveOperations::get_array_pos_minus(this->sa, x);
                        return _naive_SA1_minus;
                    }
                    else
                    {
                        throw std::runtime_error("Error: mode == Preprocessing");
                    }
                }
                else if (mode == CharInsertion && y > 0 && (int64_t)this->sa[y - 1] == this->insertion_pos - 1)
                {
                    return this->insertion_pos - 1;
                }
                else if (mode == CharInsertion && y == 0 && (int64_t)this->sa[this->sa.size() - 1] == this->insertion_pos - 1)
                {
                    return this->insertion_pos - 1;
                }
                else
                {
                    int64_t sa_yp_minus = NaiveOperations::get_array_pos_minus(this->sa, y_plus);
                    int64_t SA_j_g = NaiveOperations::compute_next_SA_in_dynamic_LF_order2(y_plus, sa_yp_minus, this->bwt, this->sa);
                    if (SA_j_g > 0)
                    {
                        return SA_j_g - 1;
                    }
                    else
                    {
                        return this->updated_text.size() - 1;
                    }
                }
            }

            // Dynamic LF methods

            std::vector<uint64_t> construct_naive_dynamic_LF_array(const std::vector<uint64_t> &next_isa) const
            {
                // std::vector<uint64_t> suffix_array = this->create_suffix_array();

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

            uint64_t dynamic_LF(uint64_t i) const
            {
                uint64_t lf = NaiveOperations::rank(this->bwt, i, this->bwt[i]) + NaiveOperations::lex_count(this->bwt, this->bwt[i]) - 1;
                if (this->get_mode() == CharInsertion)
                {
                    uint8_t old_char = NaiveOperations::get_previous_character(this->text, this->insertion_pos);
                    uint64_t q = this->isa[this->insertion_pos + this->inserted_string.size()];
                    bool b = old_char < this->bwt[i] || (q <= i && this->bwt[i] == old_char);
                    return lf + (b ? 1 : 0);
                }
                else
                {
                    return lf;
                }
            }
            std::vector<uint64_t> construct_dynamic_LF_array() const
            {
                std::vector<uint64_t> test_array;
                for (uint64_t i = 0; i < this->bwt.size(); i++)
                {
                    test_array.push_back(this->dynamic_LF(i));
                }
                return test_array;
            }

            // Build, copy, and update methods
            void build(const std::string &_text, uint64_t _insertion_pos, const std::string &_inserted_string)
            {
                text = std::string(_text.begin(), _text.end());
                inserted_string = std::string(_inserted_string.begin(), _inserted_string.end());
                insertion_pos = _insertion_pos;

                this->updated_text = text.substr(0, insertion_pos) + inserted_string + text.substr(insertion_pos);

                for (uint64_t i = 0; i < text.size(); i++)
                {
                    conceptual_matrix.push_back(NaiveOperations::get_circular_string(text, i));
                }
                NaiveOperations::sort_conceptual_matrix(this->conceptual_matrix);
                this->j = this->updated_text.size();

                sa = NaiveOperations::create_suffix_array_from_conceptual_matrix(this->conceptual_matrix, this->end_marker);
                isa = NaiveOperations::create_inverse_suffix_array_from_conceptual_matrix(this->conceptual_matrix, this->end_marker);
                bwt = NaiveOperations::construct_bwt_from_conceptual_matrix(this->conceptual_matrix);
            }
            void update()
            {
                this->j--;
                int64_t p = this->get_starting_position_of_old_circular_string(this->j);
                std::string new_circular_string = NaiveOperations::get_circular_string(this->updated_text, this->j);

                if (p != -1)
                {
                    std::string old_circular_string = NaiveOperations::get_circular_string(this->text, p);
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
                NaiveOperations::sort_conceptual_matrix(this->conceptual_matrix);

                sa = NaiveOperations::create_suffix_array_from_conceptual_matrix(this->conceptual_matrix, this->end_marker);
                isa = NaiveOperations::create_inverse_suffix_array_from_conceptual_matrix(this->conceptual_matrix, this->end_marker);
                bwt = NaiveOperations::construct_bwt_from_conceptual_matrix(this->conceptual_matrix);
            }
            const DynamicRIndexSnapShotForInsertion copy() const
            {
                DynamicRIndexSnapShotForInsertion snap_shot;
                snap_shot.text = text;
                snap_shot.updated_text = updated_text;
                snap_shot.inserted_string = inserted_string;
                snap_shot.insertion_pos = insertion_pos;
                snap_shot.conceptual_matrix = conceptual_matrix;
                snap_shot.j = j;
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

            // Naive methods
            int64_t naive_compute_isa_i_minus() const
            {
                return NaiveOperations::get_array_pos_minus(this->isa, this->insertion_pos);
            }
            int64_t naive_compute_isa_p() const
            {
                ModeForInsertion mode = this->get_mode();
                if (mode == Preprocessing)
                {
                    if (this->is_last_preprocessing_mode())
                    {
                        int64_t p = this->insertion_pos + this->inserted_string.size();
                        int64_t q = this->isa[p];
                        return q;
                    }
                    else
                    {
                        return this->isa[this->insertion_pos];
                    }
                }
                else
                {
                    int64_t p = this->insertion_pos + this->inserted_string.size();
                    int64_t q = this->isa[p];
                    return q;
                }
            }

            int64_t naive_compute_x() const
            {
                ModeForInsertion mode = this->get_mode();
                if (mode == Preprocessing)
                {
                    return this->isa[this->j - this->inserted_string.size() - 1];
                }
                else if (mode == CharInsertion)
                {
                    return -1;
                }
                else if (mode == PostPreprocessing)
                {
                    return this->isa[this->j - 1];
                }
                else
                {
                    return -1;
                }
            }
            int64_t naive_compute_y() const
            {
                ModeForInsertion mode = this->get_mode();
                if (mode != Finished)
                {
                    auto copy = this->copy();
                    copy.update();
                    return copy.isa[this->j - 1];
                }
                else
                {
                    return -1;
                }
            }

            // Print methods

            void print_conceptual_matrix(uint64_t i, int64_t sa_v, std::string circular_shift, bool index_begin_with_1 = false) const
            {
                int64_t p = this->get_starting_position_of_old_circular_string(this->j - 1);

                if (sa_v == (int64_t)this->j)
                {
                    std::cout << "\x1b[43m";
                }
                else if (sa_v == p)
                {
                    std::cout << "\x1b[47m";
                }
                std::cout << NaiveOperations::value_with_index(i, index_begin_with_1) << " \t";
                std::cout << NaiveOperations::value_with_index(sa_v, index_begin_with_1) << "\t";

                if (circular_shift.size() == this->updated_text.size())
                {
                    for (int64_t j = 0; j < (int64_t)circular_shift.size(); j++)
                    {
                        int64_t pos = sa_v + j;
                        if (pos >= (int64_t)this->updated_text.size())
                        {
                            pos -= this->updated_text.size();
                        }

                        if (pos >= this->insertion_pos && pos < this->insertion_pos + (int64_t)this->inserted_string.size())
                        {
                            std::cout << "\x1b[31m";
                            std::cout << circular_shift[j];
                            std::cout << "\x1b[39m";
                        }
                        else
                        {
                            std::cout << circular_shift[j];
                        }
                    }
                }
                else
                {
                    std::cout << circular_shift;
                }
                for (uint64_t j = circular_shift.size(); j < this->updated_text.size(); j++)
                {
                    std::cout << " ";
                }

                std::cout << " \t";

                std::cout << circular_shift[circular_shift.size() - 1] << "\t";

                if (this->j != 0)
                {
                    std::cout << NaiveOperations::value_with_index(this->dynamic_LF(i), index_begin_with_1) << "\t";
                }

                if (sa_v == (int64_t)this->j || sa_v == p)
                {
                    std::cout << "\x1b[49m" << std::flush;
                }
                std::cout << std::endl;
            }

            void print_conceptual_matrix(bool index_begin_with_1 = false) const
            {

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
                    print_conceptual_matrix(i, this->sa[i], conceptual_matrix[i], index_begin_with_1);
                }
                std::cout << "--------------------------------" << std::endl;
            }
            void print_information(bool index_begin_with_1 = false) const
            {

                if (this->j > 0)
                {
                    int64_t _x = this->naive_compute_x();
                    int64_t _y = this->naive_compute_y();

                    int64_t sa_x_minus = _x == -1 ? -1 : NaiveOperations::get_array_pos_minus(this->sa, _x);
                    int64_t sa_x_plus = _x == -1 ? -1 : NaiveOperations::get_array_pos_plus(this->sa, _x);

                    auto next_snapshot = this->copy();
                    next_snapshot.update();

                    int64_t sa_y_minus = _y == -1 ? -1 : NaiveOperations::get_array_pos_minus(next_snapshot.sa, _y);
                    int64_t sa_y_plus = _y == -1 ? -1 : NaiveOperations::get_array_pos_plus(next_snapshot.sa, _y);

                    if (index_begin_with_1)
                    {
                        _x = _x != -1 ? _x + 1 : -1;
                        _y = _y != -1 ? _y + 1 : -1;
                        sa_x_minus = sa_x_minus != -1 ? sa_x_minus + 1 : -1;
                        sa_x_plus = sa_x_plus != -1 ? sa_x_plus + 1 : -1;
                        sa_y_minus = sa_y_minus != -1 ? sa_y_minus + 1 : -1;
                        sa_y_plus = sa_y_plus != -1 ? sa_y_plus + 1 : -1;
                    }

                    std::cout << "---- Information ------------------------" << std::endl;
                    std::cout << "j = " << this->j << std::endl;
                    std::cout << "x = " << _x << std::endl;
                    std::cout << "y = " << _y << std::endl;
                    std::cout << "SA_{j}[x-1] = " << sa_x_minus << std::endl;
                    std::cout << "SA_{j}[x+1] = " << sa_x_plus << std::endl;
                    std::cout << "SA_{j-1}[y-1] = " << sa_y_minus << std::endl;
                    std::cout << "SA_{j-1}[y+1] = " << sa_y_plus << std::endl;
                }

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
                    print_conceptual_matrix(i, this->sa[i], conceptual_matrix[i], index_begin_with_1);
                }
                std::cout << "--------------------------------" << std::endl;
            }

            // Verify methods
            void verify_dynamic_LF(const DynamicRIndexSnapShotForInsertion &next) const
            {
                std::vector<uint64_t> next_isa = next.isa;
                std::vector<uint64_t> correct_array = construct_naive_dynamic_LF_array(next_isa);
                // std::string bwt = construct_bwt();
                std::vector<uint64_t> test_array = construct_dynamic_LF_array();
                try
                {
                    stool::EqualChecker::equal_check(correct_array, test_array, "dynamic LF check");
                }
                catch (const std::exception &e)
                {
                    std::cout << "Error: " << e.what() << std::endl;

                    std::cout << "j = " << this->j << std::endl;
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
            void verify_RLE_update(DynamicRIndexSnapShotForInsertion *prev) const
            {
                ModeForInsertion mode = this->get_mode();

                if (mode == Preprocessing || mode == CharInsertion || (mode == PostPreprocessing && prev->get_mode() == CharInsertion))
                {
                    int64_t _naive_isa_i_minus = this->naive_compute_isa_i_minus();
                    int64_t _isa_i_minus = this->compute_isa_i_minus(prev);
                    if (_naive_isa_i_minus != _isa_i_minus)
                    {
                        std::cout << "naive_isa_i_minus = " << _naive_isa_i_minus << ", isa_i_minus = " << _isa_i_minus << std::endl;
                        std::cout << "mode = " << mode << "/" << prev->get_mode() << std::endl;
                        throw std::runtime_error("Error: naive_isa_i_minus != isa_i_minus");
                    }
                }

                if (mode == Preprocessing || mode == CharInsertion)
                {
                    int64_t _naive_isa_p = this->naive_compute_isa_p();
                    int64_t _isa_p = this->compute_isa_p(prev);
                    if (_naive_isa_p != _isa_p)
                    {
                        std::cout << "naive_isa_p = " << _naive_isa_p << ", isa_p = " << _isa_p << std::endl;
                        std::cout << "mode = " << mode << "/" << prev->get_mode() << std::endl;
                        throw std::runtime_error("Error: naive_isa_p != isa_p");
                    }
                }

                if (mode == Preprocessing || mode == PostPreprocessing)
                {
                    int64_t _naive_x = this->naive_compute_x();
                    int64_t _x = this->compute_x(prev);

                    if (_naive_x != _x)
                    {
                        std::cout << "naive_x = " << _naive_x << ", x = " << _x << std::endl;
                        throw std::runtime_error("Error: naive_x != x");
                    }
                }

                if (mode != Finished)
                {
                    int64_t _naive_y = this->naive_compute_y();
                    int64_t _y = this->compute_y(prev);

                    if (_naive_y != _y)
                    {
                        std::cout << "naive_y = " << _naive_y << ", y = " << _y << std::endl;
                        throw std::runtime_error("Error: naive_y != y");
                    }
                }
            }

            void verify_SA_update(DynamicRIndexSnapShotForInsertion *prev, const std::vector<uint64_t> &original_SA) const
            {
                ModeForInsertion mode = this->get_mode();
                if (mode == PostPreprocessing || this->is_last_preprocessing_mode())
                {
                    int64_t _naive_inverse_phi_i = NaiveOperations::inverse_phi(original_SA, this->insertion_pos);
                    int64_t _naive_inverse_phi_i_plus = NaiveOperations::inverse_phi(original_SA, this->insertion_pos + 1);


                    int64_t x = this->naive_compute_x();
                    assert(x != -1);
                    int64_t _naive_SA1_plus = NaiveOperations::get_array_pos_plus(this->sa, x);
                    int64_t _SA1_plus = prev->compute_next_SA1_plus(_naive_inverse_phi_i, _naive_inverse_phi_i_plus, *this);
                    if (_naive_SA1_plus != _SA1_plus)
                    {
                        std::cout << "naive_SA1_plus = " << _naive_SA1_plus << ", SA1_plus = " << _SA1_plus << std::endl;
                        throw std::runtime_error("Error: naive_SA1_plus != SA1_plus");
                    }

                    int64_t _naive_SA1_minus = NaiveOperations::get_array_pos_minus(this->sa, x);
                    int64_t _SA1_minus = prev->compute_next_SA1_minus(_naive_SA1_minus, *this);
                    if (_naive_SA1_minus != _SA1_minus)
                    {
                        std::cout << "naive_SA1_minus = " << _naive_SA1_minus << ", SA1_minus = " << _SA1_minus << std::endl;
                        throw std::runtime_error("Error: naive_SA1_minus != SA1_minus");
                    }
                }
                if (prev != nullptr)
                {
                    if (this->is_last_preprocessing_mode() || mode == CharInsertion || mode == PostPreprocessing)
                    {
                        int64_t _y = this->naive_compute_y();
                        assert(_y != -1);
                        auto next_snapshot = this->copy();
                        next_snapshot.update();

                        int64_t _naive_SA2_plus = NaiveOperations::get_array_pos_plus(next_snapshot.sa, _y);
                        int64_t y_plus = prev->naive_compute_y();
                        int64_t _SA2_plus = this->compute_next_SA2_plus(y_plus);
                        if (_naive_SA2_plus != _SA2_plus)
                        {
                            std::cout << "naive_SA2_plus = " << _naive_SA2_plus << ", SA2_plus = " << _SA2_plus << std::endl;
                            throw std::runtime_error("Error: naive_SA2_plus != SA2_plus");
                        }

                        int64_t _naive_SA2_minus = NaiveOperations::get_array_pos_minus(next_snapshot.sa, _y);
                        int64_t _SA2_minus = this->compute_next_SA2_minus(y_plus);
                        if (_naive_SA2_minus != _SA2_minus)
                        {
                            std::cout << "y: " << _y << ", y_plus: " << y_plus << std::endl;
                            std::cout << "naive_SA2_minus = " << _naive_SA2_minus << ", SA2_minus = " << _SA2_minus << std::endl;
                            std::cout << "naive_SA2_plus = " << _naive_SA2_plus << ", SA2_plus = " << _SA2_plus << std::endl;

                            this->print_conceptual_matrix(true);
                            next_snapshot.print_conceptual_matrix(true);
                            throw std::runtime_error("Error: naive_SA2_minus != SA2_minus");
                        }
                    }
                }
            }

            // Test methods

            static void insertion_test(std::string text, uint64_t insertion_pos, std::string inserted_string, bool demo = false, bool index_begin_with_1 = false)
            {
                std::vector<DynamicRIndexSnapShotForInsertion> snap_shots = DynamicRIndexSnapShotForInsertion::get_all_snap_shots(text, insertion_pos, inserted_string);

                // std::cout << "insertion range = [" << insertion_pos << ", " << (insertion_pos + inserted_string.size() - 1) << "]" << std::endl;

                for (uint64_t i = 0; i < snap_shots.size() - 1; i++)
                {
                    if (demo)
                    {
                        snap_shots[i].print_conceptual_matrix(index_begin_with_1);
                    }
                    stool::dynamic_r_index::DynamicRIndexSnapShotForInsertion *prev = i == 0 ? nullptr : &snap_shots[i - 1];

                    if (i < snap_shots.size() - 1)
                    {
                        snap_shots[i].verify_RLE_update(prev);
                        snap_shots[i].verify_dynamic_LF(snap_shots[i + 1]);
                        snap_shots[i].verify_SA_update(prev, snap_shots[0].sa);
                    }
                }
                // std::cout << std::endl;
            }

            static void insertion_test(uint64_t text_size, uint64_t insertion_length, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = fm_index_test::DynamicFMIndexTest::create_alphabet_with_end_marker(chars, '$');

                // dfmi.initialize(alphabet_with_end_marker);

                std::vector<uint8_t> _text = r_index_test::DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);
                std::vector<uint8_t> _pattern = r_index_test::DynamicRIndexTest::create_text(insertion_length, chars, alphabet_with_end_marker[0], mt64);
                _pattern.pop_back();

                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, text_size - 1);
                uint64_t insertion_pos = get_rand_uni_int(mt64);

                std::string text = std::string(_text.begin(), _text.end());
                std::string pattern = std::string(_pattern.begin(), _pattern.end());

                DynamicRIndexSnapShotForInsertion::insertion_test(text, insertion_pos, pattern);
            }
            static void insertion_test(uint64_t text_size, uint64_t insertion_length, uint8_t alphabet_type, uint64_t number_of_trials, uint64_t seed)
            {
                std::cout << "Test started." << std::endl;
                for (uint64_t i = 0; i < number_of_trials; i++)
                {
                    std::cout << "+" << std::flush;
                    insertion_test(text_size, insertion_length, alphabet_type, seed++);
                }
                std::cout << "[DONE]" << std::endl;
                std::cout << std::endl;
            }
        };

    }
}

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

            uint64_t SPECIAL_GAP = 10000;

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

            const DynamicRIndexSnapShotForDeletion copy() const
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
            int64_t compute_naive_x() const {
                if(this->t != 0){
                    uint64_t p = SPECIAL_GAP + this->t-1;
                    for(uint64_t i = 0; i < this->conceptual_matrix.size(); i++){
                        if(this->sa[i] == p){
                            return i;
                        }
                    }
                    return -1;
                }else{
                    return -1;
                }             
            }
            int64_t compute_naive_y() const {
                uint64_t low_p = this->deletion_pos + 1;
                uint64_t high_p = low_p + this->deletion_length ;
                if(this->t >= low_p && this->t <= high_p){
                    return -1;
                }
                else if(this->t == 0){
                    return -1;
                }else{
                    uint64_t value = 0;
                    if(this->t < low_p){
                        value = this->t -1;
                    }else{
                        value = this->t - 1 - this->deletion_length;
                    }

                    auto copy = this->copy();
                    copy.update();
                    return copy.isa[value];
                }
            }

            void print_conceptual_matrix(bool index_begin_with_1 = false) const
            {
                std::vector<uint64_t> sa = this->create_suffix_array();
                int64_t naive_x = this->compute_naive_x();
                uint64_t low_p = this->deletion_pos + 1;
                uint64_t high_p = low_p + this->deletion_length ;


                std::cout << "---- Information ------------------------" << std::endl;
                if (this->t != 0)
                {
                    std::cout << "i \tSA_{" << this->t << "}\tCM_{" << this->t << "}\t\tL_{" << this->t << "}\tLF_{" << this->t << "}" << std::endl;
                }
                else
                {
                    std::cout << "i \tSA_{" << this->t << "}\tCM_{" << this->t << "}\t\tL_{" << this->t << "}" << std::endl;
                }
                for (uint64_t i = 0; i < conceptual_matrix.size(); i++)
                {
                    int64_t color_mode = 0;
                    if((int64_t)i == naive_x){
                        color_mode = 2;
                    }else if(this->t >= high_p - 1 && this->sa[i] == (this->t - this->deletion_length)){
                        color_mode = 1;
                    }else if(this->t < low_p - 1 && this->sa[i] == this->t){
                        color_mode = 1make;
                    }
                    //std::cout << sa[i] << "\t" << conceptual_matrix[i] << std::endl;
                    print_conceptual_matrix(i, this->sa[i], conceptual_matrix[i], index_begin_with_1, color_mode);
                }
                std::cout << "--------------------------------" << std::endl;
                
                /*
                std::cout << this->t << "--------------------------------" << std::endl;
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
                if(index_begin_with_1){
                    _i++;
                    modified_sa_v++;
                }

                std::string modified_sa_v_str = sa_v >= (int64_t)SPECIAL_GAP ? (std::to_string(modified_sa_v) + "*") : std::to_string(modified_sa_v);


                if(this->t != 0){
                    int64_t lf = this->dynamic_LF(i) == UINT64_MAX ? -1 : this->dynamic_LF(i);
                    if(index_begin_with_1){
                        lf++;
                    }
                    std::cout << _i << "\t" << modified_sa_v_str << "\t" << circular_shift << "\t\t" << this->bwt[i] << "\t" << lf;
                }
                else{
                    std::cout << _i << "\t" << modified_sa_v_str << "\t" << circular_shift << "\t\t" << this->bwt[i];
                }

                if(color_mode > 0){

                    std::cout << "\x1b[49m" << std::flush;
                }
                std::cout << std::endl;
                /*
                int64_t p = this->get_starting_position_of_old_circular_string(this->j - 1);

                if (sa_v == (int64_t)this->j)
                {
                    std::cout << "\x1b[43m";
                }
                else if (sa_v == p)
                {
                    std::cout << "\x1b[47m";
                }
                std::cout << DynamicRIndexSnapShotForInsertion::value_with_index(i, index_begin_with_1) << " \t";
                std::cout << DynamicRIndexSnapShotForInsertion::value_with_index(sa_v, index_begin_with_1) << "\t";


                if (circular_shift.size() == this->updated_text.size())
                {
                    for (uint64_t j = 0; j < circular_shift.size(); j++)
                    {        
                        uint64_t pos = sa_v + j;
                        if (pos >= this->updated_text.size())
                        {
                            pos -= this->updated_text.size();
                        }

                        if (pos >= this->deletion_pos && pos < this->deletion_pos + this->deletion_length)
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
                for(uint64_t j = circular_shift.size(); j < this->updated_text.size(); j++){
                    std::cout << " ";
                }

                std::cout << " \t";

                std::cout << circular_shift[circular_shift.size() - 1] << "\t";

                if(this->t != 0){
                    std::cout << DynamicRIndexSnapShotForInsertion::value_with_index(this->dynamic_LF(i), index_begin_with_1) << "\t";
                }

                if (sa_v == (int64_t)this->t || sa_v == p)
                {
                    std::cout << "\x1b[49m" << std::flush;
                }
                std::cout << std::endl;
                */
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

                for (uint64_t i = 0; i < snap_shots.size() - 1; i++)
                {
                    if (view)
                    {                        
                        std::cout << "t = " << snap_shots[i].t << std::endl;
                        std::cout << "new_char = " << (char)snap_shots[i].get_new_character() << std::endl;
                        std::cout << "replace_pos = " << snap_shots[i].isa[snap_shots[i].deletion_pos] << std::endl;
                        std::vector<uint64_t> next_isa = snap_shots[i + 1].isa;
                        std::vector<uint64_t> lf_array = snap_shots[i].construct_dynamic_LF_array(next_isa);
                        std::cout << "bwt: " << snap_shots[i].bwt << std::endl;
                        stool::DebugPrinter::print_integers(snap_shots[i].sa, "sa");
                        stool::DebugPrinter::print_integers(lf_array, "lf_array");

                        snap_shots[i].print_conceptual_matrix();

                        std::cout << std::endl;
                    }

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

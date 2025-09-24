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

            uint64_t j = 0;

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
                uint64_t lf = NaiveOperations::rank(this->bwt, i, this->bwt[i]) + NaiveOperations::lex_count(this->bwt, this->bwt[i]) - 1;
                if (this->j > this->insertion_pos && this->j <= this->insertion_pos + this->inserted_string.size())
                {
                    uint8_t old_char = this->get_old_character();
                    uint64_t positionToReplace = this->isa[this->insertion_pos + this->inserted_string.size()];
                    bool b = old_char < this->bwt[i] || (positionToReplace <= i && this->bwt[i] == old_char);
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
                    conceptual_matrix.push_back(NaiveOperations::get_circular_string(text, i));
                }
                this->sort_conceptual_matrix();
                this->j = this->updated_text.size();

                sa = create_suffix_array();
                isa = create_inverse_suffix_array();
                bwt = construct_bwt();
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

            static int64_t value_with_index(int64_t value, bool index_begin_with_1 = false)
            {
                return index_begin_with_1 ? value + 1 : value;
            }

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

                        if (pos >= this->insertion_pos && pos < this->insertion_pos + this->inserted_string.size())
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

                if(this->j != 0){
                    std::cout << DynamicRIndexSnapShotForInsertion::value_with_index(this->dynamic_LF(i), index_begin_with_1) << "\t";
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

                if(this->j > 0){
                    int64_t _x = this->naive_compute_x();
                    int64_t _y = this->naive_compute_y();
    
                    int64_t sa_x_minus = this->naive_compute_SA1_minus();
                    int64_t sa_x_plus = this->naive_compute_SA1_plus();
                    int64_t sa_y_minus = this->naive_compute_SA2_minus();
                    int64_t sa_y_plus = this->naive_compute_SA2_plus();
    
                    if(index_begin_with_1){
                        _x = _x != - 1 ? _x + 1 : -1;
                        _y = _y != - 1 ? _y + 1 : -1;
                        sa_x_minus = sa_x_minus != - 1 ? sa_x_minus + 1 : -1;
                        sa_x_plus = sa_x_plus != - 1 ? sa_x_plus + 1 : -1;
                        sa_y_minus = sa_y_minus != - 1 ? sa_y_minus + 1 : -1;
                        sa_y_plus = sa_y_plus != - 1 ? sa_y_plus + 1 : -1;
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


            int64_t naive_compute_x() const {
                uint64_t p = this->insertion_pos + this->inserted_string.size();
                if(this->j > p){
                    return this->isa[this->j-this->inserted_string.size()-1];
                }else if(this-> j > this->insertion_pos){
                    return -1;
                }else if(this->j > 0){
                    return this->isa[this->j - 1];
                }else{
                    return -1;
                }
            }
            int64_t compute_x(DynamicRIndexSnapShotForInsertion *prev) const {
                uint64_t high_p = this->insertion_pos + this->inserted_string.size();
                if(this->j == this->text.size() + this->inserted_string.size()){
                    return 0;
                }else if(this-> j > this->insertion_pos && this->j <= high_p){
                    return -1;
                }
                else if(this->j > 0 && this->j == this->insertion_pos){
                    return this->isa[this->insertion_pos-1];
                }
                else if(this->j > 0){
                    if(prev == nullptr){
                        throw std::runtime_error("prev is nullptr");
                    }
                    int64_t prev_x = prev->naive_compute_x();
                    uint64_t lf = NaiveOperations::rank(prev->bwt, prev_x, prev->bwt[prev_x]) + NaiveOperations::lex_count(prev->bwt, prev->bwt[prev_x]) - 1;
                    return lf;
                }else{
                    return -1;
                }

            }

            int64_t naive_compute_y() const {
                if(this->j > 0){
                    auto copy = this->copy();
                    copy.update();
                    return copy.isa[this->j - 1];    
                }else{
                    return -1;
                }
            }
            int64_t compute_y(DynamicRIndexSnapShotForInsertion *prev) const {
                uint64_t p = this->insertion_pos + this->inserted_string.size();
                if(this->j > p){
                    return this->compute_x(prev);
                }
                else if(this->j == 0){
                    return -1;
                }else if(this-> j > 0 && this->j <= this->insertion_pos){
                    int64_t prev_y = prev->naive_compute_y();
                    uint64_t lf = NaiveOperations::rank(this->bwt, prev_y, this->bwt[prev_y]) + NaiveOperations::lex_count(this->bwt, this->bwt[prev_y]) - 1;
                    return lf;
                }else{
                    int64_t prev_y = prev->naive_compute_y();
                    uint64_t lf = NaiveOperations::rank(this->bwt, prev_y, this->bwt[prev_y]) + NaiveOperations::lex_count(this->bwt, this->bwt[prev_y]) - 1;
                    int64_t q = this->isa[p];
                    bool b = this->text[this->insertion_pos-1] < this->bwt[prev_y] || (this->text[this->insertion_pos-1] == this->bwt[prev_y] && q <= prev_y);
                    if(b){
                        return lf + 1;
                    }else{
                        return lf;
                    }
                }
            }

            void verify_RLE_update(DynamicRIndexSnapShotForInsertion *prev) const {
                int64_t _naive_x = this->naive_compute_x();
                int64_t _x = this->compute_x(prev);

                if(_naive_x != _x){
                    std::cout << "naive_x = " << _naive_x << ", x = " << _x << std::endl;
                    throw std::runtime_error("Error: naive_x != x");
                }
                int64_t _naive_y = this->naive_compute_y();
                int64_t _y = this->compute_y(prev);

                if(_naive_y != _y){
                    std::cout << "naive_y = " << _naive_y << ", y = " << _y << std::endl;
                    throw std::runtime_error("Error: naive_y != y");
                }
            }
            int64_t naive_compute_SA1_minus() const {
                int64_t x = this->naive_compute_x();
                if(x == -1){
                    return -1;
                }else{
                    if(x == 0){
                        return this->sa[this->sa.size() - 1];
                    }else{
                        return this->sa[x-1];
                    }
                }
            }
            uint64_t naive_compute_SA1_plus() const {
                int64_t x = this->naive_compute_x();
                if(x == -1){
                    return -1;
                }else{
                    if(x == (int64_t)this->sa.size() - 1){
                        return this->sa[0];
                    }else{
                        return this->sa[x+1];
                    }
                }
            }
            uint64_t naive_compute_SA2_minus() const {
                assert(this->j > 0);
                auto copy = this->copy();
                copy.update();
                int64_t y = this->naive_compute_y();
                if(y == -1){
                    return -1;
                }else{
                    if(y == 0){
                        return copy.sa[copy.sa.size() - 1];
                    }else{
                        return copy.sa[y-1];
                    } 
                }
            }
            uint64_t naive_compute_SA2_plus() const {
                assert(this->j > 0);
                auto copy = this->copy();
                copy.update();
                int64_t y = this->naive_compute_y();
                if(y == -1){
                    return -1;
                }else{
                    if(y == (int64_t)copy.sa.size() - 1){
                        return copy.sa[0];
                    }else{
                        return copy.sa[y+1];
                    }
                }
            }

            
            
           
            uint8_t get_c_succ(uint8_t c_v) const{
                uint16_t c_succ = UINT16_MAX;
                for(uint64_t i = 0; i < this->bwt.size(); i++){
                    if(this->bwt[i] > c_v && (uint16_t)this->bwt[i] < c_succ){
                        c_succ = this->bwt[i];
                    }
                }
                if(c_succ == UINT16_MAX){
                    c_succ = '$';
                }

                return c_succ;
            }
            int64_t compute_sa_s_at_min_U(std::vector<std::pair<uint8_t, uint64_t>> &rle, std::vector<uint64_t> &SA_s, uint64_t v) const {
                int64_t s = -1;


                for(int64_t i = v+1; i < rle.size(); i++){
                    if(rle[i].first == rle[v].first){
                        s = SA_s[i];
                        break;
                    }
                }
                return s;
            }
            int64_t compute_sa_s_at_min_U_prime(std::vector<std::pair<uint8_t, uint64_t>> &rle, std::vector<uint64_t> &SA_s, uint8_t c_succ, uint64_t v) const {
                int64_t s = -1;
                for(int64_t i = 0; i < v; i++){
                    if(rle[i].first == c_succ){
                        s = SA_s[i];
                        break;
                    }
                }
                return s;
            }


            int64_t compute_SA1_plus(DynamicRIndexSnapShotForInsertion *prev) const {
                uint64_t high_p = this->insertion_pos + this->inserted_string.size();

                if(prev == nullptr){
                    return this->naive_compute_SA1_plus();
                }
                else if(this->j > this->insertion_pos && this->j <= high_p){
                    return -1;
                }
                else if(this->j == this->insertion_pos){
                    return this->naive_compute_SA1_plus();
                }
                else{
                    int64_t x = prev->naive_compute_x();
                    auto rle = NaiveOperations::get_RLE(prev->bwt);
                    int64_t v = NaiveOperations::get_rle_index_by_position(rle, x);
                    uint8_t c_v = rle[v].first;
                    std::cout << "x = " << x << ", v = " << v << ", c_v = " << c_v << " / ";

                    assert(v != -1 && v < rle.size());
                    uint64_t t_v = NaiveOperations::get_starting_position(rle, v);
                    uint64_t ell_v = rle[v].second;
                    auto SA_s = NaiveOperations::get_SA_s(prev->bwt, prev->sa);
                    uint8_t c_succ = prev->get_c_succ(c_v);
                    int64_t min_U = prev->compute_sa_s_at_min_U(rle, SA_s, v);
                    int64_t min_U_prime = prev->compute_sa_s_at_min_U_prime(rle, SA_s, c_succ, v);
    
                    int64_t r = 0;

                    for(uint64_t i = 0; i < rle.size(); i++){
                        std::cout << "[" << (char)rle[i].first << " " << rle[i].second << "]";
                    }
                    std::cout << std::endl;

                    std::cout << "x = " << x << ", v = " << v << ", c_v = " << c_v << ", t_v = " << t_v << ", ell_v = " << ell_v << " / " << min_U << std::endl;


                    if(x != t_v + ell_v - 1){
                        r = prev->sa[x+1];
                        std::cout << "Found1: " << r << std::endl;
                    }else if(x == t_v + ell_v - 1 && min_U != -1){
                        r = min_U;
                        std::cout << "Found2: " << r << std::endl;
                    }else{
                        r = min_U_prime;
                        std::cout << "Found3: " << r << std::endl;

                    }
                    if(r > 0){
                        r--;
                    }else{
                        r = prev->updated_text.size() -1;
                    }

                    return r;
    
                }

            }



            void verify_SA_update([[maybe_unused]] DynamicRIndexSnapShotForInsertion *prev) const {
                int64_t _naive_SA1_plus = this->naive_compute_SA1_plus();
                int64_t _SA1_plus = this->compute_SA1_plus(prev);
                if(_naive_SA1_plus != _SA1_plus){
                    std::cout << "naive_SA1_plus = " << _naive_SA1_plus << ", SA1_plus = " << _SA1_plus << std::endl;
                    throw std::runtime_error("Error: naive_SA1_plus != SA1_plus");
                }

            }

            /*
            static std::vector<DynamicRIndexSnapShotForInsertion> create_insertion_demo(std::string text, uint64_t insertion_pos, std::string inserted_string, bool index_begin_with_1 = false)
            {
                std::vector<DynamicRIndexSnapShotForInsertion> snap_shots = DynamicRIndexSnapShotForInsertion::get_all_snap_shots(text, insertion_pos, inserted_string);
            }
            */

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
    
                    if(i < snap_shots.size() - 1){
                        snap_shots[i].verify_RLE_update(prev);
                        snap_shots[i].verify_dynamic_LF(snap_shots[i + 1]);
                        snap_shots[i].verify_SA_update(prev);
                    }
    
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
                std::vector<uint8_t> _pattern = r_index_test::DynamicRIndexTest::create_text(insertion_length, chars, alphabet_with_end_marker[0], mt64);
                _pattern.pop_back();

                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, text_size - 1);
                uint64_t insertion_pos = get_rand_uni_int(mt64);

                std::string text = std::string(_text.begin(), _text.end());
                std::string pattern = std::string(_pattern.begin(), _pattern.end());

                DynamicRIndexSnapShotForInsertion::insertion_test(text, insertion_pos, pattern);
            }
            static void insertion_test(uint64_t text_size, uint64_t insertion_length, uint8_t alphabet_type, uint64_t number_of_trials, uint64_t seed){
                std::cout << "Test started." << std::endl;
                for(uint64_t i = 0; i < number_of_trials; i++){
                    std::cout << "+" << std::flush;
                    insertion_test(text_size, insertion_length, alphabet_type, seed++);
                }
                std::cout << "[DONE]" << std::endl;
                std::cout << std::endl;

            }
        };

    }
}

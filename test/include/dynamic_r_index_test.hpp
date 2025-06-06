#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "libdivsufsort/sa.hpp"
#include "stool/include/stool.hpp"
#include "../../include/dynamic_r_index.hpp"
#include "./dynamic_fm_index_test.hpp"

namespace stool
{
    namespace r_index_test
    {
        using namespace dynamic_r_index;
        using namespace fm_index_test;
        class BWT_and_SA
        {
        public:
            std::vector<uint8_t> bwt;
            std::vector<uint64_t> sa;

            static void print(std::string name, std::vector<uint8_t> &bwt)
            {
                std::string s;
                for (auto c : bwt)
                {
                    s.push_back(c);
                }
                std::cout << name << ": " << s << std::endl;
            }
            static std::vector<BWT_and_SA> build_for_deletion(std::vector<uint8_t> &bwt, std::vector<uint64_t> &sa, dynamic_r_index::FMIndexEditHistory &edit_history)
            {
                std::vector<BWT_and_SA> output;
                output.resize(edit_history.move_history.size() + 1);
                BWT_and_SA &bwt_sa = output[0];
                uint64_t insertion_pos = sa[edit_history.inserted_sa_index];
                for (auto c : bwt)
                {
                    bwt_sa.bwt.push_back(c);
                }
                for (auto it : sa)
                {
                    bwt_sa.sa.push_back(it < insertion_pos ? it : it - 1);
                }
                uint8_t oldChar = bwt_sa.bwt[edit_history.inserted_sa_index];
                bwt_sa.bwt[edit_history.replaced_sa_index] = oldChar;
                bwt_sa.bwt.erase(bwt_sa.bwt.begin() + edit_history.inserted_sa_index);
                bwt_sa.sa.erase(bwt_sa.sa.begin() + edit_history.inserted_sa_index);

                for (int64_t i = 0; i < (int64_t)edit_history.move_history.size(); i++)
                {
                    for (auto c : output[i].bwt)
                    {
                        output[i + 1].bwt.push_back(c);
                    }
                    for (auto it : output[i].sa)
                    {
                        output[i + 1].sa.push_back(it);
                    }
                    int64_t j = edit_history.move_history[i].first;
                    int64_t j_prime = edit_history.move_history[i].second;

                    uint8_t c = output[i + 1].bwt[j];
                    uint64_t v = output[i + 1].sa[j];
                    output[i + 1].bwt.erase(output[i + 1].bwt.begin() + j);
                    output[i + 1].bwt.insert(output[i + 1].bwt.begin() + j_prime, c);
                    output[i + 1].sa.erase(output[i + 1].sa.begin() + j);
                    output[i + 1].sa.insert(output[i + 1].sa.begin() + j_prime, v);
                }
                return output;
            }

            static std::vector<BWT_and_SA> build_for_insertion(std::vector<uint8_t> &bwt, std::vector<uint64_t> &sa, dynamic_r_index::FMIndexEditHistory &edit_history)
            {
                std::vector<BWT_and_SA> output;
                output.resize(edit_history.move_history.size() + 1);
                BWT_and_SA &bwt_sa = output[0];
                uint64_t insertion_pos = sa[edit_history.replaced_sa_index];

                for (auto c : bwt)
                {
                    bwt_sa.bwt.push_back(c);
                }
                for (auto it : sa)
                {
                    bwt_sa.sa.push_back(it < insertion_pos ? it : it + 1);
                }

                uint8_t oldChar = bwt_sa.bwt[edit_history.replaced_sa_index];
                bwt_sa.bwt[edit_history.replaced_sa_index] = edit_history.inserted_string[0];

                bwt_sa.bwt.insert(bwt_sa.bwt.begin() + edit_history.inserted_sa_index, oldChar);
                bwt_sa.sa.insert(bwt_sa.sa.begin() + edit_history.inserted_sa_index, insertion_pos);

                for (int64_t i = 0; i < (int64_t)edit_history.move_history.size(); i++)
                {
                    for (auto c : output[i].bwt)
                    {
                        output[i + 1].bwt.push_back(c);
                    }
                    for (auto it : output[i].sa)
                    {
                        output[i + 1].sa.push_back(it);
                    }
                    int64_t j = edit_history.move_history[i].first;
                    int64_t j_prime = edit_history.move_history[i].second;

                    uint8_t c = output[i + 1].bwt[j];
                    uint64_t v = output[i + 1].sa[j];
                    output[i + 1].bwt.erase(output[i + 1].bwt.begin() + j);
                    output[i + 1].bwt.insert(output[i + 1].bwt.begin() + j_prime, c);
                    output[i + 1].sa.erase(output[i + 1].sa.begin() + j);
                    output[i + 1].sa.insert(output[i + 1].sa.begin() + j_prime, v);
                }
                return output;
            }
            template <typename INDEX1>
            static void bwt_sa_and_isa_check(INDEX1 &dfmi, const dynamic_r_index::DynamicRIndex &drfmi)
            {
                std::vector<uint8_t> bwt1 = dfmi.get_bwt();
                std::vector<uint8_t> bwt2 = drfmi.get_bwt();

                try
                {
                    stool::equal_check("BWT CHECK", bwt1, bwt2);
                }
                catch (std::logic_error e)
                {
                    auto bwt1 = dfmi.get_bwt_str();
                    auto bwt2 = drfmi.get_bwt_str();
                    stool::DebugPrinter::print_integers_with_characters(bwt1, "Correct");
                    stool::DebugPrinter::print_integers_with_characters(bwt2, "Test");

                    throw e;
                }

                std::vector<uint64_t> correctSA = dfmi.get_sa();
                std::vector<uint64_t> testSA = drfmi.get_sa();

                try
                {
                    stool::equal_check("SA CHECK", correctSA, testSA);
                }
                catch (std::logic_error e)
                {
                    stool::DebugPrinter::print_integers(correctSA, "Correct");
                    stool::DebugPrinter::print_integers(testSA, "Test");
                    throw e;
                }

                std::vector<uint64_t> correctISA = dfmi.get_isa();
                std::vector<uint64_t> testISA = drfmi.get_isa();
                try
                {
                    stool::equal_check("ISA CHECK", correctISA, testISA);
                }
                catch (std::logic_error e)
                {
                    stool::DebugPrinter::print_integers(correctISA, "Correct");
                    stool::DebugPrinter::print_integers(testISA, "Test");
                    throw e;
                }
            }
        };
        class DynamicRIndexTest
        {

        public:
            static std::vector<uint8_t> create_text(uint64_t text_size, std::vector<uint8_t> chars, uint8_t end_marker, std::mt19937_64 &mt64)
            {
                std::vector<uint8_t> text;

                for (uint64_t i = 0; i < text_size; i++)
                {
                    std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);
                    uint8_t new_char = chars[get_rand_uni_char(mt64)];
                    text.push_back(new_char);
                }
                text.push_back(end_marker);
                return text;
            }
            static void verify_r_index(const dynamic_r_index::DynamicRIndex &drfmi)
            {
                drfmi.verify();
            }
            /*
            static void verify_r_index(const dynamic_r_index::DynamicRIndex &drfmi, const SubPhiDataStructure &sub)
            {
                sub.verify(drfmi.get_dynamic_phi());
                drfmi.verify(1);
            }
            */

            static void bwt_test_random_insertion(DynamicFMIndex &dfmi, dynamic_r_index::DynamicRIndex &drfmi, std::vector<uint8_t> &chars, std::mt19937_64 &mt64, bool bwt_check_flag)
            {
                uint64_t size = dfmi.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);
                uint8_t new_char = chars[get_rand_uni_char(mt64)];

                dfmi.insert_char(insertion_pos, new_char);
                drfmi.insert_char(insertion_pos, new_char);

                if (bwt_check_flag)
                {
                    DynamicRIndexTest::verify_r_index(drfmi);
                    BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);

                }
            }

            static void bwt_test_random_deletion(DynamicFMIndex &dfmi, dynamic_r_index::DynamicRIndex &drfmi, std::mt19937_64 &mt64, bool bwt_check_flag)
            {
                uint64_t size = dfmi.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 2);

                int64_t deletion_pos = get_rand_uni_int(mt64);
                dfmi.delete_char(deletion_pos);
                drfmi.delete_char(deletion_pos);

                if (bwt_check_flag)
                {
                    DynamicRIndexTest::verify_r_index(drfmi);
                    BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);
                }
            }
            static void random_character_insertion_and_deletion_test(uint64_t item_num, uint8_t alphabet_type, uint64_t seed, bool detailed_check)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                DynamicFMIndex dfmi;
                dfmi.initialize(alphabet_with_end_marker);
                DynamicRIndex drfmi = DynamicRIndex::build_r_index_of_end_marker(alphabet_with_end_marker);
                // drfmi.initialize(alphabet_with_end_marker, true);

                for (uint64_t i = 0; i < item_num; i++)
                {
                    bool b = detailed_check;
                    if (item_num % 100 == 0 || i + 1 == item_num)
                    {
                        b = true;
                    }
                    DynamicRIndexTest::bwt_test_random_insertion(dfmi, drfmi, chars, mt64, b);
                }

                while (dfmi.size() > 1)
                {
                    bool b = detailed_check;
                    if (item_num % 100 == 0 || dfmi.size() == 2)
                    {
                        b = true;
                    }
                    DynamicRIndexTest::bwt_test_random_deletion(dfmi, drfmi, mt64, b);
                }
            }

            static void F_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                std::vector<uint64_t> f_next_array;
                f_next_array.resize(sa.size());
                for (int64_t i = 0; i < (int64_t)sa.size(); i++)
                {
                    if (sa[i] + 1 < sa.size())
                    {
                        f_next_array[i] = isa[sa[i] + 1];
                    }
                    else
                    {
                        f_next_array[i] = isa[0];
                    }
                }

                dynamic_r_index::DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                const DynamicRLBWT &dbwt = drfmi.get_dynamic_rlbwt();

                for (int64_t i = 0; i < (int64_t)sa.size(); i++)
                {
                    RunPosition rp1 = dbwt.to_run_position(i);
                    RunPosition rp2 = DynamicRLBWTHelper::proper_successor_on_F(dbwt, rp1);
                    uint64_t lf1 = dbwt.LF(rp1.run_index, rp1.position_in_run);
                    uint64_t lf2 = dbwt.LF(rp2.run_index, rp2.position_in_run);
                    // uint64_t j = drfmi.dbwt.to_position(rp2);
                    if (lf1 + 1 < (uint64_t)sa.size())
                    {
                        if (lf1 + 1 != lf2)
                        {
                            throw std::runtime_error("Error:F_test(1)");
                        }
                    }
                    else
                    {
                        if (lf2 != 0)
                        {
                            throw std::runtime_error("Error:F_test(2)");
                        }
                    }
                }

                for (int64_t i = 0; i < (int64_t)sa.size(); i++)
                {

                    RunPosition rp1 = dbwt.to_run_position(i);

                    RunPosition rp2 = DynamicRLBWTHelper::proper_predecessor_on_F(dbwt, rp1);

                    uint64_t lf1 = dbwt.LF(rp1.run_index, rp1.position_in_run);
                    uint64_t lf2 = dbwt.LF(rp2.run_index, rp2.position_in_run);

                    // uint64_t j = drfmi.dbwt.to_position(rp2);
                    if (lf1 > 0)
                    {
                        if (lf2 + 1 != lf1)
                        {
                            throw std::runtime_error("Error:F_test(3)");
                        }
                    }
                    else
                    {
                        if (lf2 != (uint64_t)sa.size() - 1)
                        {
                            throw std::runtime_error("Error:F_test(4)");
                        }
                    }
                }
            }

            static void inverse_phi_test_without_updates(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);

                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);
                // stool::Printer::print("BWT", bwt);
                // stool::Printer::print("SA", sa);

                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> test_sa = drfmi.get_sa();
                std::vector<uint64_t> test_isa = drfmi.get_isa();

                // stool::Printer::print("Correct_SA", sa);
                // stool::Printer::print("Test SA   ", test_sa);

                stool::equal_check(test_sa, sa);
                stool::equal_check(test_isa, isa);

                const DynamicPhi &disa = drfmi.get_dynamic_phi();
                const DynamicRLBWT &dbwt = drfmi.get_dynamic_rlbwt();
                for (int64_t i = 0; i < (int64_t)text.size(); i++)
                {
                    int64_t test_isa_value = disa.isa(i, dbwt);
                    if ((int64_t)isa[i] != test_isa_value)
                    {
                        throw std::logic_error("Error: inverse_phi_test");
                    }
                }
            }

            static void check_sampling_sa(std::string name, const std::vector<uint64_t> &collect_sa, const std::vector<uint64_t> &sampling_sa)
            {
                /*
                std::cout << std::endl;
                stool::Printer::print("SA(collect)", collect_sa);
                stool::Printer::print("SA(test)   ", sampling_sa);
                */

                if (collect_sa.size() != sampling_sa.size())
                {
                    throw std::logic_error("Different Array Size");
                }
                else
                {
                    for (uint64_t i = 0; i < collect_sa.size(); i++)
                    {
                        if (sampling_sa[i] != UINT64_MAX && collect_sa[i] != sampling_sa[i])
                        {
                            if (collect_sa.size() < 100)
                            {
                                std::cout << std::endl;
                                stool::Printer::print("SA(collect)", collect_sa);
                                stool::Printer::print("SA(test)   ", sampling_sa);
                            }
                            throw std::logic_error("[" + name + "]" + "Different Values, i = " + std::to_string(i));
                        }
                    }
                }
            }
            static void save_and_load_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, UINT32_MAX);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

                uint64_t end_marker = 0;
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars, end_marker);

                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                std::string filepath = "drfmi.bits";

                stool::dynamic_r_index::DynamicRIndex drfmi = stool::dynamic_r_index::DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                {
                    std::ofstream os;
                    os.open(filepath, std::ios::binary);
                    if (!os)
                    {
                        std::cerr << "Error: Could not open file for writing." << std::endl;
                        throw std::runtime_error("File open error");
                    }
                    stool::dynamic_r_index::DynamicRIndex::save(drfmi, os, stool::Message::NO_MESSAGE);
                }

                stool::dynamic_r_index::DynamicRIndex drfmi2;
                {
                    std::ifstream ifs;
                    ifs.open(filepath, std::ios::binary);
                    if (!ifs)
                    {
                        std::cerr << "Error: Could not open file for reading." << std::endl;
                        throw std::runtime_error("File open error");
                    }

                    auto tmp = stool::dynamic_r_index::DynamicRIndex::build_from_data(ifs, stool::Message::NO_MESSAGE);
                    drfmi2.swap(tmp);
                }

                DynamicRIndexTest::verify_r_index(drfmi2);
                BWT_and_SA::bwt_sa_and_isa_check(drfmi, drfmi2);
            }

            static void detailed_string_deletion_test(uint64_t text_size, uint64_t str_len, uint8_t alphabet_type, uint64_t seed)
            {
                if(str_len < 1){
                    throw std::logic_error("The length of the deleted substring is at least 1.");
                }
                if(str_len >= text_size){
                    throw std::logic_error("The length of the deleted substring is less than the text size.");
                }
                
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                // dfmi.initialize(alphabet_with_end_marker);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);

                NaiveDynamicStringForBWT nds;
                nds.initialzie(text);
 


                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);
                // stool::Printer::print("BWT", bwt);
                // stool::Printer::print("SA", sa);

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);

                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                uint64_t size = dfmi.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 2 - str_len);
                // std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                FMIndexEditHistory edit_history, edit_history2;
                // std::vector<uint8_t> inserted_string;
                uint64_t insertion_pos = get_rand_uni_int(mt64);
                assert(insertion_pos < text_size-1);

                /*
                if(insertion_pos + str_len >= text_size){
                    str_len = text_size - insertion_pos - 1;
                }
                */




                dfmi.delete_string(insertion_pos, str_len, &edit_history);
                // edit_history.print();

                
                //std::vector<std::vector<uint64_t>> sa_arrays;
                //nds.construct_SA_arrays_for_deletion(insertion_pos, str_len, sa_arrays);
                


                //stool::dynamic_r_index::SubPhiDataStructure sub(insertion_pos, str_len, false);
                stool::dynamic_r_index::AdditionalInformationUpdatingRIndex inf = drfmi.__preprocess_of_string_deletion_operation(insertion_pos, str_len, edit_history2, nullptr);


                bool b = false;
                // uint64_t x = 0;
                while (!b)
                {
                    //b = drfmi.__reorder_RLBWT(edit_history2, inf);
                    b = drfmi.__reorder_RLBWT_for_insertion(edit_history2, inf);
                }

                DynamicRIndexTest::verify_r_index(drfmi);

                BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);
            }

            static void detailed_string_insertion_test(uint64_t text_size, uint64_t str_len, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);

                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                // drfmi.build(bwt, alphabet_with_end_marker ,stool::Message::NO_MESSAGE);

                uint64_t size = dfmi.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                FMIndexEditHistory edit_history, edit_history2;
                std::vector<uint8_t> inserted_string;
                uint64_t insertion_pos = get_rand_uni_int(mt64);

                for (uint64_t i = 0; i < str_len; i++)
                {
                    uint8_t new_char = chars[get_rand_uni_char(mt64)];
                    inserted_string.push_back(new_char);
                }

                dfmi.insert_string(insertion_pos, inserted_string, &edit_history);
                // assert(drfmi_disa.verify());

                //SubPhiDataStructure sub(insertion_pos, inserted_string.size(), true);
                stool::dynamic_r_index::AdditionalInformationUpdatingRIndex inf = drfmi.__preprocess_of_string_insertion_operation(insertion_pos, inserted_string, edit_history2);
                //sub.verify(drfmi.get_dynamic_phi());
                // assert(drfmi_disa.verify());

                bool b = false;
                // uint64_t x = 0;
                while (!b)
                {
                    b = drfmi.__reorder_RLBWT_for_insertion(edit_history2, inf);
                    //sub.verify(drfmi.get_dynamic_phi());
                }

                DynamicRIndexTest::verify_r_index(drfmi);
                BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);
            }

            static void detailed_insertion_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                // stool::dynamic_r_index::DynamicRIndex drfmi;
                // drfmi.initialize(alphabet_with_end_marker, false);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);
                // stool::Printer::print("BWT", bwt);
                // stool::Printer::print("SA", sa);

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);
                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                // drfmi.build(bwt,alphabet_with_end_marker ,stool::Message::NO_MESSAGE);

                uint64_t size = dfmi.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                uint64_t insertion_pos = get_rand_uni_int(mt64);
                uint8_t new_char = chars[get_rand_uni_char(mt64)];

                std::vector<uint8_t> new_text;
                for (auto c : text)
                {
                    new_text.push_back(c);
                    if ((uint64_t)new_text.size() == insertion_pos)
                    {
                        new_text.push_back(new_char);
                    }
                }
                std::vector<uint64_t> new_sa = libdivsufsort::construct_suffix_array(new_text, stool::Message::NO_MESSAGE);

                // std::cout << "InsChar: " << new_char << " at position " << insertion_pos << std::endl;

                FMIndexEditHistory edit_history1, edit_history2;
                //SubPhiDataStructure sub(insertion_pos, 1, true);

                dfmi.insert_char(insertion_pos, new_char, &edit_history1);
                std::vector<BWT_and_SA> bwt_sa = BWT_and_SA::build_for_insertion(bwt, sa, edit_history1);
                stool::dynamic_r_index::AdditionalInformationUpdatingRIndex inf = drfmi.__preprocess_of_char_insertion_operation(insertion_pos, new_char, edit_history2);

                stool::equal_check("BWT(0)", bwt_sa[0].bwt, drfmi.get_bwt());
                //stool::equal_check("SA(0)", bwt_sa[0].sa, drfmi.get_sa());

                bool b = false;

                uint64_t x = 0;
                while (!b)
                {
                    b = drfmi.__reorder_RLBWT_for_insertion(edit_history2, inf);
                    if (!b)
                    {
                        assert(x < edit_history1.move_history.size());
                        // std::cout << "Move: " << edit_history1.move_history[x].first << " -> " << edit_history1.move_history[x].second << std::endl;

                        x++;
                        stool::equal_check("BWT(" + std::to_string(x) + ")", bwt_sa[x].bwt, drfmi.get_bwt());
                        //const stool::dynamic_r_index::DynamicPhi &disa = drfmi.get_dynamic_phi();
                        //stool::equal_check("SA(" + std::to_string(x) + ")", bwt_sa[x].sa, sub.get_sa(disa));

                        // check_sampling_sa("SA(" + std::to_string(x) + ")", bwt_sa[x].sa, drfmi.get_sampling_sa());
                    }
                }

                DynamicRIndexTest::verify_r_index(drfmi);
                BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);
            }
            /*
            static void detailed_deletion_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                // stool::dynamic_r_index::DynamicRIndex drfmi;
                // drfmi.initialize(alphabet_with_end_marker, false);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                uint64_t size = text.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 2);
                uint64_t removal_pos = get_rand_uni_int(mt64);

                std::vector<uint8_t> new_text;
                for (uint64_t i = 0; i < text.size(); i++)
                {
                    if (i != removal_pos)
                    {
                        new_text.push_back(text[i]);
                    }
                }
                std::vector<uint64_t> new_sa = libdivsufsort::construct_suffix_array(new_text, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> new_bwt = stool::construct_BWT(new_text, new_sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);
                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);
                
                DynamicRIndex drfmi2 = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                // drfmi.build(bwt, alphabet_with_end_marker,stool::Message::NO_MESSAGE);

                FMIndexEditHistory edit_history1, edit_history2;
                SubPhiDataStructure sub(removal_pos, 1, false);




                dfmi.delete_char(removal_pos, &edit_history1);
                std::vector<BWT_and_SA> bwt_sa = BWT_and_SA::build_for_deletion(bwt, sa, edit_history1);

                stool::dynamic_r_index::AdditionalInformationUpdatingRIndex inf = drfmi.__preprocess_of_char_deletion_operation(removal_pos, edit_history2, sub);
                stool::dynamic_r_index::AdditionalInformationUpdatingRIndex inf2 = drfmi2.__preprocess_of_char_deletion_operation2(removal_pos, edit_history2, sub);

                {
                    assert(inf.y == inf2.y);
                    assert(inf.z == inf2.z);

                    auto bwt_1 = drfmi.get_bwt();
                    auto bwt_2 = drfmi2.get_bwt();
                    stool::equal_check("BWT(0)", bwt_1, bwt_2);

                    //assert(inf.value_at_y == inf2.value_at_y);
                    //assert(inf.value_at_z == inf2.value_at_z);
                }


                stool::equal_check("BWT(0)", bwt_sa[0].bwt, drfmi.get_bwt());
                stool::equal_check("SA(0)", bwt_sa[0].sa, sub.get_sa(drfmi.get_dynamic_phi()));

                bool b = false;
                uint64_t x = 0;
                while (!b)
                {
                    b = drfmi.__reorder_RLBWT_for_deletion(edit_history2, sub, inf);
                    if (!b)
                    {
                        assert(x < edit_history1.move_history.size());
                        x++;
                        stool::equal_check("BWT(" + std::to_string(x) + ")", bwt_sa[x].bwt, drfmi.get_bwt());

                        const stool::dynamic_r_index::DynamicPhi &disa = drfmi.get_dynamic_phi();
                        stool::equal_check("SA(" + std::to_string(x) + ")", bwt_sa[x].sa, sub.get_sa(disa));
                    }
                }

                DynamicRIndexTest::verify_r_index(drfmi);
                BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);
            }
            */
            static void detailed_deletion_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                // stool::dynamic_r_index::DynamicRIndex drfmi;
                // drfmi.initialize(alphabet_with_end_marker, false);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                uint64_t size = text.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 2);
                uint64_t removal_pos = get_rand_uni_int(mt64);

                std::vector<uint8_t> new_text;
                for (uint64_t i = 0; i < text.size(); i++)
                {
                    if (i != removal_pos)
                    {
                        new_text.push_back(text[i]);
                    }
                }
                std::vector<uint64_t> new_sa = libdivsufsort::construct_suffix_array(new_text, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> new_bwt = stool::construct_BWT(new_text, new_sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);
                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                // drfmi.build(bwt, alphabet_with_end_marker,stool::Message::NO_MESSAGE);

                FMIndexEditHistory edit_history1, edit_history2;
                //SubPhiDataStructure sub(removal_pos, 1, false);




                dfmi.delete_char(removal_pos, &edit_history1);
                std::vector<BWT_and_SA> bwt_sa = BWT_and_SA::build_for_deletion(bwt, sa, edit_history1);

                stool::dynamic_r_index::AdditionalInformationUpdatingRIndex inf = drfmi.__preprocess_of_char_deletion_operation(removal_pos, edit_history2);


                stool::equal_check("BWT(0)", bwt_sa[0].bwt, drfmi.get_bwt());
                //stool::equal_check("SA(0)", bwt_sa[0].sa, sub.get_sa(drfmi.get_dynamic_phi()));

                bool b = false;
                uint64_t x = 0;
                while (!b)
                {
                    b = drfmi.__reorder_RLBWT_for_insertion(edit_history2, inf);
                    if (!b)
                    {
                        assert(x < edit_history1.move_history.size());
                        x++;
                        stool::equal_check("BWT(" + std::to_string(x) + ")", bwt_sa[x].bwt, drfmi.get_bwt());

                        /*
                        const stool::dynamic_r_index::DynamicPhi &disa = drfmi.get_dynamic_phi();
                        stool::equal_check("SA(" + std::to_string(x) + ")", bwt_sa[x].sa, sub.get_sa(disa));
                        */
                    }
                }

                DynamicRIndexTest::verify_r_index(drfmi);
                BWT_and_SA::bwt_sa_and_isa_check(dfmi, drfmi);
            }
            /*
            static std::string generate_worst_string(uint64_t text_size)
            {
                uint64_t left = 2;
                uint64_t right = 4;

                for (int64_t i = 8; i < (int64_t)text_size; i++)
                {
                    if (i % 3 == 0)
                    {
                        left++;
                    }
                    else
                    {
                        right++;
                    }
                }

                std::string s;
                for (int64_t i = 0; i < (int64_t)left; i++)
                {
                    s.push_back('b');
                }
                s.push_back('a');

                for (int64_t i = 0; i < (int64_t)right; i++)
                {
                    s.push_back('b');
                }
                return s;
            }
            */
            static void backward_search_test(uint64_t text_size, uint64_t pattern_count, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, UINT32_MAX);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = DynamicFMIndexTest::create_alphabet_with_end_marker(chars);

                // stool::dynamic_r_index::DynamicRIndex drfmi;
                // drfmi.initialize(alphabet_with_end_marker, false);

                std::vector<uint8_t> text = DynamicRIndexTest::create_text(text_size, chars, alphabet_with_end_marker[0], mt64);
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, isa, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);
                DynamicRIndex drfmi = DynamicRIndex::build_from_BWT(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                // drfmi.build(bwt, alphabet_with_end_marker ,stool::Message::NO_MESSAGE);

                // stool::Printer::print_sa_table(text, sa);

                for (uint64_t i = 0; i < pattern_count; i++)
                {
                    std::vector<uint8_t> pattern = stool::UInt8VectorGenerator::create_random_substring(text, get_rand_uni_int(mt64));
                    // stool::Printer::print_string(pattern);
                    std::vector<uint64_t> result1 = dfmi.locate_query(pattern);
                    std::vector<uint64_t> result2 = drfmi.locate_query(pattern);

                    stool::equal_check(result1, result2);
                }
            }
        };

    }
}

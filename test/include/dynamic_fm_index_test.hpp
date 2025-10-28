#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "../../include/dynamic_r_index.hpp"
#include "stool/include/stool.hpp"
#include "./naive_dynamic_string_x.hpp"
namespace stool
{
    namespace fm_index_test
    {
        class DynamicFMIndexTest
        {
        public:
            static void BWT_equal_check(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                std::vector<uint8_t> correctBWT = nds.create_bwt();
                std::vector<uint8_t> testBWT = dfmi.get_bwt();
                try
                {
                    stool::EqualChecker::equal_check(correctBWT, testBWT, "BWT");
                }
                catch (std::logic_error &e)
                {
                    stool::DebugPrinter::print_characters(correctBWT, "Correct");
                    stool::DebugPrinter::print_characters(testBWT, "Test");

                    throw e;
                }
            }

            static void SA_equal_check(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                std::vector<uint64_t> correctSA = nds.create_suffix_array();
                std::vector<uint64_t> testSA = dfmi.get_sa();
                stool::EqualChecker::equal_check(correctSA, testSA, "SA");
            }
            static void ISA_equal_check(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                std::vector<uint64_t> correctISA = nds.create_inverse_suffix_array();
                std::vector<uint64_t> testISA = dfmi.get_isa();
                stool::EqualChecker::equal_check(correctISA, testISA, "ISA");
            }
            static void equal_check(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                BWT_equal_check(nds, dfmi);
                SA_equal_check(nds, dfmi);
                ISA_equal_check(nds, dfmi);
            }
            static std::vector<uint8_t> create_alphabet_with_end_marker(const std::vector<uint8_t> &alphabet, uint64_t end_marker = 0)
            {
                std::vector<uint8_t> alphabet_with_end_marker;
                alphabet_with_end_marker.push_back(end_marker);
                int64_t _c_min_ = INT64_MAX;
                for (auto c : alphabet)
                {
                    alphabet_with_end_marker.push_back(c);
                    _c_min_ = std::min(_c_min_, (int64_t)c);
                }
                if (_c_min_ > (int64_t)end_marker)
                {
                    alphabet_with_end_marker[0] = end_marker;
                }
                else if (_c_min_ <= (int64_t)end_marker)
                {
                    throw std::runtime_error("Error: create_alphabet_with_end_marker");
                }

                return alphabet_with_end_marker;
            }

            static void bwt_test_random_insertion(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, std::vector<uint8_t> &chars, std::mt19937_64 &mt64)
            {
                uint64_t size = nds.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);
                uint8_t new_char = chars[get_rand_uni_char(mt64)];
                nds.insert_char(insertion_pos, new_char);
                dfmi.insert_char(insertion_pos, new_char);
            }

            static void bwt_test_random_string_insertion(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, uint64_t pattern_length, std::vector<uint8_t> &chars, std::mt19937_64 &mt64)
            {
                uint64_t size = nds.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);
                std::vector<uint8_t> pattern;
                for (uint64_t i = 0; i < pattern_length; i++)
                {
                    pattern.push_back(chars[get_rand_uni_char(mt64)]);
                }
                nds.insert_string(insertion_pos, pattern);
                dfmi.insert_string(insertion_pos, pattern);

                BWT_equal_check(nds, dfmi);
                SA_equal_check(nds, dfmi);
                ISA_equal_check(nds, dfmi);
            }

            static void bwt_test_random_deletion(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, std::mt19937_64 &mt64)
            {
                uint64_t size = nds.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 2);

                int64_t deletion_pos = get_rand_uni_int(mt64);
                nds.delete_char(deletion_pos);
                dfmi.delete_char(deletion_pos);
            }

            static void LF_test_random_string_deletion(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicBWT &dbwt, uint64_t delete_pos, uint64_t delete_len)
            {
                if (delete_pos == 0)
                {
                    return;
                }

                /*
                std::cout << "X/" << delete_pos << "/" << delete_len << "/" << nds.size() << std::endl;
                        std::cout << "BWT: " << dbwt.get_bwt_str('$') << std::endl;
                        */

                // std::vector<uint64_t> sa = nds.create_suffix_array();
                std::vector<std::vector<uint64_t>> sa_arrays;
                std::vector<std::vector<uint64_t>> lf_arrays;
                std::vector<uint64_t> idx_array;

                nds.construct_SA_and_LF_array_for_deletion(delete_pos, delete_len, sa_arrays, lf_arrays, idx_array);

                /*

                sa_arrays.push_back(sa);
                std::vector<uint64_t> idx_array;

                idx_array.push_back(nds.create_inverse_suffix_array()[delete_pos + delete_len - 1]);

                // stool::DebugPrinter::print_integers(sa, "sa");

                for (uint64_t i = 0; i < delete_len; i++)
                {
                    std::vector<uint64_t> tmp_sa;
                    for (uint64_t j = 0; j < sa.size(); j++)
                    {
                        uint64_t lower = delete_pos + delete_len - i - 1;
                        uint64_t upper = delete_pos + delete_len - 1;
                        int64_t b_pointer = lower > 0 ? lower - 1 : nds.size() - 1;
                        if (sa[j] < lower || sa[j] > upper)
                        {
                            tmp_sa.push_back(sa[j]);
                        }
                        if (sa[j] == b_pointer)
                        {
                            idx_array.push_back(tmp_sa.size() - 1);
                        }
                    }
                    auto tmp_lf = NaiveDynamicStringForBWT::construct_dynamic_LF_array(sa_arrays[sa_arrays.size() - 1], tmp_sa);
                    sa_arrays.push_back(tmp_sa);
                    lf_arrays.push_back(tmp_lf);
                }
                */

                uint64_t new_char_pointer = delete_pos > 0 ? delete_pos - 1 : nds.size() - 1;
                uint64_t new_char = nds.text[new_char_pointer];

                uint64_t pointer = delete_pos + delete_len < (uint64_t)dbwt.size() ? delete_pos + delete_len : 0;
                uint64_t p_on_sa = nds.create_inverse_suffix_array()[pointer];
                uint64_t p_on_next = dbwt.LF(p_on_sa);
                //uint8_t old_char = dbwt.access(p_on_sa);

                uint64_t replace_pos = p_on_sa;
                // std::cout << "REPLACE: " << replace_pos << "/" << (char)old_char<< " ->" << (char)new_char << std::endl;

                dbwt.replace_BWT_character(p_on_sa, new_char);
                p_on_sa = p_on_next;
                if (p_on_sa != idx_array[0])
                {
                    throw std::runtime_error("Error: LF_test_random_string_deletion");
                }

                for (uint64_t i = 0; i < delete_len; i++)
                {
                    /*
                    std::cout << std::endl;
                    stool::DebugPrinter::print_integers(sa_arrays[i], "sa" + std::to_string(i));
                    stool::DebugPrinter::print_integers(sa_arrays[i+1], "sa" + std::to_string(i+1));

                    std::cout << "REMOVE: " << (delete_pos + delete_len - i - 1) << std::endl;
                    std::cout << "BWT: " << dbwt.get_bwt_str('$') << std::endl;
                    std::cout << "p_on_sa: " << p_on_sa << "/" << "old_char = " << (char)old_char << "/ c = " << (char)dbwt.access(p_on_sa) << ", rep_pos = " << replace_pos << std::endl;


                   stool::DebugPrinter::print_integers(lf_arrays[i], "lf" + std::to_string(i));
                   */

                    for (uint64_t j = 0; j < lf_arrays[i].size(); j++)
                    {
                        uint64_t test_p = dbwt.LF_for_deletion(j, new_char, replace_pos, p_on_sa);

                        // std::cout << "test_p: " << test_p << "/" << lf_arrays[i][j] << std::endl;
                        if (test_p != lf_arrays[i][j])
                        {
                            throw std::runtime_error("Error: LF_test_random_string_deletion");
                        }
                    }

                    //uint8_t current_char = dbwt.access(p_on_sa);
                    uint64_t p_on_next = dbwt.LF_for_deletion(p_on_sa, new_char, replace_pos, p_on_sa);

                    dbwt.remove_BWT_character(p_on_sa);
                    if (replace_pos > p_on_sa)
                    {
                        replace_pos--;
                    }

                    // std::cout << "CHECK: " << p_on_sa << " ->" << p_on_next << "/" << idx_array[1 + i] << std::endl;

                    if (p_on_next != idx_array[1 + i])
                    {
                        throw std::runtime_error("Error: LF_test_random_string_deletion" + std::to_string(i));
                    }
                    p_on_sa = p_on_next;
                }
                // std::cout << std::endl;
            }

            static void bwt_test_random_string_deletion(stool::NaiveDynamicStringX &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, uint64_t delete_len, std::mt19937_64 &mt64, [[maybe_unused]] uint64_t counter)
            {
                uint64_t size = nds.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - delete_len - 2);

                int64_t deletion_pos = get_rand_uni_int(mt64);

                nds.delete_string(deletion_pos, delete_len);

                dfmi.delete_string(deletion_pos, delete_len);

                BWT_equal_check(nds, dfmi);

                SA_equal_check(nds, dfmi);

                ISA_equal_check(nds, dfmi);
            }

            static void sampled_isa_test(uint64_t text_size, uint8_t alphabet_type, [[maybe_unused]] bool detailed_check, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                std::vector<uint8_t> text = stool::RandomString::create_random_sequence(text_size, chars, seed);
                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicBWT dbwt = stool::dynamic_r_index::DynamicBWT::build(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                stool::bwt::LFDataStructure lfds = stool::bwt::LFDataStructure::build(bwt);
                stool::bwt::BackwardISA<stool::bwt::LFDataStructure> bisa;
                bisa.set(&lfds, lfds.get_end_marker_position(), lfds.get_text_size());

                // auto tmp = bisa.to_isa();
                // stool::EqualChecker::equal_check("ISA", isa, tmp);

                stool::dynamic_r_index::DynamicSampledSA dsisa = stool::dynamic_r_index::DynamicSampledSA::build(bisa, &dbwt, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);
                // stool::dynamic_r_index::DynamicSampledSA dsisa2 = stool::dynamic_r_index::DynamicSampledSA::build(isa, &dbwt);

                // dsisa.print_info();
                // dsisa2.print_info();

                assert(dsisa.size() == sa.size());

                std::vector<uint64_t> test_sa = dsisa.get_sa();
                std::vector<uint64_t> test_isa = dsisa.get_isa();

                /*
                std::vector<uint64_t> test_sa2 = dsisa2.get_sa();
                std::vector<uint64_t> test_isa2 = dsisa2.get_isa();
                */
                stool::EqualChecker::equal_check(sa, test_sa, "SA");
                stool::EqualChecker::equal_check(isa, test_isa, "ISA");

                // stool::EqualChecker::equal_check("SA", sa, test_sa2);
                // stool::EqualChecker::equal_check("ISA", isa, test_isa2);
            }

            static void character_insertion_and_deletion_test(uint64_t item_num, uint8_t alphabet_type, bool detailed_check, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);

                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                stool::NaiveDynamicStringX nds;
                nds.initialzie(end_marker);

                stool::dynamic_r_index::DynamicFMIndex dfmi;
                dfmi.initialize(alphabet_with_end_marker);

                for (uint64_t i = 0; i < item_num; i++)
                {

                    DynamicFMIndexTest::bwt_test_random_insertion(nds, dfmi, chars, mt64);

                    if (detailed_check || (item_num % 100 == 0))
                    {
                        DynamicFMIndexTest::equal_check(nds, dfmi);
                    }
                }
                DynamicFMIndexTest::equal_check(nds, dfmi);

                while (nds.size() > 1)
                {

                    DynamicFMIndexTest::bwt_test_random_deletion(nds, dfmi, mt64);
                    if (detailed_check || (item_num % 100 == 0))
                    {
                        DynamicFMIndexTest::equal_check(nds, dfmi);
                    }
                }
                DynamicFMIndexTest::equal_check(nds, dfmi);
            }

            static void string_deletion_for_LF_test(uint64_t item_num, uint64_t pattern_length, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                std::vector<uint8_t> text = stool::RandomString::create_random_sequence(item_num, chars, seed);

                stool::NaiveDynamicStringX nds;
                nds.initialzie(end_marker);
                nds.insert_string(0, text);

                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicBWT dbwt = stool::dynamic_r_index::DynamicBWT::build(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                uint64_t size = nds.size();
                assert(size >= 10);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - pattern_length - 2);

                int64_t deletion_pos = get_rand_uni_int(mt64);
                LF_test_random_string_deletion(nds, dbwt, deletion_pos, pattern_length);
            }

            static void string_insertion_and_deletion_test(uint64_t item_num, uint64_t pattern_length, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                stool::NaiveDynamicStringX nds;
                nds.initialzie(end_marker);

                stool::dynamic_r_index::DynamicFMIndex dfmi;
                dfmi.initialize(alphabet_with_end_marker);

                for (uint8_t c : chars)
                {
                    nds.insert_char(0, c);
                    dfmi.insert_char(0, c);
                }

                while (nds.size() < (int64_t)item_num)
                {
                    DynamicFMIndexTest::bwt_test_random_string_insertion(nds, dfmi, pattern_length, chars, mt64);
                }

                uint64_t counter = 0;

                while (nds.size() > (int64_t)pattern_length + 1)
                {
                    DynamicFMIndexTest::bwt_test_random_string_deletion(nds, dfmi, pattern_length, mt64, counter++);
                }
            }

            static void save_and_load_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, UINT32_MAX);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::RandomString::create_random_sequence(text_size, chars, seed);

                uint64_t end_marker = 0;
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars, end_marker);

                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::ArrayConstructor::construct_ISA(sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::ArrayConstructor::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                std::string filepath = "dfmi.bits";

                stool::dynamic_r_index::DynamicFMIndex dfmi = stool::dynamic_r_index::DynamicFMIndex::build(bwt, alphabet_with_end_marker, stool::dynamic_r_index::DynamicSampledSA::DEFAULT_SAMPLING_INTERVAL, stool::Message::NO_MESSAGE);
                auto bwt1 = dfmi.get_bwt();
                auto sa1 = dfmi.get_sa();
                auto isa1 = dfmi.get_isa();

                {
                    std::ofstream os;
                    os.open(filepath, std::ios::binary);
                    if (!os)
                    {
                        std::cerr << "Error: Could not open file for writing." << std::endl;
                        throw std::runtime_error("File open error");
                    }
                    stool::dynamic_r_index::DynamicFMIndex::store_to_file(dfmi, os, stool::Message::NO_MESSAGE);
                }

                stool::dynamic_r_index::DynamicFMIndex dfmi2;
                {
                    std::ifstream ifs;
                    ifs.open(filepath, std::ios::binary);
                    if (!ifs)
                    {
                        std::cerr << "Error: Could not open file for reading." << std::endl;
                        throw std::runtime_error("File open error");
                    }

                    auto tmp = stool::dynamic_r_index::DynamicFMIndex::load_from_file(ifs, stool::Message::NO_MESSAGE);
                    dfmi2.swap(tmp);
                }

                auto bwt2 = dfmi2.get_bwt();

                auto sa2 = dfmi2.get_sa();

                auto isa2 = dfmi2.get_isa();

                stool::EqualChecker::equal_check(bwt1, bwt2, "BWT");
                stool::EqualChecker::equal_check(sa1, sa2, "SA");
                stool::EqualChecker::equal_check(isa1, isa2, "ISA");
            }

            static void backward_search_test(uint64_t text_size, uint64_t pattern_count, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, UINT32_MAX);
                std::vector<uint8_t> chars = stool::Alphabet::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::RandomString::create_random_sequence(text_size, chars, seed);

                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                // uint8_t end_marker = alphabet_with_end_marker[0];

                stool::dynamic_r_index::DynamicFMIndex dfmi;
                dfmi.initialize(alphabet_with_end_marker);
                dfmi.insert_string(0, text);

                text.push_back('$');
                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);

                // stool::Printer::print_sa_table(text, sa);

                for (uint64_t i = 0; i < pattern_count; i++)
                {
                    std::vector<uint8_t> pattern = stool::RandomString::create_random_substring(text, get_rand_uni_int(mt64));
                    // stool::Printer::print_string(pattern);
                    std::vector<uint64_t> result1 = stool::StringFunctionsOnSA::locate_query(text, pattern, sa);
                    std::vector<uint64_t> result2 = dfmi.locate_query(pattern);
                    std::sort(result2.begin(), result2.end());

                    stool::EqualChecker::equal_check(result1, result2);
                }
            }
        };

    }
}

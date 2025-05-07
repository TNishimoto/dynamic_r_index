#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "./naive_dynamic_string.hpp"
#include "../../include/dynamic_r_index.hpp"
#include "stool/include/stool.hpp"
namespace stool
{
    namespace fm_index_test
    {
        class DynamicFMIndexTest
        {
        public:
            static void BWT_equal_check(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                std::vector<uint8_t> correctBWT = nds.create_bwt();
                std::vector<uint8_t> testBWT = dfmi.get_bwt();
                try
                {
                    stool::equal_check("BWT", correctBWT, testBWT);
                }
                catch (std::logic_error &e)
                {
                    stool::DebugPrinter::print_integers(correctBWT, "Correct");
                    stool::DebugPrinter::print_integers(testBWT, "Test");

                    throw e;
                }
            }

            static void SA_equal_check(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                std::vector<uint64_t> correctSA = nds.create_suffix_array();
                std::vector<uint64_t> testSA = dfmi.get_sa();
                stool::equal_check("SA", correctSA, testSA);
            }
            static void ISA_equal_check(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
            {
                std::vector<uint64_t> correctISA = nds.create_inverse_suffix_array();
                std::vector<uint64_t> testISA = dfmi.get_isa();
                stool::equal_check("ISA", correctISA, testISA);
            }
            static void equal_check(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi)
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

            static void bwt_test_random_insertion(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, std::vector<uint8_t> &chars, std::mt19937_64 &mt64)
            {
                uint64_t size = nds.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
                std::uniform_int_distribution<uint64_t> get_rand_uni_char(0, chars.size() - 1);

                int64_t insertion_pos = get_rand_uni_int(mt64);
                uint8_t new_char = chars[get_rand_uni_char(mt64)];
                nds.insert_char(insertion_pos, new_char);
                dfmi.insert_char(insertion_pos, new_char);
            }

            static void bwt_test_random_string_insertion(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, uint64_t pattern_length, std::vector<uint8_t> &chars, std::mt19937_64 &mt64)
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

            static void bwt_test_random_deletion(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, std::mt19937_64 &mt64)
            {
                uint64_t size = nds.size();
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 2);

                int64_t deletion_pos = get_rand_uni_int(mt64);
                nds.delete_char(deletion_pos);
                dfmi.delete_char(deletion_pos);
            }
            static void bwt_test_random_string_deletion(stool::dynamic_r_index_test::NaiveDynamicStringForBWT &nds, stool::dynamic_r_index::DynamicFMIndex &dfmi, uint64_t delete_len, std::mt19937_64 &mt64)
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
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);
                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

                stool::dynamic_r_index::DynamicBWT dbwt = stool::dynamic_r_index::DynamicBWT::build(bwt, alphabet_with_end_marker, stool::Message::NO_MESSAGE);

                stool::bwt::LFDataStructure lfds = stool::bwt::LFDataStructure::build(bwt);
                stool::bwt::BackwardISA<stool::bwt::LFDataStructure> bisa;
                bisa.set(&lfds, lfds.get_end_marker_position(), lfds.get_text_size());

                // auto tmp = bisa.to_isa();
                // stool::equal_check("ISA", isa, tmp);

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
                stool::equal_check("SA", sa, test_sa);
                stool::equal_check("ISA", isa, test_isa);

                // stool::equal_check("SA", sa, test_sa2);
                // stool::equal_check("ISA", isa, test_isa2);
            }

            static void character_insertion_and_deletion_test(uint64_t item_num, uint8_t alphabet_type, bool detailed_check, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);

                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                stool::dynamic_r_index_test::NaiveDynamicStringForBWT nds;
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

            static void string_insertion_and_deletion_test(uint64_t item_num, uint64_t pattern_length, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars);
                uint8_t end_marker = alphabet_with_end_marker[0];

                stool::dynamic_r_index_test::NaiveDynamicStringForBWT nds;
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

                while (nds.size() > (int64_t)pattern_length + 1)
                {
                    DynamicFMIndexTest::bwt_test_random_string_deletion(nds, dfmi, pattern_length, mt64);
                }
            }

            static void save_and_load_test(uint64_t text_size, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, UINT32_MAX);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

                uint64_t end_marker = 0;
                std::vector<uint8_t> alphabet_with_end_marker = create_alphabet_with_end_marker(chars, end_marker);

                text.push_back(end_marker);

                std::vector<uint64_t> sa = libdivsufsort::construct_suffix_array(text, stool::Message::NO_MESSAGE);
                std::vector<uint64_t> isa = stool::construct_ISA(text, sa, stool::Message::NO_MESSAGE);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa, stool::Message::NO_MESSAGE);

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
                    stool::dynamic_r_index::DynamicFMIndex::save(dfmi, os, stool::Message::NO_MESSAGE);
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

                    auto tmp = stool::dynamic_r_index::DynamicFMIndex::build_from_data(ifs, stool::Message::NO_MESSAGE);
                    dfmi2.swap(tmp);
                }

                auto bwt2 = dfmi2.get_bwt();

                auto sa2 = dfmi2.get_sa();

                auto isa2 = dfmi2.get_isa();


                stool::equal_check("BWT", bwt1, bwt2);
                stool::equal_check("SA", sa1, sa2);
                stool::equal_check("ISA", isa1, isa2);

            }

            static void backward_search_test(uint64_t text_size, uint64_t pattern_count, uint8_t alphabet_type, uint64_t seed)
            {
                std::mt19937_64 mt64(seed);
                std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, UINT32_MAX);
                std::vector<uint8_t> chars = stool::UInt8VectorGenerator::create_alphabet(alphabet_type);
                std::vector<uint8_t> text = stool::UInt8VectorGenerator::create_random_sequence(text_size, chars, seed);

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
                    std::vector<uint8_t> pattern = stool::UInt8VectorGenerator::create_random_substring(text, get_rand_uni_int(mt64));
                    // stool::Printer::print_string(pattern);
                    std::vector<uint64_t> result1 = stool::StringFunctionsOnSA::locate_query(text, pattern, sa);
                    std::vector<uint64_t> result2 = dfmi.locate_query(pattern);
                    std::sort(result2.begin(), result2.end());

                    stool::equal_check(result1, result2);
                }
            }
        };

    }
}

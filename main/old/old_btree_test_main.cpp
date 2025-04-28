#include <iostream>
#include <string>
#include <memory>
#include <bitset>
#include <cassert>
#include <chrono>
// #include "qgram_tree.h"
// #include "include/debug/debug.hpp"
#include "stool/include/stool.hpp"
#include "b_tree_plus_alpha/include/b_tree_plus_alpha.hpp"
#include "../include/old_implementations/btree_include.hpp"
#include "../test/dynamic_bwt_test.hpp"
#include "../test/dynamic_r_index_test.hpp"
//

#include <filesystem>

void insert_test(uint64_t item_num, uint64_t degree, uint64_t max_key, uint64_t seed)
{
    stool::old_implementations::BTree<int, int> tree;
    tree.initialize(degree);

    std::mt19937_64 mt64(0);
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, max_key);

    std::map<int, int> mapper;

    for (uint64_t i = 0; i < item_num; i++)
    {
        std::pair<int, int> p = std::pair<int, int>(get_rand_uni_int(mt64), seed + i);
        tree.insert(p.first, p.second);
        assert(tree.check_invariant());
        mapper[p.first] = p.second;
        // tree.print();
    }
    std::vector<std::pair<int, int>> correct;
    for (auto it : mapper)
    {
        correct.push_back(std::pair<int, int>(it.first, it.second));
    }

    auto r = tree.get_key_value_pairs();
    /*
    for (auto &it : r)
    {
        std::cout << "(" << it.first << ", " << it.second << "), " << std::flush;
    }
    std::cout << std::endl;
    */

    check_output(r, correct);
}

void rank_test_for_vpom(uint64_t item_num, uint64_t degree, uint64_t seed)
{
    std::vector<int64_t> npom;
    stool::old_implementations::VectorizedPomTree pom;
    pom.initialize(degree, nullptr, false);

    std::mt19937_64 mt64(seed);

    for (uint64_t i = 0; i < item_num; i++)
    {
        rank_test_random_insert_for_vpom(npom, pom, mt64);
    }
}

void delete_test(uint64_t item_num, uint64_t degree, uint64_t max_key, uint64_t seed)
{
    stool::old_implementations::BTree<int, int> tree;
    tree.initialize(degree);

    std::mt19937_64 mt64(seed);
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, max_key);

    std::vector<std::pair<int, int>> correct;

    for (uint64_t i = 0; i < item_num; i++)
    {
        std::pair<int, int> p = std::pair<int, int>(get_rand_uni_int(mt64), i);

        tree.insert(p.first, p.second);
        assert(tree.check_invariant());
        correct.push_back(p);
    }

    for (auto &it : correct)
    {

        tree.erase(it.first);
        // tree.print();
        assert(tree.check_invariant());
    }
}

void permutation_test_random_insert(stool::old_implementations::NaivePOM &npom, stool::old_implementations::PermutationOrderMaintenance &pom, std::mt19937_64 &mt64)
{
    uint64_t size = npom.pi_list.size();
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size);
    uint64_t pi_index = get_rand_uni_int(mt64);
    uint64_t rev_pi_index = get_rand_uni_int(mt64);

    npom.insert(pi_index, rev_pi_index);
    pom.insert(pi_index, rev_pi_index);

    std::vector<uint64_t> pom_pi_vector = pom.get_pi_vector();
    std::vector<uint64_t> pom_inv_pi_vector = pom.get_inverse_pi_vector();

    stool::equal_check(pom_pi_vector, npom.pi_list);
    stool::equal_check(pom_inv_pi_vector, npom.inverse_pi_list);
}

void permutation_test_random_delete(stool::old_implementations::NaivePOM &npom, stool::old_implementations::PermutationOrderMaintenance &pom, std::mt19937_64 &mt64)
{
    uint64_t size = npom.pi_list.size();
    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
    uint64_t pi_index = get_rand_uni_int(mt64);

    npom.erase(pi_index);
    pom.erase(pi_index);

    std::vector<uint64_t> pom_pi_vector = pom.get_pi_vector();
    std::vector<uint64_t> pom_inv_pi_vector = pom.get_inverse_pi_vector();

    stool::equal_check(pom_pi_vector, npom.pi_list);
    stool::equal_check(pom_inv_pi_vector, npom.inverse_pi_list);
}

void permutation_test(uint64_t item_num, uint64_t degree, uint64_t seed)
{
    stool::old_implementations::NaivePOM npom;
    stool::old_implementations::PermutationOrderMaintenance pom;
    pom.initialize(degree);

    std::mt19937_64 mt64(seed);

    for (uint64_t i = 0; i < item_num; i++)
    {
        permutation_test_random_insert(npom, pom, mt64);
    }

    while (npom.pi_list.size() > 0)
    {
        permutation_test_random_delete(npom, pom, mt64);
    }

    std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, 2);

    for (uint64_t i = 0; i < item_num * 5; i++)
    {
        bool b = get_rand_uni_int(mt64) <= 1;
        if (b)
        {
            permutation_test_random_insert(npom, pom, mt64);
        }
        else if (!b && npom.size() != 0)
        {
            permutation_test_random_delete(npom, pom, mt64);
        }
    }
}

int main(int argc, char *argv[])
{
#ifdef DEBUG
    std::cout << "\033[41m";
    std::cout << "DEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif
#ifdef SLOWDEBUG
    std::cout << "\033[41m";
    std::cout << "SLOWDEBUG MODE!" << std::endl;
    std::cout << "\e[m" << std::endl;
    // std::cout << "\033[30m" << std::endl;
#endif

    cmdline::parser p;

    // p.add<std::string>("input_file", 'i', "input file name", true);
    p.add<uint>("mode", 'm', "mode", true);

    p.parse_check(argc, argv);
    uint64_t mode = p.get<uint>("mode");

    /*
     std::string str = "ababbbababa";
     std::vector<uint8_t> text = to_vector(str);


     stool::drfmi::DynamicBWT dbwt;
     dbwt.wavelet_tree.push_many(255, str);
     for(uint64_t i = 0;i < dbwt.wavelet_tree.size();i++){
         std::cout << (char)dbwt.wavelet_tree[i];

     }
     std::cout << std::endl;
     */

    // insert_test(80, 4);

    if (mode == 0)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 1; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                insert_test(160, degree, 1000, i);
                delete_test(160, degree, 1000, i);
                permutation_test(160, degree, i);

                // pom_insert_test(160, 4, 1000, i);
                // pom_delete_test(160, 4, 1000, i);
            }
            degree *= 2;
        }
    }
    else if (mode == 1)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;

                stool::fm_index_test::DynamicBWTTest::bwt_test(160, degree, i);
            }
            degree *= 2;
        }
    }
    else if (mode == 2)
    {
        rank_test_for_vpom(64, 4, 0);
    }
    else if (mode == 3)
    {
        stool::old_implementations::rank_test_for_vpom2(218, 4, 0);
    }
    else if (mode == 4)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;

                stool::old_implementations::permutation_test2(320, degree, i);
            }
            degree *= 2;
        }
    }
    else if (mode == 5)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                stool::r_index_test::DynamicRIndexTest::drfmi_dynamic_test(320, degree, i);
            }
            std::cout << std::endl;

            degree *= 2;
        }
    }
    else if (mode == 6)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                stool::r_index_test::DynamicRIndexTest::inverse_phi_test(320, degree, i);
            }
            std::cout << std::endl;

            degree *= 2;
        }
    }
    else if (mode == 7)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                stool::r_index_test::DynamicRIndexTest::F_test(320, degree, i);
            }
            std::cout << std::endl;

            degree *= 2;
        }
    }
    else if (mode == 8)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                //stool::r_index::DynamicRIndexTest::detailed_insertion_test(320, degree, i);
            }
            std::cout << std::endl;

            degree *= 2;
        }
    }
    else if (mode == 9)
    {
        uint64_t degree = 4;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                //stool::r_index::DynamicRIndexTest::detailed_deletion_test(320, degree, i);
            }
            std::cout << std::endl;

            degree *= 2;
        }
    }
    else if (mode == 10)
    {
        for (int64_t i = 4; i < 18; i++)
        {
            stool::r_index_test::DynamicRIndexTest::widnow_test(i, false);
        }
    }
    else if (mode == 11)
    {
        for (int64_t i = 49; i < 200; i++)
        {
            stool::r_index_test::DynamicRIndexTest::widnow_test3(i);
        }
    }
    else if (mode == 12)
    {
        std::string s = "bbbbbbbbbbbbbbbbabbbbbbbbbbbbbbbbb$";
        // std::string s = "bbbbbbbbbbbbbbbba$";
        std::vector<uint8_t> text;
        for (auto c : s)
        {
            text.push_back(c);
        }

        std::vector<uint8_t> chars;
        chars.push_back('a');
        chars.push_back('b');
        stool::r_index_test::DynamicRIndexTest::widnow_test2(text, chars);
    }
    else if (mode == 13)
    {
        dyn::packed_spsi spsi;
        for (int64_t i = 0; i < 100; i++)
        {
            spsi.push_back(1);
        }
        stool::PackedSPSIWrapper::print(spsi);

        while (spsi.size() > 1)
        {
            uint64_t x = spsi.at(spsi.size() - 1);
            spsi.increment(spsi.size() - 2, x);
            spsi.remove(spsi.size() - 1);
        }

        stool::PackedSPSIWrapper::print(spsi);
    }
    else if (mode == 14)
    {
        uint64_t length_of_insertion_string = 2;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                //stool::r_index::DynamicRIndexTest::detailed_string_insertion_test(320, 4, length_of_insertion_string, i);
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if (mode == 15)
    {
        uint64_t length_of_insertion_string = 2;
        for (uint64_t x = 0; x < 3; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                stool::fm_index_test::DynamicBWTTest::bwt_test2(320, 4, length_of_insertion_string, i);
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if (mode == 16)
    {
        uint64_t length_of_insertion_string = 1;
        for (uint64_t x = 0; x < 4; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                //stool::r_index::DynamicRIndexTest::detailed_string_deletion_test(320, 4, length_of_insertion_string, i);
            }
            std::cout << std::endl;

            length_of_insertion_string *= 2;
        }
    }
    else if (mode == 17)
    {
        uint64_t length = 8;
        std::vector<uint8_t> chars;
        chars.push_back('A');
        chars.push_back('C');
        chars.push_back('G');
        chars.push_back('T');

        for (uint64_t x = 0; x < 12; x++)
        {
            for (uint64_t i = 0; i < 100; i++)
            {
                std::cout << "+" << std::flush;
                std::vector<uint8_t> text = stool::StringGenerator::create_random_uint8_t_sequence(length, chars, i);
                text.push_back('$');

                std::vector<uint64_t> sa = stool::construct_suffix_array(text);
                std::vector<uint8_t> bwt = stool::construct_BWT(text, sa);
                text.pop_back();
                stool::r_index::DynamicRLBWT rlbwt = stool::r_index::RLBWTBuilder::build(text);

                stool::equal_check("BWT", bwt, rlbwt.get_bwt());
            }
            std::cout << std::endl;

            length *= 2;
        }
    }
    else if (mode == 18)
    {
        std::vector<uint8_t> chars, text, pattern;
        chars.push_back('a');
        chars.push_back('b');
        chars.push_back('$');
        uint64_t len = 259;
        for (uint64_t i = 0; i < len; i++)
        {
            text.push_back('a');
        }
        for (uint64_t i = 0; i < 1; i++)
        {
            pattern.push_back('b');
        }

        stool::r_index::DynamicRLBWT rlb = stool::r_index::RLBWTBuilder::build(text);
        std::vector<uint8_t> bwt = rlb.get_bwt();

        stool::r_index::DynamicRIndex drfmi;
        drfmi.initialize(chars, 4);
        drfmi.build(bwt);

        // drfmi.print_bwt_table();

        drfmi.insert_string(19, pattern);
        // drfmi.insert_char((len/2), 'b');
        drfmi.print_bwt_table();
    }
    else if (mode == 19)
    {

        dyn::packed_spsi spsi;
        //spsi.push_back(256);
            spsi.insert(0, 256);

        for (uint64_t i = 0; i < 21; i++)
        {
            std::cout << "Ins: " << i << std::endl;
            spsi.insert(1, 1);
        }
        while(spsi.size() > 0){
            std::cout << "Rem: " << spsi.size() << std::endl;

            spsi.remove(spsi.size()-1);
        }

        
    }
}
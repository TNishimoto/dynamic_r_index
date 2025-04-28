#pragma once

#include "./vectorized_pom_tree.hpp"
#include "./vpom_permutation.hpp"
#include "stool/include/debug.hpp"

namespace stool
{
    namespace old_implementations
    {
        void rank_test_random_insert_for_vpom(std::vector<int64_t> &npom, VectorizedPomTree<> &vpom, std::mt19937_64 &mt64)
        {
            uint64_t size = npom.size();
            std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size);
            uint64_t pi_index = get_rand_uni_int(mt64);
            uint64_t rev_pi_index = size;

            std::cout << "Ins: " << pi_index << ", id = " << rev_pi_index << std::endl;

            npom.insert(npom.begin() + pi_index, rev_pi_index);
            vpom.insert(pi_index);
            vpom.print_info();
            vpom.print();
            vpom.verify();

            std::vector<int64_t> pom_id_vector = vpom.get_sorted_id_vectors();

            stool::Printer::print("Collect", npom);
            stool::Printer::print("Test   ", pom_id_vector);

            stool::equal_check(pom_id_vector, npom);
        }
        void rank_test_random_delete_for_vpom(std::vector<int64_t> &npom, VectorizedPomTree<> &vpom, std::mt19937_64 &mt64)
        {
            uint64_t size = npom.size();
            std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
            uint64_t pi_index = get_rand_uni_int(mt64);

            std::cout << "Del: " << pi_index << std::endl;

            npom.erase(npom.begin() + pi_index);
            vpom.erase(pi_index);
            vpom.print_info();
            vpom.print();
            vpom.verify();

            std::vector<int64_t> pom_id_vector = vpom.get_sorted_id_vectors();

            stool::Printer::print("Collect", npom);
            stool::Printer::print("Test   ", pom_id_vector);

            stool::equal_check(pom_id_vector, npom);
        }
        void rank_test_for_vpom2(uint64_t item_num, uint64_t degree, uint64_t seed)
        {
            std::vector<int64_t> npom;
            VectorizedPomTree pom;
            pom.initialize(degree, nullptr, true);

            std::mt19937_64 mt64(seed);

            for (uint64_t i = 0; i < item_num; i++)
            {
                rank_test_random_insert_for_vpom(npom, pom, mt64);
            }

            while (npom.size() > 0)
            {
                rank_test_random_delete_for_vpom(npom, pom, mt64);
            }
        }

        void permutation_test_random_insert(NaivePOM &npom, VPomPermutation &pom, std::mt19937_64 &mt64)
        {
            uint64_t size = npom.pi_list.size();
            std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size);
            uint64_t pi_index = get_rand_uni_int(mt64);
            uint64_t rev_pi_index = get_rand_uni_int(mt64);

            // std::cout << "Insert: pi = " << pi_index << ", rev = " << rev_pi_index << std::endl;

            npom.insert(pi_index, rev_pi_index);
            pom.insert(pi_index, rev_pi_index);
            // pom.set_value(pi_index, size);

            // std::cout << "PI:" << std::endl;
            // pom.pi_tree.print_info();
            // pom.pi_tree.print();
            // std::cout << "INV_PI:" << std::endl;
            // pom.inverse_pi_tree.print_info();
            // pom.inverse_pi_tree.print();

            pom.pi_tree.verify();
            pom.inverse_pi_tree.verify();

            std::vector<uint64_t> pom_pi_vector = pom.get_pi_vector();
            std::vector<uint64_t> pom_inv_pi_vector = pom.get_inverse_pi_vector();
            // std::vector<int64_t> pom_value_vector = pom.get_value_vector();

            // stool::Printer::print("POM_pi    ", pom_pi_vector);
            // stool::Printer::print("Correct   ", npom.pi_list);
            // stool::Printer::print("POM_inv_pi", pom_inv_pi_vector);
            // stool::Printer::print("Correct   ", npom.inverse_pi_list);

            // pom.pi_tree.print();
            // pom.pi_tree.print_info();

            // stool::Printer::print("Value vec", pom_value_vector);
            // stool::Printer::print("Correct  ", npom.value_list);

            stool::equal_check(pom_pi_vector, npom.pi_list);
            stool::equal_check(pom_inv_pi_vector, npom.inverse_pi_list);
            // stool::equal_check(pom_value_vector, npom.value_list);
        }

        void permutation_test_random_delete(NaivePOM &npom, VPomPermutation &pom, std::mt19937_64 &mt64)
        {
            uint64_t size = npom.pi_list.size();
            std::uniform_int_distribution<uint64_t> get_rand_uni_int(0, size - 1);
            uint64_t pi_index = get_rand_uni_int(mt64);

            // std::cout << "Delete: pi = " << pi_index << std::endl;

            npom.erase(pi_index);
            pom.erase(pi_index);

            // std::cout << "PI:" << std::endl;
            // pom.pi_tree.print_info();
            // pom.pi_tree.print();
            // std::cout << "INV_PI:" << std::endl;
            // pom.inverse_pi_tree.print_info();
            // pom.inverse_pi_tree.print();

            pom.pi_tree.verify();
            pom.inverse_pi_tree.verify();

            std::vector<uint64_t> pom_pi_vector = pom.get_pi_vector();
            std::vector<uint64_t> pom_inv_pi_vector = pom.get_inverse_pi_vector();
            // std::vector<int64_t> pom_value_vector = pom.get_value_vector();

            // pom.pi_tree.print();
            // pom.pi_tree.print_info();

            // stool::Printer::print("Value vec", pom_value_vector);
            // stool::Printer::print("Correct  ", npom.value_list);

            stool::equal_check(pom_pi_vector, npom.pi_list);
            stool::equal_check(pom_inv_pi_vector, npom.inverse_pi_list);
            // stool::equal_check(pom_value_vector, npom.value_list);
        }
        void permutation_test2(uint64_t item_num, uint64_t degree, uint64_t seed)
        {
            NaivePOM npom;
            VPomPermutation pom;
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
    }
}
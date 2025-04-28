#pragma once
#include "./pom_tree.hpp"

namespace stool
{
        namespace old_implementations
    {
    class PermutationOrderMaintenance
    {
        POMTree pi_tree;
        POMTree inverse_pi_tree;
        uint8_t page_size_limit;

    public:
        void initialize(uint8_t _page_size_limit)
        {
            this->page_size_limit = _page_size_limit;
            this->pi_tree.initialize(this->page_size_limit);
            this->inverse_pi_tree.initialize(this->page_size_limit);
        }
        void build(std::vector<uint64_t> &pi)
        {
            uint64_t counter = 0;
            this->clear();

            for (uint64_t i = 0; i < pi.size(); i++)
            {
                counter++;
                if (counter % 100000 == 0)
                {
                    std::cout << "+" << std::flush;
                }

                this->pi_tree.insert(0);
                this->inverse_pi_tree.insert(0);
            }

            counter = 0;
            std::cout << std::endl;

            for (uint64_t i = 0; i < pi.size(); i++)
            {
                counter++;
                if (counter % 100000 == 0)
                {
                    std::cout << "+" << std::flush;
                }
                std::pair<POMTreeNode *, int64_t> pointer1 = this->pi_tree.get_subnode_index(i);
                std::pair<POMTreeNode *, int64_t> pointer2 = this->inverse_pi_tree.get_subnode_index(pi[i]);
                this->pi_tree.udpate_subnode(i, pointer2.first, pointer2.second);
                this->inverse_pi_tree.udpate_subnode(pi[i], pointer1.first, pointer1.second);
            }
            std::cout << std::endl;
        }

        void insert(int64_t pi_index, int64_t inverse_pi_index)
        {
            // std::cout << "Insert: " << pi_index << "/" << inverse_pi_index << std::endl;
            // std::cout << "\033[31m" << std::endl;
            this->pi_tree.insert(pi_index);
            assert(this->pi_tree.check_invariant(false));

            // std::cout << "\033[32m" << std::endl;
            this->inverse_pi_tree.insert(inverse_pi_index);
            assert(this->inverse_pi_tree.check_invariant(false));
            // std::cout << "\033[m" << std::endl;

            std::pair<POMTreeNode *, int64_t> pointer1 = this->pi_tree.get_subnode_index(pi_index);
            std::pair<POMTreeNode *, int64_t> pointer2 = this->inverse_pi_tree.get_subnode_index(inverse_pi_index);

            pointer1.first->items[pointer1.second].link = pointer2.first;
            pointer1.first->items[pointer1.second].link_subindex = pointer2.second;
            pointer2.first->items[pointer2.second].link = pointer1.first;
            pointer2.first->items[pointer2.second].link_subindex = pointer1.second;
            assert(this->pi_tree.check_invariant(true));
            assert(this->inverse_pi_tree.check_invariant(true));
        }
        void erase(int64_t pi_index)
        {
            uint64_t inverse_pi_index = this->access(pi_index);
            this->pi_tree.erase(pi_index);
            this->inverse_pi_tree.erase(inverse_pi_index);
        }
        void clear()
        {
            this->pi_tree.clear();
            this->inverse_pi_tree.clear();
        }

        void move_pi_index(int64_t from, int64_t to)
        {
            int64_t inverse_pi_index = this->access(from);
            if (from < to)
            {
                this->insert(to + 1, inverse_pi_index);
                this->erase(from);
            }
            else if (from > to)
            {
                this->erase(from);
                this->insert(to, inverse_pi_index);
            }
        }
        /*
        void swap_pi_index(int64_t pi_index1, int64_t pi_index2)
        {
            assert(pi_index1 >= 0);
            assert(pi_index1 < this->size());
            assert(pi_index2 >= 0);
            assert(pi_index2 < this->size());

            if (pi_index1 != pi_index2)
            {
                int64_t inverse_pi_index1 = this->access(pi_index1);
                int64_t inverse_pi_index2 = this->access(pi_index2);
                std::pair<POMTreeNode*, int64_t> idx1 = this->pi_tree.get_subnode_index(pi_index1);
                std::pair<POMTreeNode*, int64_t> idx2 = this->pi_tree.get_subnode_index(pi_index2);

                std::pair<POMTreeNode*, int64_t> ridx1 = this->inverse_pi_tree.get_subnode_index(inverse_pi_index1);
                std::pair<POMTreeNode*, int64_t> ridx2 = this->inverse_pi_tree.get_subnode_index(inverse_pi_index2);

                this->pi_tree.udpate_subnode(pi_index1, ridx2.first, ridx2.second);
                this->pi_tree.udpate_subnode(pi_index2, ridx1.first, ridx1.second);
                this->inverse_pi_tree.udpate_subnode(inverse_pi_index1, idx2.first, idx2.second);
                this->inverse_pi_tree.udpate_subnode(inverse_pi_index2, idx1.first, idx1.second);
            }
        }
        */
        /*
        void swap_inverse_pi_index(int64_t inverse_pi_index1, int64_t inverse_pi_index2)
        {
            assert(pi_index1 >= 0);
            assert(pi_index1 < this->size());
            assert(pi_index2 >= 0);
            assert(pi_index2 < this->size());
        }
        */

        uint64_t size() const
        {
            return this->pi_tree.size();
        }
        int64_t access(int64_t pi_index) const
        {
            assert(pi_index >= 0);
            assert(pi_index < this->pi_tree.size());

#ifdef TIME_DEBUG
            std::chrono::system_clock::time_point start, end;
            start = std::chrono::system_clock::now();
#endif
            std::pair<POMTreeNode *, int64_t> pointer = this->pi_tree.get_subnode_index(pi_index);
#ifdef TIME_DEBUG
            end = std::chrono::system_clock::now();
            uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            stool::pom_access_time += elapsed;
            stool::pom_access_count += 1;
#endif


            POMTreeNodeItem &item = pointer.first->items[pointer.second];

            int64_t r = 0;

            if (item.link != nullptr && item.link_subindex < item.link->items.size())
            {
                r = item.link->compute_subnode_rank(item.link_subindex);
            }
            else
            {
                r = -1;
            }

            return r;
        }
        int64_t inverse(int64_t inverse_pi_index) const
        {
            assert(inverse_pi_index >= 0);
            assert(inverse_pi_index < this->inverse_pi_tree.size());

#ifdef TIME_DEBUG
            std::chrono::system_clock::time_point start, end;
            start = std::chrono::system_clock::now();
#endif
            std::pair<POMTreeNode *, int64_t> pointer = this->inverse_pi_tree.get_subnode_index(inverse_pi_index);
#ifdef TIME_DEBUG
            end = std::chrono::system_clock::now();
            uint64_t elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            stool::pom_access_time += elapsed;
            stool::pom_access_count += 1;
#endif

            POMTreeNodeItem &item = pointer.first->items[pointer.second];

            int64_t r = 0;
            if (item.link != nullptr && item.link_subindex < item.link->items.size())
            {
                r = item.link->compute_subnode_rank(item.link_subindex);

            }
            else
            {
                r = -1;
            }
            return r;
        }

        std::vector<uint64_t> get_pi_vector() const
        {
            std::vector<uint64_t> r;
            uint64_t size = this->size();
            for (uint64_t i = 0; i < size; i++)
            {
                r.push_back(this->access(i));
            }
            return r;
        }

        std::vector<uint64_t> get_inverse_pi_vector() const
        {
            std::vector<uint64_t> r;
            uint64_t size = this->size();
            for (uint64_t i = 0; i < size; i++)
            {
                r.push_back(this->inverse(i));
            }
            return r;
        }
        void print() const
        {
            std::cout << "=== PermutationOrderMaintenance ===" << std::endl;
            this->pi_tree.print();
            this->inverse_pi_tree.print();
        }
    };
    }
}

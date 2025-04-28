#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <list>
#include <stack>
#include <cassert>
#include <unordered_map>
#include "../../time_debug.hpp"
#include "stool/include/stool.hpp"
namespace stool
{
    namespace old_implementations
    {
    using POMLink = std::pair<int64_t, uint8_t>;
    const POMLink EMPTY_POM_LINK = POMLink(-1, 0);
    const POMLink DUMMY_POM_LINK = POMLink(-2, 0);

    bool __vpom_msg_flag = false;

    template <typename T = char>
    class VectorizedPomTree
    {
        std::vector<int64_t> parent_vector;
        std::vector<int64_t> child_vector;
        std::vector<int64_t> tree_size_vector;
        std::vector<uint8_t> page_size_vector;
        std::vector<POMLink> link_vector;
        std::vector<T> value_vector;

#ifdef DEBUG
        std::vector<int64_t> id_vector;
        uint64_t id_counter = 0;
        uint64_t tmp_id = 0;
#endif

        std::stack<int64_t> unused_internal_node_indexes;
        std::stack<int64_t> unused_leaf_indexes;
        int64_t internal_node_count = 0;
        int64_t root_index = -1;
        int64_t page_size_limit = 4;
        bool _use_value_vector = false;

        VectorizedPomTree<T> *linkedVPomTree = nullptr;
        bool _is_pi_permutation = true;

    public:
        void initialize(int64_t _page_size_limit, VectorizedPomTree<T> *_linkedVPomTree, bool __use_value_vector)
        {
            if (_page_size_limit < 4)
            {
                throw std::runtime_error("Error:initialize");
            }
            this->clear();
            this->page_size_limit = _page_size_limit;
            this->linkedVPomTree = _linkedVPomTree;
            this->_use_value_vector = __use_value_vector;
        }
        void clear()
        {
            this->parent_vector.resize(0);
            this->child_vector.resize(0);
            this->tree_size_vector.resize(0);
            this->page_size_vector.resize(0);
            this->link_vector.resize(0);
            this->value_vector.resize(0);

#ifdef DEBUG
            this->id_vector.resize(0);
            this->id_counter = 0;
#endif
            while (this->unused_internal_node_indexes.size() > 0)
            {
                this->unused_internal_node_indexes.pop();
            }
            while (this->unused_leaf_indexes.size() > 0)
            {
                this->unused_leaf_indexes.pop();
            }
            this->internal_node_count = 0;
            this->linkedVPomTree = nullptr;
            this->page_size_limit = 4;
            this->root_index = -1;

            this->link_vector.shrink_to_fit();
            this->parent_vector.shrink_to_fit();
            this->page_size_vector.shrink_to_fit();
            this->child_vector.shrink_to_fit();
            this->tree_size_vector.shrink_to_fit();
            this->value_vector.shrink_to_fit();
        }

        /* Get-properties */
    public:
        std::string name() const
        {
            return this->_is_pi_permutation ? "PI" : "INVPI";
        }
        bool is_low_density() const
        {
            if (this->parent_vector.size() > 100)
            {
                return (int64_t)this->parent_vector.size() < this->unused_node_indexes_count() * 2;
            }
            else
            {
                return false;
            }
        }
        int64_t unused_node_indexes_count() const
        {
            return this->unused_internal_node_indexes.size() + this->unused_leaf_indexes.size();
        }
        int64_t get_global_element_index(POMLink pointer) const
        {
            assert(!this->is_free_node_index(pointer.first));
            assert(pointer.second < this->get_page_size(pointer.first));
            int64_t r = this->get_subtree_global_element_index(pointer);
            int64_t current_node = pointer.first;
            while (!this->is_root(current_node))
            {
                assert(current_node >= 0 && current_node < (int64_t)this->parent_vector.size());
                int64_t parent = this->parent_vector[current_node];
                int64_t edge_index = this->get_parent_edge_index(current_node);
                if (edge_index > 0)
                {
                    r += this->count_preceding_elements_in_subtree(parent, edge_index);
                }
                current_node = parent;
            }
            return r;
        }
        std::vector<POMLink> get_sorted_elements() const
        {
            std::vector<POMLink> r;
            if (this->root_index != -1)
            {
                this->get_sorted_elements_sub(this->root_index, r);
            }
            return r;
        }
        std::vector<int64_t> get_sorted_id_vectors() const
        {
            std::vector<POMLink> r1 = this->get_sorted_elements();
            std::vector<int64_t> r2;
            for ([[maybe_unused]]auto it : r1)
            {
#ifdef DEBUG
                r2.push_back(this->get_id(it.first, it.second));
#else
                r2.push_back(-1);
#endif
            }

            return r2;
        }
        std::vector<int64_t> get_postorder_nodes() const
        {
            std::vector<int64_t> r;
            if (this->root_index != -1)
            {
                this->get_postorder_nodes_sub(this->root_index, r);
            }

            return r;
        }
        void get_sorted_elements_sub(int64_t node_index, std::vector<POMLink> &output) const
        {
            int64_t size = this->get_page_size(node_index);
            if (this->is_leaf(node_index))
            {
                for (int64_t i = 0; i < size; i++)
                {
                    output.push_back(POMLink(node_index, i));
                }
            }
            else
            {
                // int64_t cindex = this->get_child_vector_index(node_index);
                for (int64_t i = 0; i <= size; i++)
                {
                    int64_t child_index = this->get_child_index(node_index, i);
                    this->get_sorted_elements_sub(child_index, output);
                    if (i < size)
                    {
                        output.push_back(POMLink(node_index, i));
                    }
                }
            }
        }

        int64_t get_height(int64_t node_index) const
        {
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            if (this->is_leaf(node_index))
            {
                return 1;
            }
            else
            {
                int64_t fst_child_index = this->get_child_vector_index(node_index);
                assert(fst_child_index >= 0 && fst_child_index < (int64_t)this->child_vector.size());
                int64_t child_index = this->child_vector[fst_child_index];

                return 1 + this->get_height(child_index);
            }
        }
        int64_t size() const
        {
            if (this->root_index == -1)
            {
                return 0;
            }
            else
            {
                return this->get_tree_size(this->root_index);
            }
        }
        POMLink get_link(POMLink pointer) const
        {
            int64_t lindex = this->get_link_vector_index(pointer.first);
            return this->link_vector[lindex + pointer.second];
        }
        T get_value(POMLink pointer) const
        {
            int64_t lindex = this->get_link_vector_index(pointer.first);
            if (this->is_used_value_vector())
            {
                return this->value_vector[lindex + pointer.second];
            }
            else
            {
                throw std::runtime_error("Error: the value vector is not used.");
            }
        }

        int64_t get_page_size(int64_t node_index) const
        {
            assert(node_index >= 0 && node_index < (int64_t)this->page_size_vector.size());
            return this->page_size_vector[node_index];
        }

    private:
        int64_t get_link_block_limit_size() const
        {
            return this->page_size_limit + 1;
        }
        int64_t get_child_block_limit_size() const
        {
            return this->page_size_limit + 2;
        }

        int64_t get_link_vector_index(int64_t node_index) const
        {
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            assert(node_index * (this->page_size_limit + 1) < (int64_t)this->link_vector.size());
            return node_index * (this->page_size_limit + 1);
        }
        int64_t get_child_vector_index(int64_t internal_node_index) const
        {
            assert(internal_node_index >= 0 && internal_node_index < internal_node_count);
            int64_t p = internal_node_index * (this->page_size_limit + 2);
            assert(p >= 0 && p < (int64_t)this->child_vector.size());
            return p;
        }
        int64_t get_child_index(int64_t internal_node_index, int64_t ith) const
        {
            uint64_t gindex = this->get_child_vector_index(internal_node_index);
            assert(gindex + ith < this->child_vector.size());
            return this->child_vector[gindex + ith];
        }

        int64_t get_children_count(int64_t node_index) const
        {
            assert(node_index >= 0 && node_index < (int64_t)this->page_size_vector.size());
            return this->page_size_vector[node_index] + 1;
        }

        int64_t get_tree_size(int64_t node_index) const
        {
            int64_t size = this->get_page_size(node_index);
            bool is_leaf = this->is_leaf(node_index);
            if (is_leaf)
            {
                return size;
            }
            else
            {
                int64_t x = size;
                int64_t fst_child_index = this->get_child_vector_index(node_index);
                for (int64_t i = 0; i <= size; i++)
                {
                    x += this->tree_size_vector[fst_child_index + i];
                }
                return x;
            }
        }

        int64_t get_tree_size(int64_t node_index, uint8_t child_index) const
        {
            // int64_t size = this->get_page_size(node_index);
            int64_t fst_child_index = this->get_child_vector_index(node_index);
            return this->tree_size_vector[fst_child_index + child_index];
        }

        int64_t get_edge_index_to_child(int64_t node_index, int64_t child_index) const
        {
            int64_t size = this->get_page_size(node_index);
            int64_t cindex = this->get_child_vector_index(node_index);
            for (int64_t i = 0; i <= size; i++)
            {
                assert(cindex + i < (int64_t)this->child_vector.size());
                if (this->child_vector[cindex + i] == child_index)
                {
                    return i;
                }
            }
            return -1;
        }
        int64_t get_parent_edge_index(int64_t node_index) const
        {
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            int64_t parent = this->parent_vector[node_index];

            if (parent == -1)
            {
                return -1;
            }
            else
            {
                int64_t p = this->get_edge_index_to_child(parent, node_index);
                if (p == -1)
                {
                    this->print_info(node_index);
                    this->print_info(parent);
                    throw std::logic_error("Error:get_edge_index");
                }
                return p;
            }
        }
        void get_postorder_nodes_sub(int64_t node_index, std::vector<int64_t> &output) const
        {
            assert(node_index >= 0);

            int64_t size = this->get_page_size(node_index);
            if (!this->is_leaf(node_index))
            {
                for (int64_t i = 0; i <= size; i++)
                {
                    int64_t child_index = this->get_child_index(node_index, i);
#ifdef DEBUG
                    if (child_index < 0)
                    {
                        throw std::logic_error("Error: get_postorder_nodes_sub, node_index: " + std::to_string(node_index));
                    }
#endif
                    assert(child_index >= 0);
                    this->get_postorder_nodes_sub(child_index, output);
                }
            }
            output.push_back(node_index);
        }
        int64_t get_subtree_global_element_index(POMLink pointer) const
        {
            if (this->is_leaf(pointer.first))
            {
                return pointer.second;
            }
            else
            {
                int64_t cindex = this->get_child_vector_index(pointer.first);
                int64_t r = 0;
                for (int64_t i = 0; i <= pointer.second; i++)
                {
                    r += this->tree_size_vector[cindex + i];
                }
                return r + pointer.second;
            }
        }

#ifdef DEBUG
        int64_t get_id(int64_t node_index, int64_t element_index) const
        {
            int64_t fst_link_index = this->get_link_vector_index(node_index);
            return this->id_vector[fst_link_index + element_index];
        }
#endif

        /* Set-Properties */
    public:
        void set_pi_permutation_flag(bool value)
        {
            this->_is_pi_permutation = value;
        }
        void set_link(POMLink pointer, POMLink link)
        {
            assert(pointer.first < (int64_t)this->parent_vector.size());
            assert(pointer.first >= 0);
            int64_t lindex = this->get_link_vector_index(pointer.first);
            this->link_vector[lindex + pointer.second] = link;
        }
        void set_value(POMLink pointer, T value)
        {
            assert(pointer.first < (int64_t)this->parent_vector.size());
            assert(pointer.first >= 0);
            int64_t lindex = this->get_link_vector_index(pointer.first);
            if (this->is_used_value_vector())
            {
                this->value_vector[lindex + pointer.second] = value;
            }
        }

    private:
#ifdef DEBUG
        void set_id(int64_t node_index, int64_t element_index, int64_t id)
        {
            int64_t fst_link_index = this->get_link_vector_index(node_index);
            this->id_vector[fst_link_index + element_index] = id;
        }
#endif

        /* Is-Properties */

    private:
        bool is_used_value_vector() const
        {
            return this->_use_value_vector;
        }
        bool is_linked() const
        {
            return this->linkedVPomTree != nullptr;
        }
        bool is_dummy_internal_node(int64_t internal_node_index) const
        {
            bool b = this->is_leaf(internal_node_index);
            if (b)
            {
                return false;
            }
            else
            {
                int64_t link_fst_index = this->get_link_vector_index(internal_node_index);
                assert(internal_node_index < (int64_t)this->page_size_vector.size());
                int64_t size = this->page_size_vector[internal_node_index];
                return this->link_vector[link_fst_index].first == -2 && size == 1;
            }
        }

        bool is_free_node_index(int64_t node_index) const
        {
            return this->page_size_vector[node_index] == UINT8_MAX;
        }

        /* Search */
    public:
        POMLink search_element_by_index(int64_t element_index) const
        {
            assert(this->root_index != -1);

            int64_t current_node_index = this->root_index;
            while (true)
            {

                std::pair<uint8_t, int64_t> idx = this->lower_bound(current_node_index, element_index);
                if (this->is_leaf(current_node_index))
                {
                    return POMLink(current_node_index, element_index);
                }
                else
                {
                    element_index = idx.second;

                    int64_t page_size = this->get_page_size(current_node_index);
                    int64_t child_tree_size = this->get_tree_size(current_node_index, idx.first);
                    if (idx.first < page_size && (int64_t)idx.second == child_tree_size)
                    {
                        return POMLink(current_node_index, idx.first);
                    }
                    else
                    {
                        current_node_index = this->get_child_index(current_node_index, idx.first);
                    }
                }
            }

            return EMPTY_POM_LINK;
        }

    private:
        std::pair<uint8_t, int64_t> lower_bound(int64_t node_index, int64_t node_index_in_subtree) const
        {
            int64_t page_size = this->get_page_size(node_index);

            if (this->is_leaf(node_index))
            {
                if (node_index_in_subtree < page_size)
                {
                    return std::pair<uint8_t, int64_t>(node_index_in_subtree, 0);
                }
                else
                {
                    throw std::runtime_error("Error:lower_bound");
                }
            }
            else
            {
                uint64_t fst_child_index = this->get_child_vector_index(node_index);
                int64_t current_subtree_node_count = 0;
                int64_t idx = node_index_in_subtree;

                for (int64_t i = 0; i < page_size; i++)
                {
                    current_subtree_node_count += this->tree_size_vector[fst_child_index + i] + 1;

                    if ((current_subtree_node_count - 1) >= node_index_in_subtree)
                    {
                        return std::pair<uint8_t, int64_t>(i, idx);
                    }

                    idx = node_index_in_subtree - current_subtree_node_count;
                }

                return std::pair<uint8_t, int64_t>(page_size, idx);
            }
        }

        POMLink search_insertion_leaf_index(int64_t rank) const
        {
            assert(this->root_index != -1);

            int64_t current_node_index = this->root_index;
            while (!this->is_leaf(current_node_index))
            {
                // uint64_t fst_child_index = this->get_child_vector_index(current_node_index);
                std::pair<uint8_t, int64_t> idx = this->lower_bound(current_node_index, rank);
                rank = idx.second;
                current_node_index = this->get_child_index(current_node_index, idx.first);
            }

            return POMLink(current_node_index, rank);
        }

        /* Methods */

        int64_t count_preceding_elements_in_subtree(int64_t node_index, uint8_t child_index) const
        {
            if (this->is_leaf(node_index))
            {
                return child_index;
            }
            else
            {
                int64_t cindex = this->get_child_vector_index(node_index);
                int64_t r = 0;
                for (int64_t i = 0; i < child_index; i++)
                {
                    assert(cindex + i < (int64_t)this->tree_size_vector.size());
                    r += this->tree_size_vector[cindex + i];
                }
                return r + child_index;
            }
        }

        int64_t split(int64_t node_index)
        {
            assert(this->get_page_size(node_index) > this->page_size_limit);
            int64_t move_length = (this->get_page_size(node_index) / 2) + 1;

            if (this->is_root(node_index))
            {
#ifdef DEBUG
                if (__vpom_msg_flag)
                {
                    std::cout << "Split root, node_index: " << node_index << "/" << this->is_leaf(node_index) << std::endl;
                }
#endif

                int64_t newLeftNode = 0;
                if (this->is_leaf(node_index))
                {
                    newLeftNode = this->create_leaf(-1);
                }
                else
                {
                    newLeftNode = this->create_new_internal_node(node_index);
                }
                int64_t newRootIndex = this->create_new_internal_node(newLeftNode, node_index);
                this->initialize_empty_node_using_dummy_element(newRootIndex);

                left_move_processing(newLeftNode, node_index, newRootIndex, move_length);
                this->root_index = newRootIndex;
                return newRootIndex;
            }
            else
            {
#ifdef DEBUG
                if (__vpom_msg_flag)
                {
                    std::cout << "Split node (not root), node_index: " << node_index << std::endl;
                }
#endif

                assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
                int64_t parent = this->parent_vector[node_index];
                int64_t newLeftNode = -1;
                if (this->is_leaf(node_index))
                {
                    newLeftNode = this->create_leaf(parent);
                }
                else
                {
                    newLeftNode = this->create_new_internal_node(node_index);
                }
                left_move_processing(newLeftNode, node_index, this->parent_vector[node_index], move_length);

                return parent;
            }
        }

        /* Public Update Methods */
    public:
        POMLink insert(uint64_t rank)
        {
            if (this->is_low_density())
            {
                this->defragmentation();
            }

#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << this->name() << ", insert(" << rank << ")" << std::endl;
            }
#endif
            if (this->root_index == -1)
            {
                if (rank == 0)
                {
                    int64_t _root_index = this->create_leaf(-1);
                    this->root_index = _root_index;
                    assert(!this->is_free_node_index(this->root_index));
                    this->add_link(this->root_index, EMPTY_POM_LINK);
                    return POMLink(this->root_index, 0);
                }
                else
                {
                    throw std::runtime_error("Error: insert1");
                }
            }
            else
            {
                POMLink ins_index = this->search_insertion_leaf_index(rank);
                int64_t size = this->get_page_size(ins_index.first);
                assert(ins_index.second <= size);
                if (size > this->page_size_limit)
                {
                    throw std::logic_error("Error: insert2");
                }

                this->right_shift_in_leaf(ins_index.first, ins_index.second, EMPTY_POM_LINK, T());

                int64_t current_node = ins_index.first;
                while (current_node != -1 && this->get_page_size(current_node) > this->page_size_limit)
                {
                    current_node = this->split(current_node);
                }

                return this->search_element_by_index(rank);
            }
        }

        bool is_leaf(int64_t node_index) const
        {
            return node_index >= this->internal_node_count;
        }
        bool is_root(int64_t node_index) const
        {
            return node_index == this->root_index;
        }

        bool erase(int64_t global_element_index)
        {

            if (this->is_low_density())
            {
                this->defragmentation();
            }

// uint64_t minLeafCount = this->page_size_limit / 2;
#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << this->name() << ", erase(" << global_element_index << ")" << std::endl;
            }
#endif
            POMLink locatedElement = this->search_element_by_index(global_element_index);
            int64_t target_node = -1;

            if (locatedElement.first != -1 && this->size() == 1)
            {
                this->temporarily_delete(this->root_index, 0, 1);
                this->release(this->root_index);
                this->root_index = -1;
            }
            else
            {
                if (this->is_leaf(locatedElement.first))
                {

                    this->replace_link_with_dummy_link(locatedElement.first, locatedElement.second);
                    this->remove_consecutive_elements(locatedElement.first, locatedElement.second, 1, false, true);
                    target_node = locatedElement.first;

                    // assert(pos < node->get_page_size());
                    // POMTreeNodeUpdate::erase_subnode_of_leaf(*node, pos);
                    // target_node = node;
                }
                else
                {
                    // throw std::runtime_error("not implemented2");
                    POMLink leftLeafIndex = this->search_element_by_index(global_element_index - 1);
                    POMLink rightLeafIndex = this->search_element_by_index(global_element_index + 1);
                    uint64_t leftLeafCount = this->get_page_size(leftLeafIndex.first);
                    uint64_t rightLeafCount = this->get_page_size(rightLeafIndex.first);

                    if (leftLeafCount > rightLeafCount)
                    {
                        this->delete_and_right_shift(locatedElement.first, locatedElement.second, leftLeafIndex.first);
                        target_node = leftLeafIndex.first;
                    }
                    else
                    {
                        this->delete_and_left_shift(locatedElement.first, locatedElement.second, rightLeafIndex.first);
                        target_node = rightLeafIndex.first;
                    }
                }
                this->balance_tree_for_deletion(target_node);
            }

            return true;
        }

    private:
        void release(int64_t node_index)
        {
            int64_t lindex = this->get_link_vector_index(node_index);
            for (int64_t i = 0; i <= this->page_size_limit; i++)
            {
                this->link_vector[lindex + i] = EMPTY_POM_LINK;
#ifdef DEBUG
                this->set_id(node_index, i, -1);
#endif
            }

            bool b = this->is_leaf(node_index);
            if (!b)
            {
                int64_t cindex = this->get_child_vector_index(node_index);
                for (int64_t i = 0; i <= this->page_size_limit + 1; i++)
                {
                    assert(cindex + i >= 0 && cindex + i < (int64_t)this->child_vector.size());
                    this->child_vector[cindex + i] = -1;
                    this->tree_size_vector[cindex + i] = -1;
                }
            }
            this->page_size_vector[node_index] = UINT8_MAX;
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            this->parent_vector[node_index] = -1;

            if (b)
            {
                this->unused_leaf_indexes.push(node_index);
            }
            else
            {
                this->unused_internal_node_indexes.push(node_index);
            }
        }

        /* Update Operation  */

        uint64_t balance_tree_for_deletion(int64_t node_index)
        {
            int64_t minLeafCount = this->page_size_limit / 2;

            int64_t target_node = node_index;
            int64_t current_root = this->root_index;

            while (target_node != -1)
            {
                int64_t pageSizeOfCurrentNode = this->get_page_size(target_node);
                if (pageSizeOfCurrentNode >= minLeafCount)
                {
                    break;
                }
                else
                {
                    if (!this->is_root(target_node))
                    {
                        assert(target_node >= 0 && target_node < (int64_t)this->parent_vector.size());
                        int64_t parent = this->parent_vector[target_node];
                        int64_t parentPageSize = this->get_page_size(parent);
                        int64_t edgePos = this->get_parent_edge_index(target_node);

                        if (edgePos > 0)
                        {
                            int64_t leftChild = this->get_child_index(parent, edgePos - 1);
                            int64_t leftChildPageSize = this->get_page_size(leftChild);

                            if (leftChildPageSize > minLeafCount)
                            {
                                this->shift_from_left_to_right(parent, edgePos - 1);
                                target_node = parent;
                            }
                            else if (parentPageSize > 1)
                            {
                                this->merge(parent, edgePos - 1);
                                target_node = parent;
                            }
                            else
                            {
                                assert(parentPageSize == 1);
                                target_node = this->special_merge(parent);
                            }
                        }
                        else if (edgePos < parentPageSize)
                        {
                            int64_t rightChild = this->get_child_index(parent, edgePos + 1);
                            int64_t rightChildPageSize = this->get_page_size(rightChild);

                            if (rightChildPageSize > minLeafCount)
                            {
                                this->shift_from_right_to_left(parent, edgePos);
                                target_node = parent;
                            }
                            else if (parentPageSize > 1)
                            {
                                this->merge(parent, edgePos);
                                target_node = parent;
                            }
                            else
                            {
                                assert(parentPageSize == 1);
                                target_node = this->special_merge(parent);
                            }
                        }
                        else
                        {
                            throw std::logic_error("Erase Error!");
                        }
                    }
                    else
                    {
                        target_node = -1;
                    }
                }
            }

            return current_root;
        }

        void shift_from_right_to_left(int64_t node_index, uint8_t element_index)
        {
#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << "shift_from_left_to_left, node_index: " << node_index << ", element_index: " << (int)element_index << std::endl;
            }
#endif

            int64_t leftChild = this->get_child_index(node_index, element_index);
            int64_t rightChild = this->get_child_index(node_index, element_index + 1);

            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            int64_t tmp_parent = this->parent_vector[node_index];
            this->parent_vector[node_index] = -1;

            int64_t pageSizeOfLeftChild = this->get_page_size(leftChild);
            this->insert_dummy_elements(leftChild, pageSizeOfLeftChild, 1, true, true);
            this->replace_dummy_element_with_the_moved_element(leftChild, node_index, pageSizeOfLeftChild, element_index);
            // int64_t childrenCountOfRightChild = this->get_children_count(rightChild);
            int64_t childrenCountOfLeftChild = this->get_children_count(leftChild);
            if (!this->is_leaf(leftChild))
            {
                this->replace_dummy_child_with_the_moved_child(leftChild, rightChild, childrenCountOfLeftChild - 1, 0);
            }

            this->replace_dummy_element_with_the_moved_element(node_index, rightChild, element_index, 0);
            this->remove_consecutive_elements(rightChild, 0, 1, false, false);

            this->parent_vector[node_index] = tmp_parent;
        }

        void shift_from_left_to_right(int64_t node_index, uint8_t element_index)
        {
            int64_t leftChild = this->get_child_index(node_index, element_index);
            int64_t rightChild = this->get_child_index(node_index, element_index + 1);

#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << "shift_from_left_to_right, node_index: " << node_index << ", element_index: " << (int)element_index << ", L: " << leftChild << ", R: " << rightChild << std::endl;
            }
#endif

            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            int64_t tmp_parent = this->parent_vector[node_index];
            this->parent_vector[node_index] = -1;

            this->insert_dummy_elements(rightChild, 0, 1, false, true);

            this->replace_dummy_element_with_the_moved_element(rightChild, node_index, 0, element_index);
            int64_t childrenCountOfLeftChild = this->get_children_count(leftChild);
            if (!this->is_leaf(rightChild))
            {
                this->replace_dummy_child_with_the_moved_child(rightChild, leftChild, 0, childrenCountOfLeftChild - 1);
            }

            int64_t pageSizeOfLeftChild = this->get_page_size(leftChild);
            this->replace_dummy_element_with_the_moved_element(node_index, leftChild, element_index, pageSizeOfLeftChild - 1);
            this->remove_consecutive_elements(leftChild, pageSizeOfLeftChild - 1, 1, true, false);

            this->parent_vector[node_index] = tmp_parent;
        }

        void remove_consecutive_elements(int64_t node_index, int64_t deletion_index, int64_t deletion_length, bool childPlusOne, bool recursivelyUpdateTreeSize)
        {
            int64_t fst_link_index = this->get_link_vector_index(node_index);
            int64_t fst_deleted_link_index = fst_link_index + deletion_index;
            bool is_leaf = this->is_leaf(node_index);
            int64_t size = this->get_page_size(node_index);

            if (deletion_length >= size)
            {
                throw std::runtime_error("Error: remove_consecutive_elements");
            }

            for (int64_t i = 0; i < (int64_t)deletion_length; i++)
            {
                if (this->link_vector[fst_deleted_link_index + i].first != -2)
                {
                    throw std::runtime_error("Error: remove_consecutive_elements");
                }
            }

            int64_t diff1 = size - (deletion_index + deletion_length);
            for (int64_t i = 0; i < diff1; i++)
            {
                this->link_vector[fst_deleted_link_index + i] = this->link_vector[fst_deleted_link_index + deletion_length + i];
#ifdef DEBUG
                int64_t _tmp_id = this->get_id(node_index, deletion_index + deletion_length + i);
                this->set_id(node_index, deletion_index + i, _tmp_id);
#endif
            }
            for (int64_t i = 0; i < (int64_t)deletion_length; i++)
            {
                this->link_vector[fst_link_index + size - i - 1] = EMPTY_POM_LINK;
#ifdef DEBUG
                this->set_id(node_index, size - i - 1, -1);
#endif
            }
            if (this->is_used_value_vector())
            {
                for (int64_t i = 0; i < diff1; i++)
                {
                    this->value_vector[fst_deleted_link_index + i] = this->value_vector[fst_deleted_link_index + deletion_length + i];
                }
                for (int64_t i = 0; i < (int64_t)deletion_length; i++)
                {
                    this->value_vector[fst_link_index + size - i - 1] = T();
                }
            }

            if (!is_leaf)
            {
                int64_t fst_child_index = this->get_child_vector_index(node_index);
                int64_t cdel_index = deletion_index + (childPlusOne ? 1 : 0);
                int64_t fst_deleted_child_index = fst_child_index + cdel_index;

                int64_t csize = this->get_children_count(node_index);
                int64_t diff2 = csize - (cdel_index + deletion_length);

                for (int64_t i = 0; i < diff2; i++)
                {
                    assert(fst_deleted_child_index + i >= 0 && fst_deleted_child_index + i < (int64_t)this->child_vector.size());
                    assert(fst_deleted_child_index + deletion_length + i >= 0 && fst_deleted_child_index + deletion_length + i < (int64_t)this->child_vector.size());

                    this->child_vector[fst_deleted_child_index + i] = this->child_vector[fst_deleted_child_index + deletion_length + i];
                    this->tree_size_vector[fst_deleted_child_index + i] = this->tree_size_vector[fst_deleted_child_index + deletion_length + i];
                }
                for (int64_t i = 0; i < (int64_t)deletion_length; i++)
                {
                    assert(fst_child_index + csize - i - 1 >= 0 && fst_child_index + csize - i - 1 < (int64_t)this->child_vector.size());

                    this->child_vector[fst_child_index + csize - i - 1] = -1;
                    this->tree_size_vector[fst_child_index + csize - i - 1] = -1;
                }
            }
            this->page_size_vector[node_index] = size - deletion_length;
            this->update_links(node_index);

            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            int64_t parent = this->parent_vector[node_index];
            if (parent != -1)
            {
                int64_t edge_index = this->get_parent_edge_index(node_index);
                int64_t cindex = this->get_child_vector_index(parent);
                this->tree_size_vector[cindex + edge_index] = this->get_tree_size(node_index);
            }

            if (recursivelyUpdateTreeSize)
            {
                this->update_recursively_tree_size(node_index);
            }
        }
        void temporarily_move_children(int64_t target_node_index, int64_t source_node_index, int8_t move_length, uint8_t target_index, uint8_t source_index)
        {
            assert(target_node_index >= 0 && target_node_index < (int64_t)this->parent_vector.size());
            assert(source_node_index >= 0 && source_node_index < (int64_t)this->parent_vector.size());

            int64_t target_fst_child_index = this->get_child_vector_index(target_node_index);
            int64_t source_fst_child_index = this->get_child_vector_index(source_node_index);

            int64_t sum_tree_size = 0;

            for (int64_t i = 0; i < (int64_t)move_length; i++)
            {
                int64_t idx1 = target_fst_child_index + target_index + i;
                int64_t idx2 = source_fst_child_index + source_index + i;
                assert(idx1 >= 0 && idx1 < (int64_t)this->child_vector.size());
                assert(idx2 >= 0 && idx2 < (int64_t)this->child_vector.size());

                int64_t child_index = this->child_vector[idx2];
                assert(this->child_vector[idx1] < 0);
                assert(child_index >= 0);
                assert(child_index < (int64_t)this->parent_vector.size());

                this->child_vector[idx1] = child_index;
                this->child_vector[idx2] = -2;
                sum_tree_size += this->tree_size_vector[idx2];
                this->tree_size_vector[idx1] = this->tree_size_vector[idx2];
                this->tree_size_vector[idx2] = 0;
                this->parent_vector[child_index] = target_node_index;
            }

            int64_t parent1 = this->parent_vector[target_node_index];
            if (parent1 >= 0)
            {
                int64_t edge_index = this->get_parent_edge_index(target_node_index);
                int64_t cindex = this->get_child_vector_index(parent1);
                this->tree_size_vector[cindex + edge_index] += sum_tree_size;
            }
            int64_t parent2 = this->parent_vector[source_node_index];
            if (parent2 >= 0)
            {
                int64_t edge_index = this->get_parent_edge_index(source_node_index);
                int64_t cindex = this->get_child_vector_index(parent2);
                this->tree_size_vector[cindex + edge_index] -= sum_tree_size;
            }
        }

        void temporarily_move_elements(int64_t target_node_index, int64_t source_node_index, int8_t move_length, uint8_t target_index, uint8_t source_index)
        {
            int64_t target_fst_link_index = this->get_link_vector_index(target_node_index);
            int64_t source_fst_link_index = this->get_link_vector_index(source_node_index);

            for (int64_t i = 0; i < (int64_t)move_length; i++)
            {
                int64_t idx1 = target_fst_link_index + target_index + i;
                int64_t idx2 = source_fst_link_index + source_index + i;
                assert(idx1 < (int64_t)this->link_vector.size());
                assert(idx2 < (int64_t)this->link_vector.size());

                assert(this->link_vector[idx1].first == -2);
                this->link_vector[idx1] = this->link_vector[idx2];
                this->link_vector[idx2] = DUMMY_POM_LINK;
                this->update_link(POMLink(target_node_index, (target_index + i)));

#ifdef DEBUG
                int64_t _tmp_id = this->get_id(source_node_index, source_index + i);
                this->set_id(target_node_index, target_index + i, _tmp_id);
                this->set_id(source_node_index, source_index + i, -2);
#endif
            }

            if (this->is_used_value_vector())
            {
                for (int64_t i = 0; i < (int64_t)move_length; i++)
                {
                    int64_t idx1 = target_fst_link_index + target_index + i;
                    int64_t idx2 = source_fst_link_index + source_index + i;
                    this->value_vector[idx1] = this->value_vector[idx2];
                    this->value_vector[idx2] = T();
                }
            }
        }

        void temporarily_move_elements_from_right_to_left(int64_t left_node_index, int64_t right_node_index, int64_t copy_length, uint8_t copy_start_index)
        {

            // int64_t left_fst_link_index = this->get_link_vector_index(left_node_index);
            // int64_t right_fst_link_index = this->get_link_vector_index(right_node_index);
            bool is_right_leaf = this->is_leaf(right_node_index);
            int64_t left_size = this->get_page_size(left_node_index);

            assert(left_size + copy_length <= this->page_size_limit);

            this->insert_dummy_elements(left_node_index, 0, copy_length, true, false);
            this->temporarily_move_elements(left_node_index, right_node_index, copy_length, left_size, copy_start_index);
            if (!is_right_leaf)
            {
                this->temporarily_move_children(left_node_index, right_node_index, copy_length + 1, 0, copy_start_index);
            }
        }
        void insert_dummy_elements(int64_t node_index, int64_t element_index, uint8_t length, bool child_plus_one, bool update_tree_size)
        {
            int64_t fst_link_index = this->get_link_vector_index(node_index);
            int64_t page_size = this->get_page_size(node_index);
            int64_t move_length = page_size - element_index;
            bool is_leaf = this->is_leaf(node_index);
            for (int64_t i = 0; i < (int64_t)move_length; i++)
            {
                int64_t d = page_size - 1 - i + length;
                int64_t idx1 = fst_link_index + d;
                int64_t idx2 = fst_link_index + page_size - 1 - i;
                assert(idx1 < (int64_t)this->link_vector.size());
                assert(idx2 < (int64_t)this->link_vector.size());
                this->link_vector[idx1] = this->link_vector[idx2];
                this->update_link(POMLink(node_index, d));
#ifdef DEBUG
                int64_t _tmp_id = this->get_id(node_index, page_size - 1 - i);
                this->set_id(node_index, d, _tmp_id);
#endif
            }

            for (uint64_t i = 0; i < length; i++)
            {
                int64_t idx = fst_link_index + element_index + i;
                assert(idx < (int64_t)this->link_vector.size());
                this->link_vector[idx] = DUMMY_POM_LINK;
#ifdef DEBUG
                this->set_id(node_index, element_index + i, -2);
#endif
            }

            if (this->is_used_value_vector())
            {
                for (int64_t i = 0; i < (int64_t)move_length; i++)
                {
                    int64_t d = page_size - 1 - i + length;
                    int64_t idx1 = fst_link_index + d;
                    int64_t idx2 = fst_link_index + page_size - 1 - i;
                    this->value_vector[idx1] = this->value_vector[idx2];
                }
                for (uint64_t i = 0; i < length; i++)
                {
                    int64_t idx = fst_link_index + element_index + i;
                    this->value_vector[idx] = T();
                }
            }

            if (!is_leaf)
            {
                int64_t fst_child_index = this->get_child_vector_index(node_index);
                int64_t children_count = page_size + 1;
                int64_t insert_child_index = element_index + (child_plus_one ? 1 : 0);
                int64_t move_length2 = children_count - insert_child_index;

                for (int64_t i = 0; i < (int64_t)move_length2; i++)
                {
                    int64_t idx1 = fst_child_index + children_count - 1 - i + length;
                    int64_t idx2 = fst_child_index + children_count - 1 - i;
                    assert(idx1 >= 0 && idx1 < (int64_t)this->child_vector.size());
                    assert(idx2 >= 0 && idx2 < (int64_t)this->child_vector.size());

                    this->child_vector[idx1] = this->child_vector[idx2];
                    this->tree_size_vector[idx1] = this->tree_size_vector[idx2];
                }
                for (uint64_t i = 0; i < length; i++)
                {
                    int64_t idx = fst_child_index + insert_child_index + i;
                    assert(idx >= 0 && idx < (int64_t)this->child_vector.size());
                    this->child_vector[idx] = -2;
                    this->tree_size_vector[idx] = 0;
                }
            }
            this->page_size_vector[node_index] += length;

            if (update_tree_size)
            {
                this->update_recursively_tree_size(node_index);
            }
        }
        void merge(int64_t node_index, uint8_t element_index)
        {
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            int64_t leftTree = this->get_child_index(node_index, element_index);
            int64_t rightTree = this->get_child_index(node_index, element_index + 1);
            assert(this->get_page_size(node_index) > 1);
#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << " merge, node_index: " << node_index << "/ element_index: " << (int)element_index << " / L: " << leftTree << " / R: " << rightTree << std::endl;
            }
#endif

            int64_t tmp_parent = this->parent_vector[node_index];
            this->parent_vector[node_index] = -1;

            int64_t leftTreePageSize = this->get_page_size(leftTree);
            uint64_t move_count = 1 + this->get_page_size(rightTree);

            this->insert_dummy_elements(leftTree, leftTreePageSize, move_count, true, true);
            this->replace_dummy_element_with_the_moved_element(leftTree, node_index, leftTreePageSize, element_index);
            this->replace_child_with_dummy_child(node_index, element_index);
            this->replace_child_with_dummy_child(node_index, element_index + 1);
            this->replace_dummy_child_with_the_new_child(node_index, element_index + 1, leftTree);

            this->remove_consecutive_elements(node_index, element_index, 1, false, false);

            this->temporarily_move_elements(leftTree, rightTree, move_count - 1, leftTreePageSize + 1, 0);
            if (!this->is_leaf(leftTree))
            {

                this->temporarily_move_children(leftTree, rightTree, move_count, leftTreePageSize + 1, 0);
            }
            int64_t cindex = this->get_child_vector_index(node_index);
            this->tree_size_vector[cindex + element_index] = this->get_tree_size(leftTree);

            this->release(rightTree);

            this->parent_vector[node_index] = tmp_parent;
        }
        void replace_child_with_dummy_child(uint64_t node_index, uint8_t child_index)
        {
            int64_t cindex = this->get_child_vector_index(node_index);
            assert(cindex + child_index >= 0 && cindex + child_index < (int64_t)this->child_vector.size());
            int64_t child = this->child_vector[cindex + child_index];
            assert(child >= 0 && child < (int64_t)this->parent_vector.size());
            this->child_vector[cindex + child_index] = -2;
            this->tree_size_vector[cindex + child_index] = 0;
            this->parent_vector[child] = -2;
        }

        void delete_and_right_shift(uint64_t node_index, uint8_t element_index, uint64_t leaf_index)
        {
#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << " delete_and_right_shift, node_index: " << node_index << ", element_index: " << (int)element_index << ", leaf_index: " << leaf_index << std::endl;
            }
#endif

            // uint64_t linkIndexOfLeaf = this->get_link_vector_index(leaf_index);
            uint64_t pageSizeOfLeaf = this->get_page_size(leaf_index);

            this->temporarily_delete(node_index, element_index, 1);
            this->replace_dummy_element_with_the_moved_element(node_index, leaf_index, element_index, pageSizeOfLeaf - 1);
            this->remove_consecutive_elements(leaf_index, pageSizeOfLeaf - 1, 1, false, true);
        }
        void delete_and_left_shift(uint64_t node_index, uint8_t element_index, uint64_t leaf_index)
        {
#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << " delete_and_left_shift, node_index: " << node_index << ", element_index: " << (int)element_index << ", leaf_index: " << leaf_index << std::endl;
            }
#endif

            // uint64_t linkIndexOfLeaf = this->get_link_vector_index(leaf_index);
            this->temporarily_delete(node_index, element_index, 1);
            this->replace_dummy_element_with_the_moved_element(node_index, leaf_index, element_index, 0);
            this->remove_consecutive_elements(leaf_index, 0, 1, false, true);
        }
        void replace_link_with_dummy_link(int64_t node_index, uint8_t element_index)
        {
            int64_t fst_link_index = this->get_link_vector_index(node_index);
            POMLink link = this->link_vector[fst_link_index + element_index];
            if (link.first >= 0 && this->linkedVPomTree != nullptr)
            {
                int64_t fst_link_index2 = this->linkedVPomTree->get_link_vector_index(link.first);
                this->linkedVPomTree->link_vector[fst_link_index2 + link.second] = DUMMY_POM_LINK;
            }
            this->link_vector[fst_link_index + element_index] = DUMMY_POM_LINK;

            if (this->is_used_value_vector())
            {
                this->value_vector[fst_link_index + element_index] = T();
            }
        }
        void temporarily_delete(int64_t node_index, uint8_t element_index, uint8_t delete_length)
        {
            // int64_t fst_link_index = this->get_link_vector_index(node_index);

            for (uint64_t i = 0; i < delete_length; i++)
            {
                this->replace_link_with_dummy_link(node_index, element_index + i);
#ifdef DEBUG
                this->set_id(node_index, element_index + i, -2);
#endif
            }
        }
        int64_t special_merge(int64_t node_index)
        {
            int64_t leftTree = this->get_child_index(node_index, 0);
            int64_t rightTree = this->get_child_index(node_index, 1);
            assert(leftTree >= 0 && leftTree < (int64_t)this->parent_vector.size());
            assert(rightTree >= 0 && rightTree < (int64_t)this->parent_vector.size());

#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << "Speacial Merge! node_index: " << node_index << " / L: " << leftTree << " / R: " << rightTree << std::endl;
            }
#endif

            int64_t leftTreePageSize = this->get_page_size(leftTree);
            uint64_t move_count = 1 + this->get_page_size(rightTree);
            this->insert_dummy_elements(leftTree, leftTreePageSize, move_count, true, false);
            this->replace_dummy_element_with_the_moved_element(leftTree, node_index, leftTreePageSize, 0);
            this->replace_child_with_dummy_child(node_index, 0);
            this->replace_child_with_dummy_child(node_index, 1);
            // this->replace_dummy_child_with_the_new_child(node_index, element_index + 1, leftTree);

            this->temporarily_move_elements(leftTree, rightTree, move_count - 1, leftTreePageSize + 1, 0);
            if (!this->is_leaf(leftTree))
            {
                this->temporarily_move_children(leftTree, rightTree, move_count, leftTreePageSize + 1, 0);
            }

            this->parent_vector[leftTree] = -1;
            this->root_index = leftTree;

            this->release(rightTree);
            this->release(node_index);

            return leftTree;
        }
        void left_move_processing(int64_t left_node_index, int64_t right_node_index, int64_t parent_index, uint64_t move_item_count)
        {
            assert(left_node_index >= 0 && left_node_index < (int64_t)this->parent_vector.size());
            assert(right_node_index >= 0 && right_node_index < (int64_t)this->parent_vector.size());

#ifdef DEBUG
            if (__vpom_msg_flag)
            {
                std::cout << " left_move_processing, L: " << left_node_index << ", R: " << right_node_index << ", P: " << parent_index << ", count: " << move_item_count << std::endl;
            }
#endif

            this->temporarily_move_elements_from_right_to_left(left_node_index, right_node_index, move_item_count - 1, 0);
            bool b = this->is_dummy_internal_node(parent_index);

            if (b)
            {
                this->parent_vector[right_node_index] = -1;
                this->replace_dummy_element_with_the_moved_element(parent_index, right_node_index, 0, move_item_count - 1);
                this->replace_dummy_child_with_the_new_child(parent_index, 0, left_node_index);
                this->replace_dummy_child_with_the_new_child(parent_index, 1, right_node_index);
                this->remove_consecutive_elements(right_node_index, 0, move_item_count, false, false);
            }
            else
            {
                int64_t edge_index = this->get_parent_edge_index(right_node_index);

                this->insert_dummy_elements(parent_index, edge_index, 1, false, false);
                this->replace_dummy_element_with_the_moved_element(parent_index, right_node_index, edge_index, move_item_count - 1);
                this->replace_dummy_child_with_the_new_child(parent_index, edge_index, left_node_index);
                this->remove_consecutive_elements(right_node_index, 0, move_item_count, false, false);
            }
        }
        void replace_dummy_element_with_the_moved_element(int64_t node_index_with_dummy_element, int64_t node_index_with_move_element, uint8_t element_index1, uint8_t element_index2)
        {
            int64_t lindex1 = this->get_link_vector_index(node_index_with_dummy_element);

            int64_t lindex2 = this->get_link_vector_index(node_index_with_move_element);
            assert(this->link_vector[lindex1 + element_index1].first == -2);
            assert(lindex1 + element_index1 < (int64_t)this->link_vector.size());
            assert(lindex1 + element_index2 < (int64_t)this->link_vector.size());

            this->link_vector[lindex1 + element_index1] = this->link_vector[lindex2 + element_index2];
            this->link_vector[lindex2 + element_index2] = DUMMY_POM_LINK;
            this->update_link(POMLink(node_index_with_dummy_element, element_index1));
            if (this->is_used_value_vector())
            {
                this->value_vector[lindex1 + element_index1] = this->value_vector[lindex2 + element_index2];
                this->value_vector[lindex2 + element_index2] = T();
            }

#ifdef DEBUG
            int64_t tmp_id = this->get_id(node_index_with_move_element, element_index2);
            this->set_id(node_index_with_dummy_element, element_index1, tmp_id);
            this->set_id(node_index_with_move_element, element_index2, -2);
#endif
        }

        void replace_dummy_element_with_the_new_element(int64_t node_index, uint8_t element_index, POMLink link, T value)
        {
            int64_t lindex = this->get_link_vector_index(node_index);
            this->link_vector[lindex + element_index] = link;
            if (this->is_used_value_vector())
            {
                this->value_vector[lindex + element_index] = value;
            }

#ifdef DEBUG
            this->set_id(node_index, element_index, this->id_counter++);
#endif
        }

        void replace_dummy_child_with_the_new_child(int64_t node_index_with_dummy_element, uint8_t child_index1, int64_t node_index)
        {
            int64_t cindex1 = this->get_child_vector_index(node_index_with_dummy_element);
            assert(cindex1 + child_index1 >= 0 && cindex1 + child_index1 < (int64_t)this->child_vector.size());
            assert(this->child_vector[cindex1 + child_index1] == -2);
            assert(this->tree_size_vector[cindex1 + child_index1] == 0);

            this->child_vector[cindex1 + child_index1] = node_index;
            this->tree_size_vector[cindex1 + child_index1] = this->get_tree_size(node_index);
            this->parent_vector[node_index] = node_index_with_dummy_element;
        }

        void replace_dummy_child_with_the_moved_child(int64_t node_index_with_dummy_element, int64_t node_index_with_move_element, uint8_t child_index1, uint8_t child_index2)
        {
            assert(node_index_with_dummy_element >= 0 && node_index_with_dummy_element < (int64_t)this->parent_vector.size());
            assert(node_index_with_move_element >= 0 && node_index_with_move_element < (int64_t)this->parent_vector.size());

            assert(!this->is_leaf(node_index_with_move_element));

            int64_t cindex1 = this->get_child_vector_index(node_index_with_dummy_element);
            int64_t cindex2 = this->get_child_vector_index(node_index_with_move_element);
            assert(cindex1 + child_index1 >= 0 && cindex1 + child_index1 < (int64_t)this->child_vector.size());
            assert(cindex2 + child_index2 >= 0 && cindex2 + child_index2 < (int64_t)this->child_vector.size());

            int64_t child = this->child_vector[cindex2 + child_index2];
            assert(child >= 0 && child < (int64_t)this->parent_vector.size());
            assert(this->child_vector[cindex1 + child_index1] == -2);
            assert(this->tree_size_vector[cindex1 + child_index1] == 0);

            this->child_vector[cindex1 + child_index1] = this->child_vector[cindex2 + child_index2];
            int64_t movedTreeSize = this->tree_size_vector[cindex2 + child_index2];
            this->tree_size_vector[cindex1 + child_index1] = movedTreeSize;

            this->child_vector[cindex2 + child_index2] = -2;
            this->tree_size_vector[cindex2 + child_index2] = -2;

            this->parent_vector[child] = node_index_with_dummy_element;

            int64_t parent1 = this->parent_vector[node_index_with_dummy_element];
            if (parent1 != -1)
            {
                int64_t edge_index = this->get_parent_edge_index(node_index_with_dummy_element);
                int64_t parent_cindex = this->get_child_vector_index(parent1);
                this->tree_size_vector[parent_cindex + edge_index] += movedTreeSize;
            }
            int64_t parent2 = this->parent_vector[node_index_with_move_element];
            if (parent2 != -1)
            {
                int64_t edge_index = this->get_parent_edge_index(node_index_with_move_element);
                int64_t parent_cindex = this->get_child_vector_index(parent2);
                this->tree_size_vector[parent_cindex + edge_index] -= movedTreeSize;
            }
        }

        int64_t create_leaf(int64_t parent)
        {
            int64_t p = this->take_unused_leaf();
            if (p != -1)
            {
                assert(p >= 0 && p < (int64_t)this->parent_vector.size());
                this->parent_vector[p] = parent;
                return p;
            }
            else
            {
                p = this->parent_vector.size();
                this->parent_vector.push_back(parent);
                this->page_size_vector.push_back(0);
                for (int64_t i = 0; i < (this->page_size_limit + 1); i++)
                {
                    this->link_vector.push_back(EMPTY_POM_LINK);
#ifdef DEBUG
                    this->id_vector.push_back(-1);
#endif
                }
                if (this->is_used_value_vector())
                {
                    for (int64_t i = 0; i < (this->page_size_limit + 1); i++)
                    {
                        this->value_vector.push_back(T());
                    }
                }

                return p;
            }
        }
        int64_t create_new_internal_node(int64_t &node_index1)
        {
            int64_t dummy = 0;
            return this->create_new_internal_node(node_index1, dummy);
        }

        int64_t take_unused_internal_node()
        {
            while (this->unused_internal_node_indexes.size() > 0)
            {
                int64_t p = this->unused_internal_node_indexes.top();
                this->unused_internal_node_indexes.pop();
                if (this->is_free_node_index(p))
                {
                    this->page_size_vector[p] = 0;
                    return p;
                }
            }
            return -1;
        }
        int64_t take_unused_leaf()
        {
            while (this->unused_leaf_indexes.size() > 0)
            {
                int64_t p = this->unused_leaf_indexes.top();
                this->unused_leaf_indexes.pop();
                if (this->is_free_node_index(p))
                {
                    this->page_size_vector[p] = 0;
                    return p;
                }
            }
            return -1;
        }

        int64_t create_new_internal_node(int64_t &node_index1, int64_t &node_index2)
        {
            int64_t p = this->take_unused_internal_node();

            if (p == -1)
            {
                int64_t dummyIndex = this->take_unused_leaf();
                if (dummyIndex == -1)
                {
                    dummyIndex = this->create_leaf(-1);
                }
#ifdef DEBUG
                if (__vpom_msg_flag)
                {
                    std::cout << "create_new_internal_node (0), dummyIndex: " << dummyIndex << std::endl;
                }
#endif

                this->swap_leaf(this->internal_node_count, dummyIndex);
                if (node_index1 == this->internal_node_count)
                {
                    node_index1 = dummyIndex;
                }
                if (node_index2 == this->internal_node_count)
                {
                    node_index2 = dummyIndex;
                }
                p = this->internal_node_count;
                this->internal_node_count++;
                assert(p >= 0 && p < (int64_t)this->parent_vector.size());
                this->parent_vector[p] = -1;
                assert(p >= 0 && p < (int64_t)this->page_size_vector.size());
                this->page_size_vector[p] = 0;

                int64_t childBlockLimitSize = this->get_child_block_limit_size();

                for (int64_t i = 0; i < childBlockLimitSize; i++)
                {
                    this->child_vector.push_back(-1);
                    this->tree_size_vector.push_back(-1);
                }
            }
            assert(p >= 0 && p < (int64_t)this->parent_vector.size());
            assert(this->page_size_vector[p] == 0);

            return p;
        }
        void initialize_empty_node_using_dummy_element(int64_t empty_node_index)
        {

            bool b = this->is_leaf(empty_node_index);
            int64_t link_fst_index = this->get_link_vector_index(empty_node_index);
            assert(link_fst_index < (int64_t)this->link_vector.size());

            this->link_vector[link_fst_index] = DUMMY_POM_LINK;
            if (this->is_used_value_vector())
            {
                this->value_vector[link_fst_index] = T();
            }
#ifdef DEBUG
            this->set_id(empty_node_index, 0, -2);
#endif

            if (!b)
            {
                int64_t child_fst_index = this->get_child_vector_index(empty_node_index);
                assert(child_fst_index >= 0 && child_fst_index + 1 < (int64_t)this->child_vector.size());
                this->child_vector[child_fst_index] = -2;
                this->child_vector[child_fst_index + 1] = -2;
                this->tree_size_vector[child_fst_index] = 0;
                this->tree_size_vector[child_fst_index + 1] = 0;
                this->page_size_vector[empty_node_index] = 1;
            }
        }

        void initialize_empty_internal_node(int64_t empty_node_index, POMLink newElement, T newValue, int64_t left_node_index, int64_t right_node_index)
        {
            int64_t link_fst_index = this->get_link_vector_index(empty_node_index);
            int64_t child_fst_index = this->get_child_vector_index(empty_node_index);
            this->link_vector[link_fst_index] = newElement;
            if (this->is_used_value_vector())
            {
                this->value_vector[link_fst_index] = newValue;
            }

#ifdef DEBUG
            this->set_id(empty_node_index, 0, this->tmp_id);
#endif
            assert(child_fst_index >= 0 && child_fst_index + 1 < (int64_t)this->child_vector.size());
            this->child_vector[child_fst_index] = left_node_index;
            this->child_vector[child_fst_index + 1] = right_node_index;

            this->tree_size_vector[child_fst_index] = this->get_tree_size(left_node_index);
            this->tree_size_vector[child_fst_index + 1] = this->get_tree_size(right_node_index);
            assert(left_node_index >= 0 && left_node_index < (int64_t)this->parent_vector.size());
            this->parent_vector[left_node_index] = empty_node_index;
            assert(right_node_index >= 0 && right_node_index < (int64_t)this->parent_vector.size());
            this->parent_vector[right_node_index] = empty_node_index;
            this->page_size_vector[empty_node_index] = 1;

            this->update_links(empty_node_index);
        }
        void right_shift_in_internal_node(int64_t node_index, uint8_t insert_index, POMLink newLink, T newValue, int64_t new_child)
        {
            int64_t size = this->get_page_size(node_index);
            int64_t csize = this->get_children_count(node_index);
            int64_t fst_link_index = this->get_link_vector_index(node_index);

            for (int64_t i = size; i > insert_index; i--)
            {
                this->link_vector[fst_link_index + i] = this->link_vector[fst_link_index + i - 1];
#ifdef DEBUG
                int64_t _tmp_id = this->get_id(node_index, i - 1);
                this->set_id(node_index, i, _tmp_id);
#endif
            }
            if (this->is_used_value_vector())
            {
                for (int64_t i = size; i > insert_index; i--)
                {
                    this->value_vector[fst_link_index + i] = this->value_vector[fst_link_index + i - 1];
                }
                this->value_vector[fst_link_index + insert_index] = newValue;
            }

            this->link_vector[fst_link_index + insert_index] = newLink;

            // int64_t new_size = size+1;
            this->page_size_vector[node_index]++;

            int64_t fst_child_index = this->get_child_vector_index(node_index);

            for (int64_t i = csize - 1; i >= insert_index; i--)
            {
                assert(fst_child_index + i + 1 >= 0 && fst_child_index + i + 1 < (int64_t)this->child_vector.size());
                this->tree_size_vector[fst_child_index + i + 1] = this->tree_size_vector[fst_child_index + i];
                this->child_vector[fst_child_index + i + 1] = this->child_vector[fst_child_index + i];
            }
            assert(fst_child_index + (int64_t)insert_index >= 0 && fst_child_index + (int64_t)insert_index < (int64_t)this->child_vector.size());

            this->child_vector[fst_child_index + insert_index] = new_child;
            int64_t new_child_tree_size = this->get_tree_size(new_child);
            // this->print_info(new_child);
            this->tree_size_vector[fst_child_index + insert_index] = new_child_tree_size;
            assert(new_child >= 0 && new_child < (int64_t)this->parent_vector.size());
            this->parent_vector[new_child] = node_index;

#ifdef DEBUG
            this->set_id(node_index, insert_index, this->tmp_id);
#endif
        }

        void right_shift_in_leaf(int64_t node_index, uint8_t insert_index, POMLink newLink, T newValue)
        {
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            int64_t tmp_parent = this->parent_vector[node_index];
            this->parent_vector[node_index] = -1;
            this->insert_dummy_elements(node_index, insert_index, 1, false, false);
            this->replace_dummy_element_with_the_new_element(node_index, insert_index, newLink, newValue);
            this->parent_vector[node_index] = tmp_parent;
            this->update_recursively_tree_size(node_index);
        }

        void update_recursively_tree_size(int64_t node_index)
        {

            int64_t current_node_index = node_index;
            assert(current_node_index < (int64_t)this->parent_vector.size());
            while (this->parent_vector[current_node_index] != -1)
            {
                int64_t parent = this->parent_vector[current_node_index];
                int64_t size = this->get_tree_size(current_node_index);
                int64_t parent_edge_index = this->get_parent_edge_index(current_node_index);
                int64_t cindex = this->get_child_vector_index(parent);
                this->tree_size_vector[cindex + parent_edge_index] = size;
                current_node_index = parent;
            }
        }
        void add_link(int64_t node_index, POMLink link)
        {
            int64_t size = this->get_page_size(node_index);
            int64_t fst_link_index = this->get_link_vector_index(node_index);
            if (size < this->page_size_limit)
            {
                this->link_vector[fst_link_index + size] = link;
#ifdef DEBUG
                this->set_id(node_index, size, this->id_counter++);
#endif
            }
            else
            {
                throw std::runtime_error("Error: add_link, node_index: " + std::to_string(node_index) + ", size: " + std::to_string(size));
            }
            this->page_size_vector[node_index]++;
        }

        public:

        void defragmentation()
        {
            int64_t i = 0;
            int64_t j = this->internal_node_count - 1;
            while (i < j)
            {
                if (this->is_free_node_index(j))
                {
                    j--;
                }
                else if (this->is_free_node_index(i))
                {
                    this->swap_internal_nodes(i, j);
                    i++;
                }
                else
                {
                    i++;
                }
            }

            int64_t csize = this->get_child_block_limit_size() * (j + 1);
            this->child_vector.resize(csize);
            this->tree_size_vector.resize(csize);
            this->internal_node_count = j + 1;

            i = this->internal_node_count;
            j = this->parent_vector.size() - 1;

            while (i < j)
            {
                if (this->is_free_node_index(j))
                {
                    j--;
                }
                else if (this->is_free_node_index(i))
                {
                    this->swap_leaf(i, j);
                    i++;
                }
                else
                {
                    i++;
                }
            }

            int64_t lsize = this->get_link_block_limit_size() * (j + 1);

            this->link_vector.resize(lsize);
            this->parent_vector.resize(j + 1);
            this->page_size_vector.resize(j + 1);
            if (this->is_used_value_vector())
            {
                this->value_vector.resize(lsize);
                this->value_vector.shrink_to_fit();
            }

            this->link_vector.shrink_to_fit();
            this->parent_vector.shrink_to_fit();
            this->page_size_vector.shrink_to_fit();
            this->child_vector.shrink_to_fit();
            this->tree_size_vector.shrink_to_fit();

            while (this->unused_internal_node_indexes.size() > 0)
            {
                this->unused_internal_node_indexes.pop();
            }
            while (this->unused_leaf_indexes.size() > 0)
            {
                this->unused_leaf_indexes.pop();
            }
        }
        private:

        void swap_internal_nodes(int64_t internal_node_index1, int64_t internal_node_index2)
        {

            int64_t cindex1 = this->get_child_vector_index(internal_node_index1);
            int64_t cindex2 = this->get_child_vector_index(internal_node_index2);
            int64_t lindex1 = this->get_link_vector_index(internal_node_index1);
            int64_t lindex2 = this->get_link_vector_index(internal_node_index2);

            bool b1 = this->is_free_node_index(internal_node_index1);
            bool b2 = this->is_free_node_index(internal_node_index2);
            if (!b1)
            {
                int64_t children_count = this->get_children_count(internal_node_index1);
                for (int64_t i = 0; i < children_count; i++)
                {
                    int64_t child = this->child_vector[cindex1 + i];
                    this->parent_vector[child] = internal_node_index2;
                }
            }
            if (!b2)
            {
                int64_t children_count = this->get_children_count(internal_node_index2);
                for (int64_t i = 0; i < children_count; i++)
                {
                    int64_t child = this->child_vector[cindex2 + i];
                    this->parent_vector[child] = internal_node_index1;
                }
            }

            if (this->parent_vector[internal_node_index1] != -1)
            {
                int64_t parent = this->parent_vector[internal_node_index1];
                int64_t idx = this->get_parent_edge_index(internal_node_index1);
                int64_t cindex = this->get_child_vector_index(parent);
                assert(cindex + idx >= 0 && cindex + idx < (int64_t)this->child_vector.size());
                this->child_vector[cindex + idx] = internal_node_index2;
            }

            if (this->parent_vector[internal_node_index2] != -1)
            {
                int64_t parent = this->parent_vector[internal_node_index2];
                int64_t idx = this->get_parent_edge_index(internal_node_index2);
                int64_t cindex = this->get_child_vector_index(parent);
                assert(cindex + idx >= 0 && cindex + idx < (int64_t)this->child_vector.size());
                this->child_vector[cindex + idx] = internal_node_index1;
            }

            int64_t tmp_parent = this->parent_vector[internal_node_index1];
            this->parent_vector[internal_node_index1] = this->parent_vector[internal_node_index2];
            this->parent_vector[internal_node_index2] = tmp_parent;

            int64_t tmp_page_size = this->page_size_vector[internal_node_index1];
            this->page_size_vector[internal_node_index1] = this->page_size_vector[internal_node_index2];
            this->page_size_vector[internal_node_index2] = tmp_page_size;

            for (int64_t i = 0; i <= this->page_size_limit; i++)
            {
                POMLink tmp_link = this->link_vector[lindex1 + i];
                this->link_vector[lindex1 + i] = this->link_vector[lindex2 + i];
                this->link_vector[lindex2 + i] = tmp_link;

#ifdef DEBUG
                int64_t tmp_id1 = this->get_id(internal_node_index1, i);
                int64_t tmp_id2 = this->get_id(internal_node_index2, i);
                this->set_id(internal_node_index1, i, tmp_id2);
                this->set_id(internal_node_index2, i, tmp_id1);
#endif
            }
            if (this->is_used_value_vector())
            {
                for (int64_t i = 0; i <= this->page_size_limit; i++)
                {
                    T tmp_value = this->value_vector[lindex1 + i];
                    this->value_vector[lindex1 + i] = this->value_vector[lindex2 + i];
                    this->value_vector[lindex2 + i] = tmp_value;
                }
            }

            int64_t child_block_limit_size = this->get_child_block_limit_size();

            for (int64_t i = 0; i < child_block_limit_size; i++)
            {
                int64_t tmp_child = this->child_vector[cindex1 + i];
                this->child_vector[cindex1 + i] = this->child_vector[cindex2 + i];
                this->child_vector[cindex2 + i] = tmp_child;

                int64_t tmp_tree_size = this->tree_size_vector[cindex1 + i];
                this->tree_size_vector[cindex1 + i] = this->tree_size_vector[cindex2 + i];
                this->tree_size_vector[cindex2 + i] = tmp_tree_size;
            }

            if (this->root_index == internal_node_index1)
            {
                this->root_index = internal_node_index2;
            }
            else if (this->root_index == internal_node_index2)
            {
                this->root_index = internal_node_index1;
            }

            if (!is_free_node_index(internal_node_index1))
            {
                this->update_links(internal_node_index1);
            }
            if (!is_free_node_index(internal_node_index2))
            {
                this->update_links(internal_node_index2);
            }
        }

        void swap_leaf(int64_t leaf1, int64_t leaf2)
        {

            assert(leaf1 < (int64_t)this->parent_vector.size());
            if (this->parent_vector[leaf1] != -1)
            {
                int64_t parent = this->parent_vector[leaf1];
                int64_t idx = this->get_parent_edge_index(leaf1);
                int64_t cindex = this->get_child_vector_index(parent);
                assert(cindex + idx >= 0 && cindex + idx < (int64_t)this->child_vector.size());
                this->child_vector[cindex + idx] = leaf2;
            }
            assert(leaf2 < (int64_t)this->parent_vector.size());
            if (this->parent_vector[leaf2] != -1)
            {
                int64_t parent = this->parent_vector[leaf2];
                int64_t idx = this->get_parent_edge_index(leaf2);
                int64_t cindex = this->get_child_vector_index(parent);
                assert(cindex + idx >= 0 && cindex + idx < (int64_t)this->child_vector.size());
                this->child_vector[cindex + idx] = leaf1;
            }

            int64_t tmp_parent = this->parent_vector[leaf1];
            this->parent_vector[leaf1] = this->parent_vector[leaf2];
            this->parent_vector[leaf2] = tmp_parent;

            int64_t tmp_page_size = this->page_size_vector[leaf1];
            this->page_size_vector[leaf1] = this->page_size_vector[leaf2];
            this->page_size_vector[leaf2] = tmp_page_size;

            int64_t fst_link_index1 = this->get_link_vector_index(leaf1);
            int64_t fst_link_index2 = this->get_link_vector_index(leaf2);

            for (int64_t i = 0; i <= this->page_size_limit; i++)
            {
                POMLink tmp_link = this->link_vector[fst_link_index1 + i];
                this->link_vector[fst_link_index1 + i] = this->link_vector[fst_link_index2 + i];
                this->link_vector[fst_link_index2 + i] = tmp_link;

#ifdef DEBUG
                int64_t tmp_id1 = this->get_id(leaf1, i);
                int64_t tmp_id2 = this->get_id(leaf2, i);
                this->set_id(leaf1, i, tmp_id2);
                this->set_id(leaf2, i, tmp_id1);
#endif
            }
            if (this->is_used_value_vector())
            {
                for (int64_t i = 0; i <= this->page_size_limit; i++)
                {
                    T tmp_value = this->value_vector[fst_link_index1 + i];
                    this->value_vector[fst_link_index1 + i] = this->value_vector[fst_link_index2 + i];
                    this->value_vector[fst_link_index2 + i] = tmp_value;
                }
            }

            if (this->root_index == leaf1)
            {
                this->root_index = leaf2;
            }
            else if (this->root_index == leaf2)
            {
                this->root_index = leaf1;
            }

            if (!is_free_node_index(leaf1))
            {
                this->update_links(leaf1);
            }

            if (!is_free_node_index(leaf2))
            {
                this->update_links(leaf2);
            }
        }

        void update_links(int64_t node_index)
        {
            assert(node_index >= 0);
            if (this->linkedVPomTree != nullptr)
            {
                int64_t lindex = this->get_link_vector_index(node_index);
                int64_t size = this->get_page_size(node_index);
                for (int64_t i = 0; i < size; i++)
                {
                    POMLink link = this->link_vector[lindex + i];

                    if (link.first >= (int64_t)this->linkedVPomTree->parent_vector.size())
                    {
                        std::cout << "#NODE, " << node_index << "/" << link.first << "/" << this->linkedVPomTree->parent_vector.size() << std::endl;
                    }
                    assert(link.first < (int64_t)this->linkedVPomTree->parent_vector.size());
                    if (link.first >= 0)
                    {
                        this->linkedVPomTree->set_link(link, POMLink(node_index, i));
                    }
                }
            }
        }
        void update_link(POMLink position)
        {
            // assert(position.second < this->page_size_vector[position.first]);

            if (this->linkedVPomTree != nullptr)
            {
                int64_t lindex = this->get_link_vector_index(position.first);
                POMLink link = this->link_vector[lindex + position.second];
                if (link.first >= 0)
                {
                    this->linkedVPomTree->set_link(link, position);
                }
            }
        }

        /* to_string */

        std::string to_string([[maybe_unused]]int64_t node_index, [[maybe_unused]]uint8_t element_index) const
        {
            // int64_t element_global_index = this->get_link_vector_index(node_index) + element_index;

            std::string s;
            s.push_back('(');
#ifdef DEBUG
            s += std::to_string(this->get_id(node_index, element_index));
#else
            s += "*";
#endif

            s.push_back(')');
            return s;
        }
        std::string to_string(uint64_t node_index) const
        {
            std::string s;
            s.clear();
            uint64_t page_size = this->get_page_size(node_index);

            for (uint64_t i = 0; i < page_size; i++)
            {
                s += this->to_string(node_index, i);
            }

            return s;
        }

        std::string get_tree_string(uint64_t node_index, std::vector<std::string> &output) const
        {

            uint64_t height = this->get_height(node_index);
            while (output.size() < height)
            {
                output.push_back("");
            }
            if (this->is_leaf(node_index))
            {
                std::string str = this->to_string(node_index);
                assert(output.size() > height - 1);
                output[height - 1] += str;

                return str;
            }
            else
            {

                std::vector<std::string> child_strs;
                uint64_t page_size = this->get_page_size(node_index);

                for (int64_t i = 0; i <= (int64_t)page_size; i++)
                {
                    uint64_t child_index = this->get_child_index(node_index, i);
                    uint64_t page_size = this->get_page_size(node_index);

                    std::string output_str = this->get_tree_string(child_index, output);
                    child_strs.push_back(output_str);

                    if (i < (int64_t)page_size)
                    {
                        uint64_t m = (this->to_string(node_index, i)).size();
                        for (uint64_t h = 0; h < height - 1; h++)
                        {
                            for (uint64_t j = 0; j < m; j++)
                            {
                                assert(output.size() > h);
                                output[h].push_back(' ');
                            }
                        }
                    }
                }
                std::string str = this->to_string(node_index, child_strs);
                assert(output.size() > height - 1);
                output[height - 1] += str;

                return str;
            }
        }
        std::string to_string(uint64_t node_index, std::vector<std::string> &children_strs) const
        {
            std::string s;
            uint64_t page_size = this->get_page_size(node_index);

            for (uint64_t i = 0; i < page_size; i++)
            {
                if (i < children_strs.size())
                {
                    for (uint64_t j = 0; j < children_strs[i].size(); j++)
                    {
                        s.push_back('-');
                    }
                    s += this->to_string(node_index, i);
                }
            }
            if (children_strs.size() > page_size)
            {
                for (uint64_t j = 0; j < children_strs[page_size].size(); j++)
                {
                    s.push_back('-');
                }
            }

            return s;
        }

    public:
        void print() const
        {
            if (this->root_index == -1)
                return;
            std::vector<std::string> r;
            // assert(this->root != nullptr);
            this->get_tree_string(this->root_index, r);

            // this->root->get_tree_string(r);
            int64_t height = this->get_height(this->root_index);

            for (int64_t h = height; h >= 1; h--)
            {
                assert((int64_t)r.size() > h - 1);
                std::cout << r[h - 1] << std::endl;
            }
            std::cout << std::endl;
        }
        void print_info() const
        {
            std::cout << "###PRINT_INFO / " << std::endl;
            for (uint64_t i = 0; i < this->parent_vector.size(); i++)
            {
                if (this->page_size_vector[i] != 255)
                {
                    this->print_info(i);
                }
            }
        }

        void print_info(int64_t node_index) const
        {
            assert(node_index >= 0 && node_index < (int64_t)this->parent_vector.size());
            uint64_t height = this->get_height(node_index);
            uint64_t page_size = this->get_page_size(node_index);

            uint64_t lindex = this->get_link_vector_index(node_index);
            int64_t parent = this->parent_vector[node_index];

            std::cout << "Node: " << node_index << ", height = " << height << ", leaf?: " << (this->is_leaf(node_index) ? "yes" : "no") << ", page_size: " << page_size << ", parent: " << parent << ", TreeSize: " << this->get_tree_size(node_index) << std::endl;

            std::vector<int64_t> link1_vec, link2_vec, link3_vec;

            for (int64_t i = 0; i <= this->page_size_limit; i++)
            {
                assert(lindex + i < this->link_vector.size());
                int64_t lvalue = this->link_vector[lindex + i].first;
                int64_t rvalue = this->link_vector[lindex + i].second;

                link1_vec.push_back(lvalue);
                link2_vec.push_back(rvalue);
#ifdef DEBUG
                if (this->linkedVPomTree != nullptr)
                {
                    if (lvalue >= 0)
                    {
                        int64_t _tmp_id = this->linkedVPomTree->get_id(lvalue, rvalue);
                        link3_vec.push_back(_tmp_id);
                    }
                    else
                    {
                        link3_vec.push_back(-1);
                    }
                }
#endif
            }
            stool::Printer::print(" Link     ", link1_vec);
            stool::Printer::print(" Link(Sub)", link2_vec);
            stool::Printer::print(" Link(Ptr)", link3_vec);
            /*
            if (this->is_used_value_vector())
            {
                std::vector<T> value_vec;

                for (int64_t i = 0; i <= this->page_size_limit; i++)
                {
                    value_vec.push_back(this->value_vector[lindex + i]);
                }
                stool::Printer::print(" Value", value_vec);
            }
            */

            if (!this->is_leaf(node_index))
            {
                uint64_t pindex = this->get_child_vector_index(node_index);

                std::vector<int64_t> rank_vec, child_vec;
                for (int64_t i = 0; i <= this->page_size_limit; i++)
                {
                    assert(pindex + i < this->tree_size_vector.size());
                    int64_t tvalue = this->tree_size_vector[pindex + i];
                    rank_vec.push_back(tvalue);
                    assert(pindex + i < this->child_vector.size());
                    int64_t cvalue = this->child_vector[pindex + i];

                    child_vec.push_back(cvalue);
                }
                stool::Printer::print(" TreeSize", rank_vec);
                stool::Printer::print(" ChildIndex", child_vec);
            }
#ifdef DEBUG
            std::vector<int64_t> id_vec;
            for (int64_t i = 0; i <= this->page_size_limit; i++)
            {
                int64_t ivalue = this->get_id(node_index, i);

                id_vec.push_back(ivalue);
            }
            stool::Printer::print(" ID", id_vec);
#endif
            std::cout << "-------" << std::endl;
        }
        /* Verification */

        bool verify() const
        {
            std::vector<int64_t> node_indexes = this->get_postorder_nodes();
            for (auto it : node_indexes)
            {
                this->verify(it);
            }
            return true;
        }

        bool verify(int64_t node_index) const
        {
            int64_t size = this->get_page_size(node_index);
            int64_t lindex = this->get_link_vector_index(node_index);
            assert(node_index < (int64_t)this->parent_vector.size());
            int64_t parent = this->parent_vector[node_index];
#ifdef DEBUG
            for (int64_t i = size; i < this->page_size_limit + 1; i++)
            {
                int64_t _tmp_id = this->get_id(node_index, i);
                if (_tmp_id != -1)
                {
                    throw std::logic_error(this->name() + ", Error: verify(ID vector), node_index: " + std::to_string(node_index) + ", element_index: " + std::to_string(i) + ", ID = " + std::to_string(_tmp_id) + " == -1");
                }
            }
            for (int64_t i = 0; i < size; i++)
            {
                int64_t _tmp_id = this->get_id(node_index, i);

                if (_tmp_id < 0)
                {
                    throw std::logic_error(this->name() + ", Error: verify(ID vector), node_index: " + std::to_string(node_index) + ", element_index: " + std::to_string(i) + ", ID = " + std::to_string(_tmp_id) + " >= 0");
                }
            }

#endif

            if (this->linkedVPomTree != nullptr)
            {
                for (int64_t i = 0; i < size; i++)
                {
                    assert(this->link_vector[lindex + i].first != -2);
                    assert(this->link_vector[lindex + i].first < (int64_t)this->linkedVPomTree->parent_vector.size());

                    if (this->link_vector[lindex + i].first >= 0)
                    {
                        bool b = this->linkedVPomTree->is_free_node_index(this->link_vector[lindex + i].first);
                        if (b)
                        {
                            throw std::logic_error(this->name() + ", Error: verify(Pointer), node_index: " + std::to_string(node_index) + ", element_index: " + std::to_string(i) + ", Link = " + std::to_string(this->link_vector[lindex + i].first));
                        }
                        assert(this->link_vector[lindex + i].second < this->linkedVPomTree->page_size_vector[this->link_vector[lindex + i].first]);
                    }
                }
            }

            if (parent != -1)
            {
                int64_t edge_index = this->get_edge_index_to_child(parent, node_index);
                if (edge_index == -1)
                {
                    throw std::logic_error(this->name() + ", Error: verify(Parent Edge index Error), node_index: " + std::to_string(node_index));
                }
            }

            if (this->is_leaf(node_index))
            {
            }
            else
            {
                int64_t csize = this->get_children_count(node_index);
                for (int64_t i = 0; i < csize; i++)
                {
                    int64_t child_index = this->get_child_index(node_index, i);
                    int64_t p1 = this->get_tree_size(node_index, i);
                    int64_t p2 = this->get_tree_size(child_index);
                    bool b = p1 == p2;
                    if (!b)
                    {
                        std::string errorMsg = this->name() + ", Error: verify(Tree Size Error), node_index = " + std::to_string(node_index) + ", child_edge_index = " + std::to_string(i) + ", " + std::to_string(p1) + "!=" + std::to_string(p2);
                        throw std::logic_error(errorMsg);
                    }
                }
                int64_t blockSize = this->get_child_block_limit_size();

                for (int64_t i = csize; i < blockSize; i++)
                {
                    int64_t p1 = this->get_tree_size(node_index, i);
                    if (p1 != -1)
                    {
                        std::cout << "Tree Size Error: node: " << node_index << ", element_index: " << i << ", tree_size: " << p1 << std::endl;
                        throw std::logic_error(this->name() + ", Error: verify(Tree Size Error2)");
                    }
                }
            }
            return true;
        }
        void print_memory() const {
            uint64_t parent_vector_bytes = (this->parent_vector.capacity() * sizeof(int64_t));
            uint64_t child_vector_bytes = (this->child_vector.capacity() * sizeof(int64_t));
            uint64_t tree_size_bytes = (this->tree_size_vector.capacity() * sizeof(int64_t));
            uint64_t page_size_bytes = (this->page_size_vector.capacity() * sizeof(uint8_t));
            uint64_t link_bytes = (this->link_vector.capacity() * sizeof(POMLink));
            uint64_t value_bytes = (this->value_vector.capacity() * sizeof(T));
            uint64_t total_bytes = parent_vector_bytes + child_vector_bytes + tree_size_bytes + page_size_bytes + link_bytes + value_bytes;

            std::cout << "========== MEMORY of VectorizedPomTree ========" << std::endl;
            std::cout << "parent_vector   : " << parent_vector_bytes << " bytes (size/capacity) = (" << this->parent_vector.size() << "/" << this->parent_vector.capacity() << ")" << std::endl;
            std::cout << "child_vector    : " << child_vector_bytes << " bytes (size/capacity) = (" << this->child_vector.size() << "/" << this->child_vector.capacity() << ")" << std::endl;
            std::cout << "tree_size_vector: " << tree_size_bytes << " bytes (size/capacity) = (" << this->tree_size_vector.size() << "/" << this->tree_size_vector.capacity() << ")" << std::endl;
            std::cout << "page_size_vector: " << page_size_bytes << " bytes (size/capacity) = (" << this->page_size_vector.size() << "/" << this->page_size_vector.capacity() << ")" << std::endl;
            std::cout << "link_vector     : " << link_bytes << " bytes (size/capacity) = (" << this->link_vector.size() << "/" << this->link_vector.capacity() << ")" << std::endl;
            std::cout << "value_vector    : " << value_bytes << " bytes (size/capacity) = (" << this->value_vector.size() << "/" << this->value_vector.capacity() << ")" << std::endl;
            std::cout << "Total           : " << total_bytes << " bytes (" << (total_bytes / this->size()) << " bytes per element)" << std::endl;            
            std::cout << "Total           : " << total_bytes << " bytes (" << (total_bytes / this->link_vector.capacity()) << " bytes per capacity element)" << std::endl;            
            std::cout << "Density         :  " << ((double) this->size()) / ((double)this->link_vector.size())  << std::endl;
            std::cout << "===============================================" << std::endl;

        }
    };
    }
}
#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <list>
#include <stack>
#include <cassert>
#include <unordered_map>
#include "./pom_tree_node.hpp"
#include "./pom_tree_node_debug.hpp"
#include "./pom_tree_node_update.hpp"
namespace stool
{
    namespace old_implementations
    {
    class POMTree
    {
    public:
        int64_t page_size_limit;
        POMTreeNode *root = nullptr;

        void initialize(int64_t _page_size_limit)
        {
            this->page_size_limit = _page_size_limit;
            this->root = new POMTreeNode();
            this->root->initialize(nullptr);
        }
        int64_t size() const
        {
            return this->root->tree_size;
        }
        void clear(){
            while(this->size() > 0){
                this->erase(0);
            }
        }
        std::pair<POMTreeNode *, int64_t> search_insertion_leaf_index(int64_t rank) const
        {
            POMTreeNode *current_node = this->root;
            while (!current_node->is_bottom())
            {
                int64_t idx = current_node->lower_bound(rank);

                if (idx > 0)
                {
                    int64_t sub_rank = current_node->get_sub_rank(idx - 1);
                    rank -= (sub_rank + 1);
                }
                current_node = current_node->get_child(idx);
            }
            assert(current_node != nullptr);

            return std::pair<POMTreeNode *, int64_t>(current_node, rank);
        }
        std::pair<POMTreeNode *, int64_t> get_subnode_index(int64_t rank) const
        {

            POMTreeNode *current_node = this->root;
            assert(current_node != nullptr);

            while (!current_node->is_bottom())
            {
                int64_t idx = current_node->lower_bound(rank);
                if (idx != current_node->get_page_size())
                {
                    int64_t sub_rank = current_node->get_sub_rank(idx);

                    if (sub_rank == rank)
                    {
                        rank = idx;
                        break;
                        //return std::pair<POMTreeNode *, int64_t>(current_node, idx);
                    }
                }
                if (idx > 0)
                {
                    int64_t sub_rank = current_node->get_sub_rank(idx - 1);
                    rank -= (sub_rank + 1);
                }
                current_node = current_node->get_child(idx);
            }
            assert(current_node != nullptr);

            return std::pair<POMTreeNode *, int64_t>(current_node, rank);
        }
        void udpate_subnode(int64_t rank, POMTreeNode *link, uint8_t link_subindex)
        {
            if (rank < 0 || rank >= this->size())
            {
                throw std::runtime_error("Error: udpate_subnode");
            }

            std::pair<POMTreeNode *, int64_t> nodeIndex = this->get_subnode_index(rank);
            POMTreeNode *node = nodeIndex.first;
            POMTreeNodeItem item(link, link_subindex, node->items[nodeIndex.second].child);
            node->items[nodeIndex.second] = item;
        }

        bool insert(uint64_t rank)
        {
            assert(this->root != nullptr);

            std::pair<POMTreeNode *, int64_t> leaf_index = this->search_insertion_leaf_index(rank);
            POMTreeNodeUpdate::insert_at_position(*leaf_index.first, leaf_index.second);
            this->balance_tree_for_isnertion(leaf_index.first);



            return true;
        }
        bool erase(int64_t rank)
        {
            assert(rank < this->size());
            [[maybe_unused]] uint64_t minLeafCount = this->page_size_limit / 2;
            auto subindex = this->get_subnode_index(rank);
            POMTreeNode *node = subindex.first;
            int64_t pos = subindex.second;
            POMTreeNode *target_node = nullptr;

            if (node->is_bottom())
            {
                assert(pos < node->get_page_size());
                POMTreeNodeUpdate::erase_subnode_of_leaf(*node, pos);
                target_node = node;
            }
            else
            {
                auto left_subindex = this->get_subnode_index(rank - 1);
                auto right_subindex = this->get_subnode_index(rank + 1);

                POMTreeNode *leftLeaf = left_subindex.first;
                POMTreeNode *rightLeaf = right_subindex.first;
                uint64_t leftLeafCount = leftLeaf->get_page_size();
                uint64_t rightLeafCount = rightLeaf->get_page_size();

                if (leftLeafCount > rightLeafCount)
                {
                    assert(leftLeafCount > minLeafCount);
                    POMTreeNodeUpdate::delete_and_right_shift(*node, pos);
                    target_node = leftLeaf;
                }
                else
                {

                    assert(rightLeafCount >= minLeafCount);
                    POMTreeNodeUpdate::delete_and_left_shift(*node, pos);
                    target_node = rightLeaf;
                }
            }
            this->root = POMTreeNodeUpdate::balance_tree_for_deletion(*target_node, this->page_size_limit, *this->root);

            
            // this->print();
            // assert(this->root->check_parent_edge_index());
            // assert(this->check_invariant());

            // assert(this->root->check_tree_size());

            return true;
        }

        bool check_invariant(bool strict_link_check) const
        {
            auto nodes = this->get_postorder_nodes();
            std::vector<int64_t> pvec;
            
            /*
            for(uint64_t i = 0; i<nodes.size();i++){
                pvec.push_back(nodes[i]->compute_node_rank());
            }

            std::cout << "Size: " << this->get_size() << std::endl;
                stool::Printer::print(pvec);

            for(uint64_t i = 1; i<nodes.size();i++){
                assert(pvec[i-1] < pvec[i]);
            }
            */


            for(auto &it : nodes){
                POMTreeNodeDebug::check_invariant(*it, this->page_size_limit, strict_link_check);
            }
            return true;
        }
        void print() const
        {
            std::vector<POMTreeNode*> nodes = this->get_postorder_nodes();
            /*
            for(auto &node : nodes){
                std::cout << "rank: " << node->compute_last_subnode_rank() << ", tree size: " << node->tree_size << std::endl;
            }
            */

            std::vector<std::string> r;
            assert(this->root != nullptr);
            POMTreeNodeDebug::get_tree_string(*this->root, r);

            // this->root->get_tree_string(r);
            int64_t height = this->root->get_height();
            for (int64_t h = height; h >= 1; h--)
            {
                assert((int64_t)r.size() > h - 1);
                std::cout << r[h - 1] << std::endl;
            }
            std::cout << std::endl;
        }
        std::vector<std::pair<POMTreeNode *, uint8_t>> get_all_links() const
        {
            std::vector<std::pair<POMTreeNode *, uint8_t>> output;
            this->root->get_all_links(output);
            return output;
        }
        std::vector<POMTreeNode*> get_preorder_nodes() const {
            std::vector<POMTreeNode*> r;
            this->root->get_preorder_nodes(r);
            return r;
        }
        std::vector<POMTreeNode*> get_postorder_nodes() const {
            std::vector<POMTreeNode*> r;
            this->root->get_postorder_nodes(r);
            return r;
        }

    private:
        void balance_tree_for_isnertion(POMTreeNode *affected_leaf)
        {
            POMTreeNode* current_node = affected_leaf;
            while (current_node != nullptr)
            {

                if (current_node->get_page_size() > this->page_size_limit)
                {
                    POMTreeNodeUpdate::split(*current_node);
                }

                current_node = current_node->parent;
                if (current_node != nullptr && current_node->is_root())
                {
                    this->root = current_node;
                }
            }
        }
    };
    }
}

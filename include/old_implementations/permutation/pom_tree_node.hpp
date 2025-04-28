#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <list>
#include <stack>
#include <cassert>
#include "../../time_debug.hpp"
namespace stool
{
        namespace old_implementations
    {
    class POMTreeNode;

    class POMTreeNodeItem
    {
    public:
        POMTreeNode *child;
        POMTreeNode *link;
        uint8_t link_subindex;
        POMTreeNodeItem() : child(nullptr), link(nullptr), link_subindex(UINT8_MAX)
        {
        }
        POMTreeNodeItem(POMTreeNode *_link, uint8_t _link_subindex, POMTreeNode *_child) : child(_child), link(_link), link_subindex(_link_subindex)
        {
        }
    };

    class POMTreeNode
    {
    public:
        int64_t tree_size;
        POMTreeNode *parent;
        std::vector<POMTreeNodeItem> items;
        POMTreeNode *rightmost_child = nullptr;

        POMTreeNode(){

        }

        void initialize(POMTreeNode *_parent)
        {
            this->parent = _parent;
            this->tree_size = 0;
        }

        int64_t lower_bound(int64_t subrank) const
        {
            int64_t current_subrank = 0;
            for (uint64_t i = 0; i < this->items.size(); i++)
            {
                assert(this->items[i].child != nullptr);
                current_subrank += this->items[i].child->tree_size;
                if (current_subrank >= subrank)
                {
                    return i;
                }
                current_subrank++;
            }
            return this->items.size();
        }
        int64_t count_subnodes_in_subtree() const
        {
            uint64_t tree_size = this->items.size();
            if (!this->is_bottom())
            {
                for (uint64_t i = 0; i <= this->items.size(); i++)
                {
                    POMTreeNode *node = this->get_child(i);
                    assert(node != nullptr);
                    tree_size += node->tree_size;
                }
            }
            return tree_size;
        }
        POMTreeNode *get_child(uint64_t pos) const
        {
            if (pos <= this->items.size())
            {
                return pos < this->items.size() ? this->items[pos].child : this->rightmost_child;
            }
            else
            {
                throw std::runtime_error("GetChild Error");
            }
            return nullptr;
        }
        void get_all_links(std::vector<std::pair<POMTreeNode *, uint8_t>> &output) const
        {
            for (auto &it : this->items)
            {
                if (it.child != nullptr)
                {
                    it.child->get_all_links(output);
                }
                output.push_back(std::pair<POMTreeNode *, uint8_t>(it.link, it.link_subindex));
            }
            if (this->rightmost_child != nullptr)
            {
                this->rightmost_child->get_all_links(output);
            }
        }
        int64_t get_parent_edge_index() const
        {
            if (!this->is_root())
            {
                for (uint64_t i = 0; i <= parent->items.size(); i++)
                {
                    POMTreeNode *node = parent->get_child(i);
                    if (node == this)
                    {
                        return i;
                    }
                }
            }
            else
            {
                return -1;
            }
            std::cout << "Error: " << this->get_height() << "/" << this->items.size() << std::endl;
            throw std::logic_error("Error: get_parent_edge_index");
        }
        POMTreeNode *get_leftmost_leaf()
        {
            if (this->is_bottom())
            {
                return this;
            }
            else
            {
                return this->items[0].child->get_leftmost_leaf();
            }
        }
        POMTreeNode *get_rightmost_leaf()
        {
            if (this->is_bottom())
            {
                return this;
            }
            else
            {
                return this->rightmost_child->get_rightmost_leaf();
            }
        }

        bool is_root() const
        {
            return this->parent == nullptr;
        }


        int64_t compute_last_subnode_rank() const
        {
            return this->compute_subnode_rank(this->items.size() - 1);
        }
        bool is_bottom() const
        {
            return this->rightmost_child == nullptr;
        }
        
        int64_t get_sub_rank(int64_t subindex) const
        {
            if (this->is_bottom())
            {
                return subindex;
            }
            else
            {
                int64_t xrank = subindex;
                for (int64_t i = 0; i <= subindex; i++)
                {
                    assert(i < (int64_t)this->items.size());
                    xrank += this->items[i].child->tree_size;
                }
                return xrank;
            }
        }
        

        int64_t compute_subnode_rank(int64_t subindex) const
        {

            uint64_t r = this->get_sub_rank(subindex);
            
            POMTreeNode* currentNode = this->parent;
            uint64_t idx = this->get_parent_edge_index();
            while(currentNode != nullptr){
                if(idx > 0){
                    r += currentNode->get_sub_rank(idx-1) + 1;
                }
                idx = currentNode->get_parent_edge_index();
                currentNode = currentNode->parent;
            }
            return r;

        }

        int64_t get_page_size() const
        {
            return this->items.size();
        }
        bool is_balanced_height() const
        {
            int64_t tHeight = this->get_height() - 1;
            for (int64_t i = 0; i <= this->get_page_size(); i++)
            {
                POMTreeNode *child = this->get_child(i);
                int64_t h = child == nullptr ? 0 : child->get_height();
                if (tHeight != h)
                {
                    return false;
                }
            }
            return true;
        }

        std::string to_string(uint64_t subnode_index) const
        {
            std::string s;
            s.push_back('(');
            if (this->items[subnode_index].link != nullptr)
            {
                uint64_t rank = this->items[subnode_index].link->compute_last_subnode_rank();
                s += std::to_string(rank);
            }
            else
            {
                s += "*";
            }
            s += "[";

            s += std::to_string(this->items[subnode_index].link_subindex);
            s.push_back(']');

            s.push_back(')');
            return s;
        }
        std::string to_string() const
        {
            std::string s;
            s.clear();
            for (uint64_t i = 0; i < this->items.size(); i++)
            {
                s += this->to_string(i);
            }
            // s.push_back('[');
            // s += std::to_string(this->tree_size);
            // s.push_back(']');

            return s;
        }
        int64_t get_height() const
        {

            if (this->is_bottom())
            {
                return 1;
            }
            else
            {
                return this->rightmost_child->get_height() + 1;
            }
        }
        void get_preorder_nodes(std::vector<POMTreeNode *> &output) const
        {
            output.push_back(const_cast<POMTreeNode *>(this));
            if (!this->is_bottom())
            {
                for (int64_t i = 0; i <= this->get_page_size(); i++)
                {
                    POMTreeNode *child = this->get_child(i);
                    output.push_back(child);
                    child->get_preorder_nodes(output);
                }
            }
        }
        void get_postorder_nodes(std::vector<POMTreeNode *> &output) const
        {
            if (!this->is_bottom())
            {
                for (int64_t i = 0; i <= this->get_page_size(); i++)
                {
                    POMTreeNode *child = this->get_child(i);
                    child->get_preorder_nodes(output);
                }
            }
            output.push_back(const_cast<POMTreeNode *>(this));
        }
    };
    }
}

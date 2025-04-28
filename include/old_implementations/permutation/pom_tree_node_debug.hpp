#pragma once
#include "./pom_tree_node.hpp"

namespace stool
{
    namespace old_implementations
    {
    class POMTreeNodeDebug
    {
    public:
        static std::string get_tree_string(const POMTreeNode &node, std::vector<std::string> &output)
        {
            uint64_t height = node.get_height();
            while (output.size() < height)
            {
                output.push_back("");
            }
            if (node.is_bottom())
            {

                std::string str = node.to_string();
                assert(output.size() > height - 1);
                output[height - 1] += str;

                return str;
            }
            else
            {

                std::vector<std::string> child_strs;

                for (int64_t i = 0; i <= node.get_page_size(); i++)
                {
                    POMTreeNode *tmp = node.get_child(i);
                    assert(tmp != nullptr);

                    std::string output_str = POMTreeNodeDebug::get_tree_string(*tmp, output);
                    child_strs.push_back(output_str);

                    if (i < node.get_page_size())
                    {
                        assert(i < (int64_t)node.items.size());
                        uint64_t m = (node.to_string(i)).size();
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
                std::string str = POMTreeNodeDebug::to_string(node, child_strs);
                assert(output.size() > height - 1);
                output[height - 1] += str;

                return str;
            }
        }

        

        static std::string to_string(const POMTreeNode &node, std::vector<std::string> &children_strs)
        {
            std::string s;

            for (uint64_t i = 0; i < node.items.size(); i++)
            {
                if (i < children_strs.size())
                {
                    for (uint64_t j = 0; j < children_strs[i].size(); j++)
                    {
                        s.push_back('-');
                    }
                    s += node.to_string(i);
                }
            }
            if (children_strs.size() > node.items.size())
            {
                uint64_t last = node.items.size();
                for (uint64_t j = 0; j < children_strs[last].size(); j++)
                {
                    s.push_back('-');
                }
            }
            // s.push_back('[');
            // s += std::to_string(this->tree_size);
            // s.push_back(']');

            return s;
        }

        static bool check_tree_size(const POMTreeNode &node)
        {
            int64_t m = node.count_subnodes_in_subtree();
            if (m != node.tree_size)
            {
                throw std::logic_error("Error: check_tree_size");
                return false;
            }
            return true;
        }
        /*
        static bool check_parent_edge_index(const POMTreeNode &node)
        {
            if (!node.is_bottom())
            {
                bool b = true;

                for (int64_t i = 0; i <= node.get_page_size(); i++)
                {
                    POMTreeNode *child = node.get_child(i);
                    int64_t x = child->get_parent_edge_index();
                    if (x == -1)
                    {
                        return false;
                    }
                    else
                    {
                        b = b && POMTreeNodeDebug::check_parent_edge_index(*child);
                    }
                }
                return b;
            }
            else
            {
                return true;
            }
        }
        */

        static bool check_parent_edge_index(const POMTreeNode &node)
        {
            if (!node.is_root())
            {
                int64_t p = node.get_parent_edge_index();
                POMTreeNode *parent = node.parent;
                POMTreeNode *child = parent->get_child(p);
                if (child != &node)
                {
                    throw std::logic_error("Error: check_parent_edge_index");
                }
            }
            return true;
        }
        static bool check_balanced_height(const POMTreeNode &node)
        {
            if (!node.is_balanced_height())
            {
                throw std::logic_error("Error: check_balanced_height");
                return false;
            }
            return true;
        }

        static bool check_page_size(const POMTreeNode &node, int64_t max_page_size)
        {
            bool b3 = (node.get_page_size() <= max_page_size) && ((max_page_size / 2) <= node.get_page_size());
            if (!node.is_root() && !b3)
            {
                std::cout << "Error: " << node.get_page_size() << "/" << max_page_size << std::endl;
                throw std::logic_error("Error: check_page_size");
            }
            return true;
        }

        static bool check_link(const POMTreeNode &node, bool strict_link_check)
        {
            for (auto it : node.items)
            {
                if (it.link == nullptr)
                {
                    if(strict_link_check){
                    throw std::logic_error("Error1: check_link");
                    }
                }
                else
                {
                    if (it.link->items[it.link_subindex].link != &node)
                    {
                        throw std::logic_error("Error2: check_link");
                    }
                }
            }
            return true;
        }

        static bool check_invariant(const POMTreeNode &node, uint64_t max_page_size, bool strict_link_check)
        {
            check_parent_edge_index(node);
            check_balanced_height(node);
            check_page_size(node, max_page_size);
            check_tree_size(node);

                check_link(node, strict_link_check);


            return true;
        }
    };
    }
}

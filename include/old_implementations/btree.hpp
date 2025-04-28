#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <list>
#include <stack>
#include <cassert>

#include <unordered_map>
#include "./btree_node.hpp"

namespace stool
{
    namespace old_implementations
    {
    template <typename K, typename V>
    class BTree
    {

    public:
        uint64_t page_size;
        BTreeNode<K, V> *root = nullptr;

        void initialize(uint64_t _page_size)
        {
            this->page_size = _page_size;
            this->root = new BTreeNode<K, V>();
            this->root->initialize();
        }
        
        bool check_invariant() const
        {
            return this->root->check_invariant(true, this->page_size);
        }

        bool insert(K key, V value)
        {
            assert(this->root != nullptr);

            using SearchResult = std::pair<BTreeNode<K, V> *, uint64_t>;
            std::vector<SearchResult> search_path;
            this->root->search(key, search_path);
            assert(search_path.size() > 0);
            SearchResult top = search_path[search_path.size() - 1];

            top.first->insert_at_position(key, value, top.second);
            search_path.pop_back();

            BTreeNode<K, V> *currentNode = top.first;

            while (currentNode->items.size() > this->page_size)
            {
                if (search_path.size() > 0)
                {
                    top = search_path[search_path.size() - 1];
                    top.first->split(top.second);
                    currentNode = top.first;
                    search_path.pop_back();
                }
                else
                {
                    this->root = this->root->root_split();
                    currentNode = this->root;
                }
            }

            return true;
        }
        bool erase(K key)
        {
            using SearchResult = std::pair<BTreeNode<K, V> *, uint64_t>;
            std::vector<SearchResult> search_path;
            bool hit = this->root->search(key, search_path);
            if (!hit)
                return false;

            assert(search_path.size() > 0);
            SearchResult top = search_path[search_path.size() - 1];
            BTreeNode<K, V> *top_node = top.first;
            uint64_t pos = top.second;

            uint64_t minLeafCount = this->page_size / 2;


            if (top_node->is_bottom())
            {
                top_node->items.erase(top_node->items.begin() + pos);
            }
            else
            {

                std::vector<SearchResult> leftPath = top_node->get_left_rightmost_leaf_path(pos);
                std::vector<SearchResult> rightPath = top_node->get_right_leftmost_leaf_path(pos);
                BTreeNode<K, V> *leftLeafParent = leftPath[leftPath.size() - 1].first;
                BTreeNode<K, V> *rightLeafParent = rightPath[rightPath.size() - 1].first;
                uint64_t leftLeafCount = leftLeafParent->get_page_size();
                uint64_t rightLeafCount = rightLeafParent->get_page_size();
                search_path.pop_back();

                if (leftLeafCount > rightLeafCount)
                {
                    assert(leftLeafCount > minLeafCount);
                    top_node->delete_and_right_shift(pos, leftLeafParent);
                    for (auto &it : leftPath)
                    {
                        search_path.push_back(it);
                    }
                }
                else
                {
                    assert(rightLeafCount >= minLeafCount);
                    top_node->delete_and_left_shift(pos, rightLeafParent);
                    for (auto &it : rightPath)
                    {
                        search_path.push_back(it);
                    }
                }
            }

            while (search_path.size() > 0)
            {
                top = search_path[search_path.size() - 1];
                top_node = top.first;
                search_path.pop_back();
                if (top_node->get_page_size() >= minLeafCount)
                {
                    break;
                }
                else
                {
                    if (search_path.size() > 0)
                    {
                        BTreeNode<K, V> *parent = search_path[search_path.size() - 1].first;
                        uint64_t edgePos = search_path[search_path.size() - 1].second;

                        BTreeNode<K, V> *leftNode = parent->get_child(edgePos - 1);
                        BTreeNode<K, V> *rightNode = parent->get_child(edgePos + 1);

                        if (leftNode != nullptr)
                        {

                            uint64_t leftSize = leftNode->get_page_size();

                            if (leftSize > minLeafCount)
                            {
                                parent->shift_from_left_to_right(edgePos - 1);
                            }
                            else
                            {
                                parent->merge(edgePos - 1);
                            }
                        }
                        else if (rightNode != nullptr)
                        {

                            uint64_t rightSize = rightNode->get_page_size();
                            
                            if (rightSize > minLeafCount)
                            {
                                parent->shift_from_right_to_left(edgePos);
                            }
                            else
                            {
                                parent->merge(edgePos);
                            }
                        }
                        else
                        {
                            throw std::logic_error("Erase Error!");
                        }
                    }
                    else
                    {
                        assert(top_node == this->root);
                        if (this->root->get_page_size() == 0 && this->root->rightmost_child != nullptr)
                        {
                            BTreeNode<K, V> *nextRoot = this->root->rightmost_child;
                            delete this->root;
                            this->root = nextRoot;
                        }
                    }
                }
            }

            return true;
        }

        std::vector<std::pair<K, V>> get_key_value_pairs() const
        {
            std::vector<std::pair<K, V>> r;
            this->root->get_key_value_pairs(r);
            return r;
        }

        void print() const
        {
            std::vector<std::string> r;
            this->root->get_tree_string(r);
            uint64_t height = this->root->get_height();
            for (int64_t h = height; h >= 1; h--)
            {
                std::cout << r[h - 1] << std::endl;
            }
            std::cout << std::endl;
        }
    };
    }
}

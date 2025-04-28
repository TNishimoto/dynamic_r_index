#pragma once
#include <vector>
#include <cstdint>
#include <iostream>
#include <list>
#include <stack>

#include <unordered_map>

namespace stool
{
        namespace old_implementations
    {
    template <typename K, typename V>
    class BTreeNode;

    template <typename K, typename V>
    class BTreeNodeItem
    {
    public:
        K key;
        V value;
        BTreeNode<K, V> *child;

        BTreeNodeItem() : child(nullptr)
        {
        }

        BTreeNodeItem(K _key, V _value) : key(_key), value(_value), child(nullptr)
        {
        }
    };

    template <typename K, typename V>
    class BTreeNode
    {
    public:
        std::vector<BTreeNodeItem<K, V>> items;
        BTreeNode<K, V> *rightmost_child = nullptr;

        void initialize()
        {
        }

        int lower_bound(K key) const
        {
            for (uint64_t i = 0; i < this->items.size(); i++)
            {
                if (this->items[i].key >= key)
                {
                    return i;
                }
            }
            return this->items.size();
        }
        void clear()
        {
        }
        
        bool split(uint64_t node_index)
        {
            assert(node_index <= this->items.size());
            BTreeNodeItem<K, V> newBtreeNodeItem = BTreeNodeItem<K, V>();

            if (node_index < this->items.size())
            {
                assert(this->items[node_index].child != nullptr);
                this->items[node_index].child->move_processing(newBtreeNodeItem);
                this->items.insert(this->items.begin() + node_index, newBtreeNodeItem);
            }
            else
            {
                assert(this->rightmost_child != nullptr);
                this->rightmost_child->move_processing(newBtreeNodeItem);
                this->items.push_back(newBtreeNodeItem);
            }
            return true;
        }
        uint64_t get_page_size() const
        {
            return this->items.size();
        }
        bool is_bottom() const
        {
            bool b = true;
            for (auto &it : this->items)
            {
                b = b && (it.child == nullptr);
            }
            return b && (this->rightmost_child == nullptr);
        }
        std::vector<BTreeNode<K, V> *> get_children() const
        {
            std::vector<BTreeNode<K, V> *> r;
            for (auto &it : this->items)
            {
                if (it.child != nullptr)
                {
                    r.push_back(it.child);
                }
            }
            if (this->rightmost_child != nullptr)
            {
                r.push_back(this->rightmost_child);
            }
            return r;
        }
        uint64_t get_height() const
        {

            if (this->is_bottom())
            {
                return 1;
            }
            else
            {
                uint64_t child_max_height = 1;
                std::vector<BTreeNode<K, V> *> tmp = this->get_children();
                for (BTreeNode<K, V> *it : tmp)
                {
                    child_max_height = std::max(child_max_height, it->get_height());
                }
                return child_max_height + 1;
            }
        }
        bool is_balanced_height() const
        {
            if (this->is_bottom())
            {
                return true;
            }
            else
            {
                bool b = true;
                uint64_t tHeight = this->get_height() - 1;
                std::vector<BTreeNode<K, V> *> tmp = this->get_children();
                for (BTreeNode<K, V> *it : tmp)
                {
                    b = b && (it->get_height() == tHeight);
                }
                return b;
            }
        }
        K get_smallest_key() const
        {
            if (this->is_bottom())
            {
                return this->items[0].key;
            }
            else
            {
                assert(this->items[0].child != nullptr);
                return this->items[0].child->get_smallest_key();
            }
        }
        K get_largest_key() const
        {
            if (this->is_bottom())
            {
                return this->items[this->items.size() - 1].key;
            }
            else
            {
                assert(this->rightmost_child != nullptr);
                return this->rightmost_child->get_largest_key();
            }
        }

        bool check_invariant(bool is_root, uint64_t max_page_size) const
        {
            if (!this->is_bottom())
            {
                for (uint64_t i = 0; i < this->items.size(); i++)
                {
                    BTreeNode<K, V> *leftTree = this->get_child(i);
                    BTreeNode<K, V> *rightTree = this->get_child(i+1);

                    K key = this->items[i].key;
                    if (key <= leftTree->get_largest_key())
                    {
                        std::cout << "Smallest Key Error! " << key << std::endl;

                        return false;
                    }

                    if (key >= rightTree->get_smallest_key())
                    {
                        std::cout << "Largest Key Error! " << key << std::endl;

                        return false;
                    }
                }
            }

            bool b1 = this->is_balanced_height();
            if (!b1)
            {
                std::cout << "Not Balanced!" << std::endl;
            }
            bool b2 = true;

            std::vector<BTreeNode<K, V> *> tmp = this->get_children();
            for (BTreeNode<K, V> *it : tmp)
            {
                bool bX = it->check_invariant(false, max_page_size);
                b2 = b2 && bX;
            }

            if (is_root)
            {
                return b1 && b2;
            }
            else
            {
                bool b3 = (this->get_page_size() <= max_page_size) && ((max_page_size / 2) <= this->get_page_size());
                if (!b3)
                {
                    std::cout << "Count Error!" << std::endl;
                }

                return b1 && b2 && b3;
            }
        }

        void insert_at_position(K key, V value, uint64_t pos)
        {
            assert(pos <= this->items.size());
            if (pos < this->items.size())
            {
                if (this->items[pos].key == key)
                {
                    this->items[pos].key = key;
                    this->items[pos].value = value;
                }
                else
                {
                    this->items.insert(this->items.begin() + pos, BTreeNodeItem<K, V>(key, value));
                }
            }
            else
            {
                this->items.push_back(BTreeNodeItem<K, V>(key, value));
            }
        }

        void get_key_value_pairs(std::vector<std::pair<K, V>> &output) const
        {
            for (auto &it : this->items)
            {
                if (it.child != nullptr)
                {
                    it.child->get_key_value_pairs(output);
                }
                output.push_back(std::pair<K, V>(it.key, it.value));
            }
            if (this->rightmost_child != nullptr)
            {
                this->rightmost_child->get_key_value_pairs(output);
            }
        }

        using SearchResult = std::pair<BTreeNode<K, V> *, uint64_t>;
        bool search(K key, std::vector<SearchResult> &output_path)
        {
            bool hit = false;
            output_path.clear();
            uint64_t idx1 = this->lower_bound(key);

            output_path.push_back(SearchResult(this, idx1));

            while (true)
            {
                SearchResult topPair = output_path[output_path.size() - 1];
                uint64_t current_idx = topPair.second;
                BTreeNode<K, V> *current_node = topPair.first;

                if (current_idx < current_node->items.size())
                {
                    BTreeNodeItem<K, V> &item = current_node->items[current_idx];
                    if (item.key == key)
                    {
                        hit = true;
                        break;
                    }
                    else
                    {
                        if (item.child == nullptr)
                        {
                            break;
                        }
                        else
                        {
                            uint64_t idx2 = item.child->lower_bound(key);
                            output_path.push_back(SearchResult(item.child, idx2));
                        }
                    }
                }
                else
                {
                    BTreeNode<K, V> *child = current_node->rightmost_child;
                    if (child == nullptr)
                    {
                        break;
                    }
                    else
                    {
                        uint64_t idx2 = child->lower_bound(key);
                        output_path.push_back(SearchResult(child, idx2));
                    }
                }
            }
            return hit;
        }
        std::vector<SearchResult> get_left_rightmost_leaf_path(uint64_t pos)
        {
            std::vector<SearchResult> r;
            r.push_back(SearchResult(this, pos));

            if (this->is_bottom())
            {
                return r;
            }

            while (true)
            {
                SearchResult topPair = r[r.size() - 1];
                uint64_t current_idx = topPair.second;
                BTreeNode<K, V> *current_node = topPair.first;
                BTreeNode<K, V> *next_node = current_node->get_child(current_idx);
                assert(next_node != nullptr);
                if (next_node->rightmost_child != nullptr)
                {
                    r.push_back(SearchResult(next_node, next_node->items.size()));
                }
                else
                {
                    r.push_back(SearchResult(next_node, next_node->items.size() - 1));
                    break;
                }
            }
            return r;
        }

        std::vector<SearchResult> get_right_leftmost_leaf_path(uint64_t pos)
        {
            std::vector<SearchResult> r;

            if (this->is_bottom())
            {
                r.push_back(SearchResult(this, pos));
                return r;
            }
            r.push_back(SearchResult(this, pos + 1));

            while (true)
            {
                SearchResult topPair = r[r.size() - 1];
                uint64_t current_idx = topPair.second;
                BTreeNode<K, V> *current_node = topPair.first;
                BTreeNode<K, V> *next_node = current_node->get_child(current_idx);
                assert(next_node != nullptr && next_node->items.size() > 0);
                r.push_back(SearchResult(next_node, 0));

                if (next_node->items[0].child == nullptr)
                {
                    break;
                }
            }
            return r;
        }
        void delete_and_right_shift(uint64_t pos, BTreeNode<K, V> *leftLeaf)
        {
            uint64_t leafPos = leftLeaf->items.size() - 1;
            BTreeNodeItem<K, V> &movedItem = leftLeaf->items[leafPos];
            this->items[pos].key = movedItem.key;
            this->items[pos].value = movedItem.value;
            leftLeaf->items.pop_back();
        }
        void delete_and_left_shift(uint64_t pos, BTreeNode<K, V> *rightLeaf)
        {
            BTreeNodeItem<K, V> &movedItem = rightLeaf->items[0];
            this->items[pos].key = movedItem.key;
            this->items[pos].value = movedItem.value;
            rightLeaf->items.erase(rightLeaf->items.begin() + 0);
        }
        void shift_from_left_to_right(uint64_t pos)
        {
            BTreeNode<K, V> *leftTree = this->items[pos].child;
            BTreeNode<K, V> *rightTree = pos + 1 < this->items.size() ? this->items[pos + 1].child : this->rightmost_child;
            BTreeNode<K, V> *tmp_left = leftTree->rightmost_child;

            BTreeNodeItem<K, V> rightNewNodeItem;

            rightNewNodeItem.key = this->items[pos].key;
            rightNewNodeItem.value = this->items[pos].value;
            rightNewNodeItem.child = tmp_left;
            rightTree->items.insert(rightTree->items.begin() + 0, rightNewNodeItem);

            BTreeNodeItem<K, V> &movedItem1 = leftTree->items[leftTree->items.size() - 1];
            this->items[pos].key = movedItem1.key;
            this->items[pos].value = movedItem1.value;
            this->items[pos].child = leftTree;

            leftTree->rightmost_child = movedItem1.child;
            leftTree->items.pop_back();
        }
        void shift_from_right_to_left(uint64_t pos)
        {
            BTreeNode<K, V> *leftTree = this->get_child(pos);
            BTreeNode<K, V> *rightTree = this->get_child(pos + 1);
            assert(rightTree != nullptr);

            BTreeNodeItem<K, V> leftNewNodeItem;

            leftNewNodeItem.key = this->items[pos].key;
            leftNewNodeItem.value = this->items[pos].value;
            leftNewNodeItem.child = leftTree->rightmost_child;
            leftTree->items.push_back(leftNewNodeItem);

            BTreeNodeItem<K, V> &movedItem1 = rightTree->items[0];
            leftTree->rightmost_child = movedItem1.child;

            this->items[pos].key = movedItem1.key;
            this->items[pos].value = movedItem1.value;
            this->items[pos].child = leftTree;

            rightTree->items.erase(rightTree->items.begin() + 0);
        }

        void merge(uint64_t pos)
        {
            BTreeNode<K, V> *leftTree = this->get_child(pos);
            BTreeNode<K, V> *rightTree = this->get_child(pos+1);

            BTreeNodeItem<K, V> newNodeItem;
            newNodeItem.key = this->items[pos].key;
            newNodeItem.value = this->items[pos].value;
            newNodeItem.child = leftTree->rightmost_child;
            leftTree->items.push_back(newNodeItem);

            for (auto &it : rightTree->items)
            {
                leftTree->items.push_back(it);
            }
            leftTree->rightmost_child = rightTree->rightmost_child;
            if (pos + 1 < this->items.size())
            {
                this->items[pos + 1].child = leftTree;
            }
            else
            {
                this->rightmost_child = leftTree;
            }
            this->items.erase(this->items.begin() + pos);

            delete rightTree;
        }
        BTreeNode<K, V> *get_child(uint64_t pos) const
        {
            if (pos <= this->items.size())
            {
                return pos < this->items.size() ? this->items[pos].child : this->rightmost_child;
            }
            else
            {
                return nullptr;
            }
        }


        std::string to_string() const
        {
            std::string s;
            for (uint64_t i = 0; i < this->items.size(); i++)
            {
                s.push_back('(');
                s += std::to_string(this->items[i].key);
                s.push_back(')');
            }
            return s;
        }

        std::string get_tree_string(std::vector<std::string> &output) const
        {
            uint64_t height = this->get_height();
            while (output.size() < height)
            {
                output.push_back("");
            }
            if (this->is_bottom())
            {

                std::string str = this->to_string();
                output[height - 1] += str;

                return str;
            }
            else
            {

                std::vector<std::string> child_strs;
                std::vector<BTreeNode<K, V> *> tmp = this->get_children();
                assert(tmp.size() > 0);
                for (uint64_t i = 0; i < tmp.size(); i++)
                {
                    child_strs.push_back(tmp[i]->get_tree_string(output));

                    if (i + 1 < tmp.size())
                    {
                        uint64_t m = 2 + std::to_string(this->items[i].key).size();
                        for (uint64_t h = 0; h < height - 1; h++)
                        {
                            for (uint64_t j = 0; j < m; j++)
                            {
                                output[h].push_back(' ');
                            }
                        }
                    }
                }
                std::string str = this->to_string(child_strs);
                output[height - 1] += str;
                return str;
            }
        }

        std::string to_string(std::vector<std::string> &children_strs) const
        {
            std::string s;
            for (uint64_t i = 0; i < this->items.size(); i++)
            {
                if (i < children_strs.size())
                {

                    for (uint64_t j = 0; j < children_strs[i].size(); j++)
                    {
                        s.push_back('-');
                    }
                    s.push_back('(');
                    s += std::to_string(this->items[i].key);
                    s.push_back(')');
                }
            }
            if (children_strs.size() > this->items.size())
            {
                uint64_t last = this->items.size();
                for (uint64_t j = 0; j < children_strs[last].size(); j++)
                {
                    s.push_back('-');
                }
            }
            return s;
        }

        BTreeNode<K, V>* root_split()
        {
            BTreeNode<K, V>* root = this;
            assert(root != nullptr);
            BTreeNode<K, V> *newRoot = new BTreeNode<K, V>();
            newRoot->initialize();
            BTreeNodeItem<K, V> newBtreeNodeItem = BTreeNodeItem<K, V>();
            root->move_processing(newBtreeNodeItem);

            newRoot->items.push_back(newBtreeNodeItem);
            newRoot->rightmost_child = root;
            return newRoot;
        }

    private:
        bool split_last_node(uint page_max_size)
        {
            if (this->items.size() < page_max_size)
            {
                uint64_t center_index = this->rightmost_child->items.size() / 2;
                BTreeNodeItem<K, V> &center_item = this->rightmost_child->items[center_index];
                this->items.push_back(BTreeNodeItem<K, V>(center_item.key, center_item.value));
            }
            else
            {
                return false;
            }
        }
        void move_processing(BTreeNodeItem<K, V> &newBtreeNodeItem)
        {
            BTreeNode<K, V> *newNode = new BTreeNode<K, V>();
            uint64_t center_index = this->items.size() / 2;

            newBtreeNodeItem.key = this->items[center_index].key;
            newBtreeNodeItem.value = this->items[center_index].value;
            newBtreeNodeItem.child = newNode;

            for (uint64_t i = 0; i < center_index; i++)
            {
                newNode->items.push_back(this->items[i]);
            }
            newNode->rightmost_child = this->items[center_index].child;

            std::vector<BTreeNodeItem<K, V>> newItems;
            for (uint64_t i = center_index + 1; i < this->items.size(); i++)
            {
                newItems.push_back(this->items[i]);
            }
            this->items.swap(newItems);
        }
    };
    }
}
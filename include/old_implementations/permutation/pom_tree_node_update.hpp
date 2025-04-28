#pragma once
#include "./pom_tree_node.hpp"

namespace stool
{
        namespace old_implementations
    {
    class POMTreeNodeUpdate
    {
    public:
        static void insert_at_position(POMTreeNode &node, int64_t pos)
        {

            for (int64_t i = pos; i < node.get_page_size(); i++)
            {
                POMTreeNodeItem &item = node.items[i];
                if (item.link != nullptr)
                {
                    assert(item.link->items[item.link_subindex].link == &node);
                    item.link->items[item.link_subindex].link_subindex++;
                }
            }

            assert(pos <= (int64_t)node.items.size());
            if (pos < (int64_t)node.items.size())
            {
                node.items.insert(node.items.begin() + pos, POMTreeNodeItem());
            }
            else
            {
                node.items.push_back(POMTreeNodeItem());
            }

            POMTreeNode *current_node = &node;
            while (current_node != nullptr)
            {
                current_node->tree_size++;
                current_node = current_node->parent;
            }
        }
        static void update_tree_size(POMTreeNode &node)
        {
            node.tree_size = node.count_subnodes_in_subtree();
        }
        static void update_informaiton_of_linked_node(POMTreeNode &node, uint64_t pos, bool item_is_removed)
        {
            if (node.items[pos].link != nullptr)
            {
                POMTreeNode *linkedNode = node.items[pos].link;
                POMTreeNodeItem &linkedItem = linkedNode->items[node.items[pos].link_subindex];
                if (item_is_removed)
                {
                    linkedItem.link = nullptr;
                    linkedItem.link_subindex = UINT8_MAX;
                }else{
                    linkedItem.link = &node;
                    linkedItem.link_subindex = pos;
                }
            }
        }

        static void delete_and_right_shift(POMTreeNode &node, uint64_t pos)
        {
            POMTreeNode *leftLeaf = node.get_child(pos)->get_rightmost_leaf();
            uint64_t leafPos = leftLeaf->get_page_size() - 1;
            POMTreeNodeItem &movedItem = leftLeaf->items[leafPos];

            update_informaiton_of_linked_node(node, pos, true);
            node.items[pos].link = movedItem.link;
            node.items[pos].link_subindex = movedItem.link_subindex;
            update_informaiton_of_linked_node(node, pos, false);

            leftLeaf->items.pop_back();

            POMTreeNode *currentNode = leftLeaf;
            while (currentNode != nullptr)
            {
                currentNode->tree_size--;
                currentNode = currentNode->parent;
            }
        }
        static void delete_and_left_shift(POMTreeNode &node, uint64_t pos)
        {
            POMTreeNode *rightLeaf = node.get_child(pos + 1)->get_leftmost_leaf();
            POMTreeNodeItem &movedItem = rightLeaf->items[0];

            update_informaiton_of_linked_node(node, pos, true);
            node.items[pos].link = movedItem.link;
            node.items[pos].link_subindex = movedItem.link_subindex;
            update_informaiton_of_linked_node(node, pos, false);

            rightLeaf->items.erase(rightLeaf->items.begin() + 0);
            POMTreeNodeUpdate::update_node_item_information(*rightLeaf);

            POMTreeNode *currentNode = rightLeaf;
            while (currentNode != nullptr)
            {
                currentNode->tree_size--;
                currentNode = currentNode->parent;
            }
        }
        static void shift_from_left_to_right(POMTreeNode &node, uint64_t pos)
        {
            POMTreeNode *leftTree = node.get_child(pos);
            POMTreeNode *rightTree = node.get_child(pos + 1);
            POMTreeNode *tmp_left = leftTree->rightmost_child;

            POMTreeNodeItem rightNewNodeItem;

            rightNewNodeItem.link = node.items[pos].link;
            rightNewNodeItem.link_subindex = node.items[pos].link_subindex;
            rightNewNodeItem.child = tmp_left;
            if (rightNewNodeItem.child != nullptr)
            {
                rightNewNodeItem.child->parent = rightTree;
            }

            rightTree->items.insert(rightTree->items.begin() + 0, rightNewNodeItem);
            POMTreeNodeUpdate::update_node_item_information(*rightTree);

            POMTreeNodeItem &movedItem1 = leftTree->items[leftTree->items.size() - 1];
            node.items[pos].link = movedItem1.link;
            node.items[pos].link_subindex = movedItem1.link_subindex;
            node.items[pos].child = leftTree;
            POMTreeNodeUpdate::update_informaiton_of_linked_node(node, pos, false);

            leftTree->rightmost_child = movedItem1.child;
            if (leftTree->rightmost_child != nullptr)
            {
                leftTree->rightmost_child->parent = leftTree;
            }
            leftTree->items.pop_back();

            POMTreeNodeUpdate::update_tree_size(*leftTree);
            POMTreeNodeUpdate::update_tree_size(*rightTree);
        }
        static void shift_from_right_to_left(POMTreeNode &node, uint64_t pos)
        {

            POMTreeNode *leftTree = node.get_child(pos);
            POMTreeNode *rightTree = node.get_child(pos + 1);
            //POMTreeNode *tmp_right = rightTree->items[0].child;

            POMTreeNodeItem leftNewNodeItem;

            leftNewNodeItem.link = node.items[pos].link;
            leftNewNodeItem.link_subindex = node.items[pos].link_subindex;
            leftNewNodeItem.child = leftTree->rightmost_child;
            if (leftNewNodeItem.child != nullptr)
            {
                leftNewNodeItem.child->parent = leftTree;
            }

            leftTree->items.push_back(leftNewNodeItem);
            update_informaiton_of_linked_node(*leftTree, leftTree->get_page_size()-1, false);

            POMTreeNodeItem &movedItem1 = rightTree->items[0];
            leftTree->rightmost_child = movedItem1.child;
            if (leftTree->rightmost_child != nullptr)
            {
                leftTree->rightmost_child->parent = leftTree;
            }

            node.items[pos].link = movedItem1.link;
            node.items[pos].link_subindex = movedItem1.link_subindex;
            node.items[pos].child = leftTree;
            update_informaiton_of_linked_node(node, pos, false);

            rightTree->items.erase(rightTree->items.begin() + 0);
            POMTreeNodeUpdate::update_node_item_information(*rightTree);

            POMTreeNodeUpdate::update_tree_size(*leftTree);
            POMTreeNodeUpdate::update_tree_size(*rightTree);
        }
        static void merge(POMTreeNode &node, uint64_t pos)
        {
            POMTreeNode *leftTree = node.get_child(pos);
            POMTreeNode *rightTree = node.get_child(pos + 1);

            POMTreeNodeItem newNodeItem;
            newNodeItem.link = node.items[pos].link;
            newNodeItem.link_subindex = node.items[pos].link_subindex;
            newNodeItem.child = leftTree->rightmost_child;
            if (newNodeItem.child != nullptr)
            {
                newNodeItem.child->parent = leftTree;
            }
            leftTree->items.push_back(newNodeItem);

            for (auto &it : rightTree->items)
            {
                leftTree->items.push_back(it);
                if (it.child != nullptr)
                {
                    it.child->parent = leftTree;
                }
            }
            leftTree->rightmost_child = rightTree->rightmost_child;
            if (leftTree->rightmost_child != nullptr)
            {
                leftTree->rightmost_child->parent = leftTree;
            }
            POMTreeNodeUpdate::update_node_item_information(*leftTree);

            if (pos + 1 < node.items.size())
            {
                node.items[pos + 1].child = leftTree;
            }
            else
            {
                node.rightmost_child = leftTree;
            }
            node.items.erase(node.items.begin() + pos);
            POMTreeNodeUpdate::update_node_item_information(node);


            POMTreeNodeUpdate::update_tree_size(*leftTree);
            POMTreeNodeUpdate::update_tree_size(node);

            delete rightTree;
        }
        static POMTreeNode *split(POMTreeNode &node)
        {
            if (node.is_root())
            {

                // POMTreeNode *root = this;

                POMTreeNode *newRoot = new POMTreeNode();

                newRoot->initialize(nullptr);

                node.parent = newRoot;
                newRoot->rightmost_child = &node;

                POMTreeNode *newLeftnode = new POMTreeNode();
                newLeftnode->initialize(newRoot);

                POMTreeNodeItem dummyItem(nullptr, UINT8_MAX, newLeftnode);
                newRoot->items.push_back(dummyItem);
                POMTreeNodeUpdate::left_move_processing(node);
                POMTreeNodeUpdate::update_node_item_information(*newRoot);
                POMTreeNodeUpdate::update_tree_size(*newRoot);

                return newRoot;
            }
            else
            {

                // node.parent;
                assert(node.parent != nullptr);
                POMTreeNode *newLeftNode = new POMTreeNode();
                newLeftNode->initialize(node.parent);

                POMTreeNodeItem dummyItem(nullptr, UINT8_MAX, newLeftNode);
                int64_t edgeIndex = node.get_parent_edge_index();
                assert(edgeIndex != -1);
                node.parent->items.insert(node.parent->items.begin() + edgeIndex, dummyItem);
                POMTreeNodeUpdate::left_move_processing(node);

                POMTreeNodeUpdate::update_node_item_information(*node.parent);
                POMTreeNodeUpdate::update_tree_size(*node.parent);

                return node.parent;
            }
        }
        static void update_node_item_information(POMTreeNode &node)
        {
            for (uint64_t i = 0; i < node.items.size(); i++)
            {
                if (node.items[i].child != nullptr)
                {
                    node.items[i].child->parent = &node;
                }
                if (node.items[i].link != nullptr)
                {
                    POMTreeNodeItem &item = node.items[i].link->items[node.items[i].link_subindex];
                    item.link = &node;
                    item.link_subindex = i;
                }
            }
            if (node.rightmost_child != nullptr)
            {
                node.rightmost_child->parent = &node;
            }
        }
        static void left_move_processing(POMTreeNode &node)
        {
            int64_t edge_idx = node.get_parent_edge_index();
            uint64_t center_index = node.items.size() / 2;
            assert(edge_idx > 0 && edge_idx <= (int64_t)node.parent->items.size());

            POMTreeNodeItem &dummyNode = node.parent->items[edge_idx - 1];
            POMTreeNode *leftNode = node.parent->get_child(edge_idx - 1);
            assert(leftNode != nullptr);

            dummyNode.link = node.items[center_index].link;
            dummyNode.link_subindex = node.items[center_index].link_subindex;
            dummyNode.child = leftNode;

            for (uint64_t i = 0; i < center_index; i++)
            {
                leftNode->items.push_back(node.items[i]);
            }
            leftNode->rightmost_child = node.items[center_index].child;

            std::vector<POMTreeNodeItem> newItems;
            for (uint64_t i = center_index + 1; i < node.items.size(); i++)
            {
                newItems.push_back(node.items[i]);
            }
            node.items.swap(newItems);

            POMTreeNodeUpdate::update_node_item_information(*leftNode);
            POMTreeNodeUpdate::update_node_item_information(node);

            POMTreeNodeUpdate::update_tree_size(node);
            POMTreeNodeUpdate::update_tree_size(*leftNode);
        }
        static void erase_subnode_of_leaf(POMTreeNode &leaf, int64_t pos)
        {
            POMTreeNodeItem &item = leaf.items[pos];
            if (item.link != nullptr)
            {
                POMTreeNodeItem &linkedItem = item.link->items[item.link_subindex];
                linkedItem.link = nullptr;
                linkedItem.link_subindex = UINT8_MAX;
            }
            leaf.items.erase(leaf.items.begin() + pos);
            POMTreeNodeUpdate::update_node_item_information(leaf);

            POMTreeNode *currentNode = &leaf;
            while (currentNode != nullptr)
            {
                currentNode->tree_size--;
                currentNode = currentNode->parent;
            }
        }
        static POMTreeNode *balance_tree_for_deletion(POMTreeNode &node, int64_t page_size_limit, POMTreeNode &root)
        {
            int64_t minLeafCount = page_size_limit / 2;

            POMTreeNode *target_node = &node;
            POMTreeNode *current_root = &root;

            while (target_node != nullptr)
            {
                if (target_node->get_page_size() >= minLeafCount)
                {
                    break;
                }
                else
                {
                    if (!target_node->is_root())
                    {
                        POMTreeNode *parent = target_node->parent;
                        int64_t edgePos = target_node->get_parent_edge_index();

                        if (edgePos > 0)
                        {
                            POMTreeNode *leftNode = parent->get_child(edgePos - 1);
                            int64_t leftSize = leftNode->get_page_size();

                            if (leftSize > minLeafCount)
                            {
                                POMTreeNodeUpdate::shift_from_left_to_right(*parent, edgePos - 1);
                            }
                            else
                            {
                                POMTreeNodeUpdate::merge(*parent, edgePos - 1);
                            }
                        }
                        else if (edgePos < parent->get_page_size())
                        {
                            POMTreeNode *rightNode = parent->get_child(edgePos + 1);

                            int64_t rightSize = rightNode->get_page_size();

                            if (rightSize > minLeafCount)
                            {
                                POMTreeNodeUpdate::shift_from_right_to_left(*parent, edgePos);
                            }
                            else
                            {
                                POMTreeNodeUpdate::merge(*parent, edgePos);
                            }
                        }
                        else
                        {
                            throw std::logic_error("Erase Error!");
                        }
                        target_node = parent;
                    }
                    else
                    {
                        if (current_root->get_page_size() == 0 && current_root->rightmost_child != nullptr)
                        {
                            POMTreeNode *nextRoot = current_root->rightmost_child;
                            nextRoot->parent = nullptr;
                            delete current_root;
                            current_root = nextRoot;
                        }
                        target_node = nullptr;
                    }
                }
            }
            return current_root;
        }
    };
    }

}

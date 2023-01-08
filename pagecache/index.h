//
// Created by Kirill Gavrilov on 03.01.2023.
//

#include "pagecache.h"

#

#ifndef QDB_INDEX_H
#define QDB_INDEX_H

template<typename T>
class Node {
private:
    T *keys;
    int *keys_count;
    int *children_count;
    std::byte *store_data;
    int *children;
public:
    PageId page_id;
    int t;

    Node(PageCache &page_cache, int t) : Node(page_cache, page_cache.create_page({0}), t) {
        set_parent({-1, -1});
        set_right({-1, -1});
    };

    Node(PageCache &page_cache, PageId page_id, int t) : page_id(page_id), t(t) {
        auto data = page_cache.read_page(page_id);
        store_data = data;
        int currentOffset = 4 * sizeof(int);
        keys_count = (int *) (data + currentOffset);
        currentOffset += sizeof(int);
        children_count = (int *) (data + currentOffset);
        currentOffset += sizeof(int);
        keys = (T *) (data + currentOffset);
        currentOffset += sizeof(T) * (2 * t);
        children = (int *) (data + currentOffset);
    }

    Node<T> get_child_at(PageCache &page_cache, int index) {
        return Node(page_cache, get_child_at(index), t);
    }

    PageId get_child_at(int index) {
        return {children[index * 2], children[index * 2 + 1]};
    }

    Node<T> get_parent(PageCache &page_cache) {
        return Node(page_cache, get_parent(), t);
    }

    bool get_is_leaf() {
        return get_children_count() == 0;
    }

    void set_parent(PageId id) {
        *((int *) store_data) = id.fileId.id;
        *((int *) (store_data + sizeof(int))) = id.id;
    }

    PageId get_parent() {
        return {*((int *) store_data), *((int *) (store_data + sizeof(int)))};
    }

    Node<T> get_right(PageCache &page_cache) {
        return Node(page_cache, get_right(), t);
    }

    PageId get_right() {
        return {*((int *) (store_data + 2 * sizeof(int))), *((int *) (store_data + 3 * sizeof(int)))};
    }

    void set_right(PageId id) {
        *((int *) (store_data + 2 * sizeof(int))) = id.fileId.id;
        *((int *) (store_data + 3 * sizeof(int))) = id.id;
    }

    int get_keys_count() {
        return *keys_count;
    }

    T get_key_at(int index) {
        return keys[index];
    }

    void set_key_at(int index, T key) {
        keys[index] = key;
    }

    void set_keys_count(int count) {
        (*keys_count) = count;
    }

    void set_children_at(int index, PageId id) {
        children[index * 2] = id.fileId.id;
        children[index * 2 + 1] = id.id;
    }

    void set_children_count(int count) {
        (*children_count) = count;
    }

    int get_children_count() {
        return *children_count;
    }

    bool has_key(T key) {
        for (int i = 0; i < get_keys_count(); i++) {
            if (key == get_key_at(i)) {
                return true;
            }
        }
        return false;
    }
};

template<typename T>
class BTree {
private:
    PageCache &page_cache;
    int t;
    Node<T> shadow_root;

    Node<T> find_suitable_leaf(T key) {
        auto current = get_root();
        while (!current.get_is_leaf()) {
            for (int i = 0; i <= current.get_keys_count(); i++) {
                if (i == current.get_keys_count() || key < current.get_key_at(i)) {
                    current = current.get_child_at(page_cache, i);
                    break;
                }
            }
        }
        return current;
    }

    Node<T> get_root() {
        return Node<T>(page_cache, root_page_id, t);
    }

    void split_node(Node<T> node) {
        auto node_page_id = node.page_id;
        auto fresh_node = Node<T>(page_cache, t);
        auto fresh_node_page_id = fresh_node.page_id;
        node = Node<T>(page_cache, node_page_id, t);
        bool is_leaf = node.get_is_leaf();

        if ((node.get_right()).id != -1) {
            fresh_node.set_right(node.get_right());
        }

        node.set_right(fresh_node.page_id);
        auto split_key = node.get_key_at(t);

        fresh_node.set_keys_count(t - 1);
        fresh_node.set_children_count(t);
        node.set_keys_count(t);
        node.set_children_count(t + 1);

        for (int i = 0; i < fresh_node.get_keys_count(); i++) {
            fresh_node.set_key_at(i, node.get_key_at(i + t + 1));
        }

        page_cache.write(fresh_node.page_id);
        page_cache.write(node.page_id);
        if (is_leaf) {
            fresh_node.set_keys_count(fresh_node.get_keys_count() + 1);
            for (int i = fresh_node.get_keys_count() - 1; i > 0; i--) {
                fresh_node.set_key_at(i, fresh_node.get_key_at(i - 1));
            }
            fresh_node.set_key_at(0, split_key);
            fresh_node.set_children_count(0);
            node.set_children_count(0);
        } else {
            for (int i = 0; i <= fresh_node.get_keys_count(); i++) {
                fresh_node.set_children_at(i, node.get_child_at(i + t + 1));
                page_cache.write(fresh_node.page_id);
                auto child = fresh_node.get_child_at(page_cache, i);
                fresh_node = Node<T>(page_cache, fresh_node_page_id, t);
                node = Node<T>(page_cache, node_page_id, t);
                child.set_parent(fresh_node.page_id);
                page_cache.write(child.page_id);

            }
        }
        page_cache.write(fresh_node.page_id);
        page_cache.write(node.page_id);
        auto root = get_root();
        if (node_page_id.id == root.page_id.id) {
            root = Node<T>(page_cache, t);
            fresh_node = Node<T>(page_cache, fresh_node_page_id, t);
            node = Node<T>(page_cache, node_page_id, t);
            root_page_id = root.page_id;
            root.set_keys_count(1);
            root.set_key_at(0, split_key);
            root.set_children_count(2);
            root.set_children_at(0, node.page_id);
            root.set_children_at(1, fresh_node.page_id);
            node.set_parent(root.page_id);
            fresh_node.set_parent(root.page_id);
            page_cache.write(root.page_id);
            page_cache.write(fresh_node.page_id);
            page_cache.write(node.page_id);
        } else {
            fresh_node.set_parent(node.get_parent());
            page_cache.write(fresh_node.page_id);
            auto node_parent = node.get_parent(page_cache);
            fresh_node = Node<T>(page_cache, fresh_node_page_id, t);
            node = Node<T>(page_cache, node_page_id, t);
            int position_in_parent = 0;
            while (position_in_parent < node_parent.get_keys_count() &&
                   split_key > node_parent.get_key_at(position_in_parent)) {
                position_in_parent++;
            }

            for (int i = node_parent.get_keys_count(); i > position_in_parent; i--) {
                node_parent.set_key_at(i, node_parent.get_key_at(i - 1));
            }
            for (int i = node_parent.get_keys_count() + 1; i > position_in_parent + 1; i--) {
                node_parent.set_children_at(i, node_parent.get_child_at(i - 1));
            }
            node_parent.set_key_at(position_in_parent, split_key);
            node_parent.set_children_at(position_in_parent + 1, fresh_node.page_id);
            node_parent.set_keys_count(node_parent.get_keys_count() + 1);
            node_parent.set_children_count(node_parent.get_children_count() + 1);
            page_cache.write(node_parent.page_id);
            if (node_parent.get_keys_count() == 2 * t) {
                page_cache.write(node.page_id);
                page_cache.write(fresh_node.page_id);
                split_node(node_parent);
                return;
            }
        }
        page_cache.write(node.page_id);
        page_cache.write(fresh_node.page_id);
    }

    static int calculate_t_from_page_size(int page_size) {
        return (page_size - 4 * sizeof(int) - 2 * sizeof(int)) / (2 * sizeof(T) + 4 * sizeof(int)) - 4;
    }

public:
    PageId root_page_id;

    BTree(PageCache &page_cache, int default_page_size) : page_cache(page_cache),
                                                          t(calculate_t_from_page_size(default_page_size)),
                                                          shadow_root(page_cache, t),
                                                          root_page_id(shadow_root.page_id) {};

    BTree(PageCache &page_cache, PageId page_id, int default_page_size) : page_cache(page_cache),
                                                                          t(calculate_t_from_page_size(
                                                                                  default_page_size)),
                                                                          shadow_root(page_cache, page_id, t),
                                                                          root_page_id(page_id) {};

    ~BTree() = default;

    int sync() {
        return page_cache.sync();
    }

    bool insert_key(T key) {
        auto leaf = find_suitable_leaf(key);
        if (leaf.has_key(key)) {
            return false;
        }
        int pos_to_insert = 0;
        while (pos_to_insert < leaf.get_keys_count() && key > leaf.get_key_at(pos_to_insert)) { pos_to_insert++; }

        for (int i = leaf.get_keys_count(); i > pos_to_insert; i--) {
            leaf.set_key_at(i, leaf.get_key_at(i - 1));
        }
        leaf.set_key_at(pos_to_insert, key);
        leaf.set_keys_count(leaf.get_keys_count() + 1);
        page_cache.write(leaf.page_id);
        if (leaf.get_keys_count() == 2 * t) {
            split_node(leaf);
        }

        return true;
    }

    bool has_key(T key) {
        auto leaf = find_suitable_leaf(key);
        for (int i = 0; i < leaf.get_keys_count(); i++) {
            if (key == leaf.get_key_at(i)) {
                return true;
            }
        }
        return false;
    }
};



#endif //QDB_INDEX_H

#include "btree.h"

template <typename T>
BTree<T>::BTree(size_t _min_degree, bool (*compare)(T, T)) {
    min_degree = _min_degree;
    comparator = compare;
    root = (Node<T>*) malloc(sizeof(Node<T>));
    init_node(root);
    root->is_leaf = true;
}

template <typename T>
BTree<T>::~BTree() {
    free_node(root);
}

template <typename T>
void BTree<T>::insert(T k) {
    if (root->size == 2 * min_degree - 1) {
        Node<T> *add = (Node<T>*) malloc(sizeof(Node<T>));
        init_node(add);
        add->is_leaf = false;
        add->childs[0] = root;
        root = add;
        split(add, 0);
    }

    Node<T> *curr = root;

    while (!curr->is_leaf) {
        int index = curr->size - 1;
        while (index >= 0 && comparator(k, curr->key[index])) {
            index--;
        }
        index++;

        if (curr->childs[index]->size == 2 * min_degree - 1) {
            split(curr, index);
            if (comparator(curr->key[index], k)) {
                index++;
            }
        }
        curr = curr->childs[index];
    }

    insert_node(curr, k);
}

template <typename T>
T BTree<T>::remove(T k) {
    Node<T> *curr = root;
    for (;;) {
        size_t i = find_node(curr, k);

        if (i < curr->size && !(comparator(curr->key[i], k) || comparator(k, curr->key[i]))) {
            T result = curr->key[i];

            if (curr->is_leaf) {
                delete_node(curr, i);
            } else {
                Node<T> *left = curr->childs[i];
                Node<T> *right = curr->childs[i + 1];

                if (left->size >= min_degree) {
                    while (!(left->is_leaf)) {
                        recalc(left, left->size);
                        left = left->childs[left->size];
                    }
                    curr->key[i] = delete_node(left, left->size - 1);
                } else if (right->size >= min_degree) {
                    while (!(right->is_leaf)) {
                        recalc(right, 0);
                        right = right->childs[0];
                    }
                    curr->key[i] = delete_node(right, 0);
                } else {
                    merge(curr, i);
                    curr = left;
                    continue;
                }
            }

            return result;
        } else {
            if (curr->is_leaf) {
                std::cerr << "ERROR: key not found";
            }

            char result = recalc(curr, i);
            if (result == 2) {
                curr = root;
            } else {
                curr = curr->childs[find_node(curr, k)];
            }
        }
    }
}

template <typename T>
std::pair<Node<T>*, size_t> BTree<T>::search(T k) {
    Node<T> *x = root;

    for (;;) {
        size_t i = find_node(x, k);

        if (i < x->size && !(comparator(k, x->key[i]) || comparator(x->key[i], k))) {
            return std::pair<Node<T>*, size_t>(x, i);
        } else if (x->is_leaf) {
            return std::pair<Node<T>*, size_t>(NULL, 0);
        } else {
            x = x->childs[i];
        }
    }
}

template <typename T>
T BTree<T>::search_key(T k) {    
    std::pair<Node<T>*, size_t> node = search(k);

    if (node.first == NULL) {
        std::cerr << "ERROR: key not found";
    }

    return node.first->key[node.second];
}

template <typename T>
void BTree<T>::init_node(Node<T> *x) {
    x->size = 0;
    x->key = (T*) malloc((2 * min_degree - 1) * sizeof(T));
    x->childs = (Node<T>**) malloc(2 * min_degree * sizeof(Node<T>*));
}

template <typename T>
void BTree<T>::free_node(Node<T> *x) {
    if (!x->is_leaf) {
        for (size_t i = 0; i <= x->size; i++) {
            free_node(x->childs[i]);
        }
    }

    free(x->childs);
    free(x->key);
    free(x);
}

template <typename T>
size_t BTree<T>::find_node(Node<T> *x, T k) {
    size_t i = 0;

    while (i < x->size && comparator(x->key[i], k)) {
        i++;
    }

    return i;
}

template <typename T>
size_t BTree<T>::insert_node(Node<T> *x, T k) {
    int index;

    for (index = x->size; index > 0 && comparator(k, x->key[index - 1]); index--) {
        x->key[index] = x->key[index - 1];
        x->childs[index + 1] = x->childs[index];
    }

    std::cerr << x->key[index] << " " << "INSERT NODE\n";
    x->childs[index + 1] = x->childs[index];
    x->key[index] = k;
    x->size++;

    return index;
}

template <typename T>
T BTree<T>::delete_node(Node<T> *x, size_t index) {
    T result = x->key[index];
    x->size--;

    while (index < x->size) {
        x->key[index] = x->key[index + 1];
        x->childs[index + 1] = x->childs[index + 2];
        index++;
    }

    return result;
}

template <typename T>
void BTree<T>::split(Node<T> *x, int i) {
    Node<T> *split_node = x->childs[i];
    Node<T>* add = (Node<T>*) malloc(sizeof(Node<T>));;
    init_node(add);

    add->is_leaf = split_node->is_leaf;
    add->size = min_degree - 1;

    for (size_t j = 0; j < min_degree - 1; j++) {
        add->key[j] = split_node->key[j + min_degree];
    }
    
    if (!split_node->is_leaf) {
        for (size_t j = 0; j < min_degree; j++) {
            add->childs[j] = split_node->childs[j + min_degree];
        }
    }
    
    split_node->size = min_degree - 1;

    insert_node(x, split_node->key[min_degree - 1]);
    x->childs[i + 1] = add;
}

template <typename T>
char BTree<T>::merge(Node<T> *parent, size_t i) {

    Node<T> *left = parent->childs[i];
    Node<T> *right = parent->childs[i + 1];

    left->key[left->size] = delete_node(parent, i);
    size_t j = ++(left->size);

    for (size_t k = 0; k < right->size; k++) {
        left->key[j + k] = right->key[k];
        left->childs[j + k] = right->childs[k];
    }
    left->size += right->size;
    left->childs[left->size] = right->childs[right->size];

    free(right->childs);
    free(right->key);
    free(right);

    if (parent->size == 0) {
        root = left;
        free(parent->childs);
        free(parent->key);
        free(parent);
        return 2;
    }

    return 1;
}

template <typename T>
char BTree<T>::recalc(Node<T> *parent, size_t index) {
    Node<T> *kid = parent->childs[index];

    if (kid->size < min_degree) {
        if (index != 0 && parent->childs[index - 1]->size >= min_degree) {
            Node<T> *leftKid = parent->childs[index - 1];

            for (size_t i = insert_node(kid, parent->key[index - 1]); i != 0; i--) {
                kid->childs[i] = kid->childs[i - 1];
            }

            kid->childs[0] = leftKid->childs[leftKid->size];
            parent->key[index - 1] = delete_node(leftKid, leftKid->size - 1);
        } else if (index != parent->size && parent->childs[index + 1]->size >= min_degree) {
            Node<T> *rightKid = parent->childs[index + 1];
            insert_node(kid, parent->key[index]);
            kid->childs[kid->size] = rightKid->childs[0];
            rightKid->childs[0] = rightKid->childs[1];
            parent->key[index] = delete_node(rightKid, 0);
        } else if (index != 0) {
            return merge(parent, index - 1);
        } else {
            return merge(parent, index);
        }

        return 1;
    }

    return 0;
}
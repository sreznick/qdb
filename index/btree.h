#pragma once

#include <iostream>
#include <stdlib.h>
#include <utility>
#include <stdio.h>

template <typename T>
struct Node {
	Node<T> **childs;
	T *key;
	size_t size;
	bool is_leaf;
};

template <typename T>
class BTree {
private:
	void init_node(Node<T>*);
	void free_node(Node<T>*);
	size_t find_node(Node<T>*, T);
	size_t insert_node(Node<T>*, T);
	T delete_node(Node<T>*, size_t);
	void split(Node<T>*, int);
	char merge(Node<T>*, size_t);
	char recalc(Node<T>*, size_t);

	Node<T> *root;
	bool (*comparator)(T, T);
	size_t min_degree;

public:
    // Min degree and compare function for T
	BTree(size_t, bool (*)(T, T));
	~BTree();

	void insert(T);
	T remove(T);
	std::pair<Node<T>*, size_t> search(T);
	T search_key(T);
};

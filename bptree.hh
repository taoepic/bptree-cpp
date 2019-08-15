/*
 * B+ Tree C++ Implementation
 *
 * Tao Zhang <taoepic@gmail.com>
*/
#ifndef BPTREE_HH___
#define BPTREE_HH___

#include <cstdint>
#include <climits>
#include <vector>
#include <iostream>
#include <cassert>

enum bpnode_type : uint8_t {
	NODE_NONE = 0,
	NODE_LEAF = 0x1,
	NODE_INNER = 0x2,
};

template <typename K, typename V> class bptree;

template <typename K, typename V>
class bpnode {
protected:
	bpnode_type type;
	int16_t num_keys;
	std::vector<K> keys;
	bpnode<K,V> *parent;

public:
	friend class bptree<K,V>;
	bpnode(bpnode_type type_, int16_t nk_, bpnode<K,V> *p_) :
		type{type_}, num_keys{nk_}, parent{p_} {}
	virtual bpnode_type get_type() const noexcept = 0;
	int16_t get_num_keys() const noexcept { return num_keys; }
	bool is_leaf() const noexcept { return get_type() == NODE_LEAF; }
	bool is_inner() const noexcept { return get_type() == NODE_INNER; }
	virtual ~bpnode() {};

private:
	int check_children_index_by_key(const K& key) const noexcept;
};

template <typename K, typename V>
class bpnode_leaf : public bpnode<K,V> {
protected:
	std::vector<V> values;	
	bpnode<K,V> *next;

public:
	friend class bptree<K,V>;
	bpnode_leaf(int m_) : next{nullptr}, bpnode<K,V>{NODE_LEAF, 0, nullptr} {
		this->keys.reserve(m_);
		values.reserve(m_);
	}
	virtual bpnode_type get_type() const noexcept override { return NODE_LEAF; }
	virtual ~bpnode_leaf() {}
};


template <typename K, typename V>
class bpnode_inner : public bpnode<K,V> {
protected:
	std::vector<bpnode<K,V>*> children;

public:
	friend class bptree<K,V>;
	bpnode_inner(int m_) : bpnode<K,V>{NODE_INNER, 0, nullptr} {
		this->keys.reserve(m_);
		children.reserve(m_ + 1);
	}
	virtual bpnode_type get_type() const noexcept override { return NODE_INNER; }
	virtual ~bpnode_inner() {}
};

template <typename K, typename V>
class bptree {
protected:
	bpnode<K,V> *root;
	int depth;
	int count;
	int m;

public:
	bptree(int m_) : m{m_}, depth{0}, count{0}, root{nullptr} {}
	~bptree();
	int get_count() const noexcept { return count; }
	int get_depth() const noexcept { return depth; }
	bool find_key(const K& key, V*& value) const noexcept;
	void insert_key(const K& key, const V& value) noexcept;
	bool delete_key(const K& key) noexcept;
	void dump() const noexcept;
	void dump_brief() const noexcept;
	void dump_leaf_keys() const noexcept;
	void check() const noexcept;

private:
	void check_node(bpnode<K,V>* p, bpnode<K,V>* n) const noexcept;
	void destroy_node(bpnode<K,V> *n);
	bool find_leaf(const K& key, int& idx, bpnode_leaf<K,V>*& node) const noexcept;
	void insert_leaf_node(bpnode_leaf<K,V>* n, const K& key, const V& value) noexcept;
	void leaf_split_if_full(bpnode_leaf<K,V>* n) noexcept;
	void insert_inner_node(bpnode_inner<K,V>* n, const K& key,
			bpnode<K,V>* child1, bpnode<K,V>* child2) noexcept;
	void inner_split_if_full(bpnode_inner<K,V>* n) noexcept;
	void dump_node(const bpnode<K,V>* n, 
			const bpnode<K,V>* p, long level) const noexcept;
	void print_keys_range(int level, const K* key, const V* value,
		bool is_leaf, bool is_null) const noexcept;
	void remove_leaf_key(bpnode_leaf<K,V>* n, const K& key) noexcept;
	void get_sibling(bpnode<K,V>* n, 
			bpnode<K,V>** left, bpnode<K,V>** right) const noexcept;
	void check_inner_node_size(bpnode_inner<K,V>* n) noexcept;
	void leaf_borrow_left(bpnode_leaf<K,V>* n, bpnode_leaf<K,V>* s) noexcept;
	void leaf_borrow_right(bpnode_leaf<K,V>* n, bpnode_leaf<K,V>* s) noexcept;
	bpnode_inner<K,V>* leaf_merge_left(
			bpnode_leaf<K,V>* n, bpnode_leaf<K,V>* s) noexcept;
	bpnode_inner<K,V>* leaf_merge_right(
			bpnode_leaf<K,V>* n, bpnode_leaf<K,V>* s) noexcept;
	void inner_borrow_left(bpnode_inner<K,V>* n, bpnode_inner<K,V>* s) noexcept;
	void inner_borrow_right(bpnode_inner<K,V>* n, bpnode_inner<K,V>* s) noexcept;
	bpnode_inner<K,V>* inner_merge_left(
			bpnode_inner<K,V>* n, bpnode_inner<K,V>* s) noexcept;
};

template <typename K, typename V>
bptree<K,V>::~bptree() {
	if (root == nullptr)
		return;
	destroy_node(root);
}

template <typename K, typename V>
void bptree<K,V>::check() const noexcept {
	if (root == nullptr)
		return;

	if (depth == 1 && root->is_inner()) {
		std::cout << "check tree: depth 1 but root inner" << std::endl;
		exit(-1);
	}

	check_node(nullptr, root);
}

template <typename K, typename V>
void bptree<K,V>::check_node(bpnode<K,V>* p, bpnode<K,V>* n) const noexcept {
	if (n->parent != p) {
		std::cout << "check node: found err, parent not match" << std::endl;
		exit(-2);
	}
	if (n->num_keys >= m) {
		std::cout << "check node: found err, num_keys " << n->num_keys << " >= "
				<< m << std::endl;
		exit(-1);
	}

	if (n->is_inner()) {
		bpnode_inner<K,V>* nn = dynamic_cast<bpnode_inner<K,V>*>(n);
		int ks = nn->keys.size();
		int cs = nn->children.size();
		if (ks != nn->num_keys || cs != nn->num_keys + 1) {
			std::cout << "check node: found err, ks " << ks << " cs " << cs 
				<< " key_nums " << nn->num_keys << std::endl;
			exit(-2);
		}
		for (bpnode<K,V>* &c : nn->children)
			check_node(n, c);
	} else {
		bpnode_leaf<K,V>* nn = dynamic_cast<bpnode_leaf<K,V>*>(n);
		int ks = nn->keys.size();
		int vs = nn->keys.size();
		if (ks != nn->num_keys || vs != nn->num_keys) {
			std::cout << "check node: found err, ks " << ks << " vs " << vs
				<< " key_nums " << nn->num_keys << std::endl;
			exit(-3);
		}
	}
}

template <typename K, typename V>
void bptree<K,V>::destroy_node(bpnode<K,V> *n) {
	if (n == nullptr)
		return;
	if (n->is_leaf()) {
		delete n;
		return;
	}
	for (bpnode<K,V>* &c : (dynamic_cast<bpnode_inner<K,V>*>(n))->children) 
		destroy_node(c);
	delete n;
}

template <typename K, typename V>
int bpnode<K,V>::check_children_index_by_key(const K& key) const noexcept {
	int index = 0;
	while (index < num_keys) {
		if (key < keys[index])
			return index;
		index++;
	}
	return index;
}

template <typename K, typename V>
bool bptree<K,V>::find_key(const K& k, V*& v) const noexcept {
	int idx;
	bpnode_leaf<K,V>* n;

	if (find_leaf(k, idx, n)) {
		v = &n->values[idx];
		return true;
	}
	return false;
}

template <typename K, typename V>
bool bptree<K,V>::find_leaf(const K& key, int& idx, bpnode_leaf<K,V>*& node) const noexcept {
	bpnode<K,V> *n = root;
	int i;

	if (n == nullptr) {
		node = nullptr;
		return false;
	}
	
	bpnode_inner<K,V>* inner;
	while (!n->is_leaf()) {
		inner = dynamic_cast<bpnode_inner<K,V>*>(n);
		i = inner->check_children_index_by_key(key);
		assert(inner->children[i] != nullptr);
		n = inner->children[i];
	}
	node = dynamic_cast<bpnode_leaf<K,V>*>(n);
	for (i = 0; i < n->num_keys; i++) {
		if (n->keys[i] == key) {
			idx = i;
			return true;
		}
	}
	return false;
}

template <typename K, typename V>
void bptree<K,V>::insert_key(const K& key, const V& value) noexcept {
	bpnode_leaf<K,V> *n;
	if (root == nullptr) {
		n = new bpnode_leaf<K,V>(m);
		n->keys.push_back(key);
		n->values.push_back(value);
		n->num_keys = 1;
		root = n;
		root->parent = nullptr;
		depth = 1;
		count = 1;
		return;
	}

	int idx;
	if (find_leaf(key, idx, n)) {
		n->values[idx] = value;
		return;
	}

	/* insert into the bottom leaf node */
	insert_leaf_node(n, key, value);
	count++;
}

template <typename K, typename V>
void bptree<K,V>::insert_leaf_node(bpnode_leaf<K,V>* n, const K& key, const V& value) noexcept {
	typename std::vector<K>::iterator it = n->keys.begin();
	typename std::vector<V>::iterator vit = n->values.begin();
	for (; it < n->keys.end(); it++, vit++) {
		if (key < *it) {
			n->keys.insert(it, key);
			n->values.insert(vit, value);
			goto done;
		}
	}
	n->keys.insert(n->keys.end(), key);
	n->values.insert(n->values.end(), value);
done:
	n->num_keys++;
	leaf_split_if_full(n);
}

template <typename K, typename V>
void bptree<K,V>::leaf_split_if_full(bpnode_leaf<K,V>* n) noexcept {
	if (n->num_keys < m)
		return;

	/* split into two, floor(m/2) left, others to new one */
	bpnode_leaf<K,V> *new_leaf = new bpnode_leaf<K,V>(m);
	int k = m / 2;

	new_leaf->keys.insert(new_leaf->keys.begin(), 
		n->keys.begin() + k, n->keys.end());
	new_leaf->values.insert(new_leaf->values.begin(), 
		n->values.begin() + k, n->values.end());
	n->keys.erase(n->keys.begin() + k, n->keys.end());
	n->values.erase(n->values.begin() + k, n->values.end());
	
	new_leaf->num_keys = n->num_keys - k;
	new_leaf->parent = n->parent;
	n->num_keys = k;

	new_leaf->next = n->next;
	n->next = new_leaf;

	/* due to split into two nodes, old n->keys[k] (or new->keys[0]) should
	 * insert into upper inner node
	*/

	insert_inner_node(dynamic_cast<bpnode_inner<K,V>*>(n->parent), 
			new_leaf->keys[0], n, new_leaf);
}

template <typename K, typename V>
void bptree<K,V>::insert_inner_node(bpnode_inner<K,V>* n, const K& key,
					bpnode<K,V>* child1, bpnode<K,V>* child2) noexcept {
	if (n == NULL) {
		/* it's on top, should add a new inner node as new root */
		bpnode_inner<K,V>* new_inner = new bpnode_inner<K,V>(m);
		new_inner->num_keys = 1;
		new_inner->keys.push_back(key);
		new_inner->children.push_back(child1);
		new_inner->children.push_back(child2);
		child1->parent = new_inner;
		child2->parent = new_inner;
		root = new_inner;
		depth++;
		return;
	}

	/* first insert key into the node n at right place
	 * then check whether that inner node is full
	*/
	typename std::vector<K>::iterator it;
	typename std::vector<bpnode<K,V>*>::iterator cit;
	for (it = n->keys.begin(), cit = n->children.begin(); 
				it < n->keys.end(); it++, cit++) {
		if (key < *it) {
			n->keys.insert(it, key);
			*cit = child1;
			cit++;
			n->children.insert(cit, child2);
			n->num_keys++;
			inner_split_if_full(n);
			return;
		}
	}

	n->keys.push_back(key);
	n->children.pop_back();
	n->children.push_back(child1);
	n->children.push_back(child2);
	n->num_keys++;
	inner_split_if_full(n);
}

template <typename K, typename V>
void bptree<K,V>::inner_split_if_full(bpnode_inner<K,V>* n) noexcept {
	if (n->num_keys < m)
		return;

	/* split into two, floor(m/2) left old, others move to new */
	bpnode_inner<K,V>* new_inner = new bpnode_inner<K,V>(m);
	int k = m / 2;

	K up_key = n->keys[k];
	new_inner->keys.insert(new_inner->keys.begin(), 
			n->keys.begin() + k + 1, n->keys.end());
	new_inner->children.insert(new_inner->children.begin(),
			n->children.begin() + k + 1, n->children.end());
	for (auto &c : new_inner->children) 
		c->parent = new_inner;
	new_inner->num_keys = n->num_keys - k - 1;
	new_inner->parent = n->parent;

	n->keys.erase(n->keys.begin() + k, n->keys.end());
	n->children.erase(n->children.begin() + k + 1, n->children.end());
	n->num_keys = k;

	insert_inner_node(dynamic_cast<bpnode_inner<K,V>*>(n->parent), 
		up_key, n, new_inner);
}

template <typename K, typename V>
void bptree<K,V>::dump_brief() const noexcept {
	std::cout << "B+ tree, depth " << depth << ","
		<< "count " << count << "\n";
	if (root == nullptr) {
		std::cout << "<<empty B+ tree>>" << std::endl;
		return;
	}
}

template <typename K, typename V>
void bptree<K,V>::dump() const noexcept {
	std::cout << "B+ tree, depth " << depth << ","
		<< "count " << count << ":\n";
	if (root == nullptr) {
		std::cout << "<<empty B+ tree>>" << std::endl;
		return;
	}
	bool is_min = true, is_max = true;
	dump_node(root, nullptr, 0);
}

template <typename K, typename V>
void bptree<K,V>::print_keys_range(int level, const K* key, const V* value,
		bool is_leaf, bool is_null) const noexcept {
	for (int i = 0; i < level; i++)
		std::cout << "\t";

	if (is_null)
		std::cout << "<null> ";
	else if (is_leaf)
		std::cout << *key << " -> " << *value;
	else
		std::cout << *key;

	std::cout << std::endl;
}

template <typename K, typename V>
void bptree<K,V>::dump_node(const bpnode<K,V>* n, const bpnode<K,V>* p, long level) const noexcept {
	if (n == nullptr) {
		print_keys_range(level, nullptr, nullptr, false, true);
		return;
	}

	if (n->parent != p) {
		std::cout << "ERR: unmatch parent node, num_keys: " 
			<< n->num_keys << std::endl;
		return;
	}
	if (n->is_leaf()) {
		for (int i = 0; i < n->num_keys; i++) {
			print_keys_range(
				level, &n->keys[i], 
				&(dynamic_cast<const bpnode_leaf<K,V>*>(n)->values[i]), 
				true, false);
		}
		return;
	}

	dump_node(dynamic_cast<const bpnode_inner<K,V>*>(n)->children[0], n, level + 1);
	for (int i = 0; i < n->num_keys; i++) {
		print_keys_range(level, &n->keys[i], nullptr, false, false);
		dump_node(dynamic_cast<const bpnode_inner<K,V>*>(n)->children[i + 1], 
			n, level + 1);
	}
}

template <typename K, typename V>
void bptree<K,V>::dump_leaf_keys() const noexcept {
	if (root == nullptr) {
		std::cout << "{}" << std::endl;
		return;
	}

	bpnode<K,V>* n = root;
	while (n->is_inner())
		n = dynamic_cast<bpnode_inner<K,V>*>(n)->children[0];

	while (n != nullptr) {
		std::cout << "{" << n->keys[0];
		for (int i = 1; i < n->num_keys; i++)
			std::cout << "," << n->keys[i];
		std::cout << "} ";
		n = dynamic_cast<bpnode_leaf<K,V>*>(n)->next;
	}
	std::cout << std::endl;
}

template <typename K, typename V>
bool bptree<K,V>::delete_key(const K& key) noexcept {
	bpnode_leaf<K,V>* n;
	int idx;
	if (!find_leaf(key, idx, n)) {
		return false;
	}
	if (count == 1) {
		delete root;
		root = nullptr;
		count = 0;
		depth = 0;
		return true;
	}
	remove_leaf_key(n, key);
	count--;
	return true;
}

template <typename K, typename V>
void bptree<K,V>::remove_leaf_key(bpnode_leaf<K,V>* n, const K& key) noexcept {
	typename std::vector<K>::iterator it;
	typename std::vector<V>::iterator vit;
	int i;
	bpnode_inner<K,V>* p;
	bpnode<K,V> *left, *right;
	int left_count, right_count;

	int min_limits = (m - 1) / 2;

	for (it = n->keys.begin(), vit = n->values.begin(); 
				it < n->keys.end(); it++, vit++) {
		if (key == *it)
			break;
	}
	assert(it != n->keys.end());

	/* delete key in leaf */
	n->keys.erase(it);
	n->values.erase(vit);
	n->num_keys--;

	p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);
	if (p == nullptr) {
		/* top node permits to have less than min_limits keys */
		return;
	}

	for (i = 0; i <= p->num_keys; i++) {
		if (n == p->children[i])
			break;
	}

	if (n->num_keys >= min_limits) {
		/* size ok, just replace upper key if needs */
		if (i > 0) {		
			p->keys.at(i - 1) = n->keys[0];
		}
		return;
	}

	/* needs to check the sibling node to borrow one */
	get_sibling(n, &left, &right);
	left_count = (left == nullptr) ? 0 : left->num_keys;
	right_count = (right == nullptr) ? 0 : right->num_keys;
	if (left_count > min_limits || right_count > min_limits) {
		if (right_count <= left_count) {
			leaf_borrow_left(n, dynamic_cast<bpnode_leaf<K,V>*>(left));
			p->keys.at(i - 1) = n->keys[0];
		} else {
			leaf_borrow_right(n, dynamic_cast<bpnode_leaf<K,V>*>(right));
			p->keys.at(i) = right->keys[0];
		}

		return;
	}

	/* sibling has not enough keys, we need to coalesce */
	if (left_count > right_count) {
		p = leaf_merge_left(n, dynamic_cast<bpnode_leaf<K,V>*>(left));
	} else {
		p = leaf_merge_right(n, dynamic_cast<bpnode_leaf<K,V>*>(right));
	}

	/* after merging, we need to check parent node */
	check_inner_node_size(p);
}

template <typename K, typename V>
void bptree<K,V>::get_sibling(bpnode<K,V>* n, 
			bpnode<K,V>** left, bpnode<K,V>** right) const noexcept {
	int i;

	assert(n->parent != nullptr);
	bpnode_inner<K,V>* p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);
	for (i = 0; i <= p->num_keys; i++) {
		if (p->children[i] == n)
			break;
	}
	assert(i <= p->num_keys);
	if (left != nullptr) 
		*left = ((i == 0) ? nullptr : p->children[i - 1]);
	if (right != nullptr)
		*right = ((i == p->num_keys) ? nullptr : p->children[i + 1]);
}

/* we need to check whether inner node n's key item count < m/2 */
template <typename K, typename V>
void bptree<K,V>::check_inner_node_size(bpnode_inner<K,V>* n) noexcept {
	int min_limits = (m - 1) / 2;
	bpnode<K,V> *left, *right;
	bpnode_inner<K,V> *p;
	int left_count, right_count;

	if (n->num_keys >= min_limits)
		return;

	if (n->num_keys == 0) {
		if (n->parent == nullptr) {
			/* node n is empty and top, we move children to top */
			depth--;
			assert(n->children[0] != nullptr);
			n->children[0]->parent = nullptr;
			root = n->children[0];
			return;
		}
	}

	/* top node, we don't change anything */
	if (n->parent == nullptr)
		return;

	get_sibling(n, &left, &right);
	left_count = (left == nullptr) ? 0 : left->num_keys;
	right_count = (right == nullptr) ? 0 : right->num_keys;
	if (left_count > min_limits || right_count > min_limits) {
		if (right_count >= left_count)
			inner_borrow_right(n, dynamic_cast<bpnode_inner<K,V>*>(right));
		else
			inner_borrow_left(n, dynamic_cast<bpnode_inner<K,V>*>(left));
		return;
	}

	if (left_count > right_count)
		p = inner_merge_left(n, dynamic_cast<bpnode_inner<K,V>*>(left));
	else
		p = inner_merge_left(dynamic_cast<bpnode_inner<K,V>*>(right), n);

	/* after merging, we need to check parent node */
	check_inner_node_size(p);
}

template <typename K, typename V>
void bptree<K,V>::leaf_borrow_left(bpnode_leaf<K,V>* n, bpnode_leaf<K,V>* s) noexcept {
	n->keys.insert(n->keys.begin(), s->keys.back());
	n->values.insert(n->values.begin(), s->values.back());
	n->num_keys++;

	s->keys.pop_back();
	s->values.pop_back();
	s->num_keys--;
}

template <typename K, typename V>
void bptree<K,V>::leaf_borrow_right(bpnode_leaf<K,V>* n, bpnode_leaf<K,V>* s) noexcept {
	n->keys.push_back(s->keys[0]);
	n->values.push_back(s->values[0]);
	n->num_keys++;

	s->keys.erase(s->keys.begin());
	s->values.erase(s->values.begin());
	s->num_keys--;
}

template <typename K, typename V>
bpnode_inner<K,V>* bptree<K,V>::leaf_merge_left(bpnode_leaf<K,V>* n, 
				bpnode_leaf<K,V>* s) noexcept {
	bpnode_inner<K,V>* p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);

	/* coalesce n + s, we don't need to drag p down because of n is leaf
	 * node, we already have the same parent key
	*/
	s->keys.insert(s->keys.end(), n->keys.begin(), n->keys.end());
	s->values.insert(s->values.end(), n->values.begin(), n->values.end());
	s->num_keys += n->num_keys;

	typename std::vector<bpnode<K,V>*>::iterator it;
	typename std::vector<K>::iterator kit;
	for (it = p->children.begin(), kit = p->keys.begin(); 
			it < p->children.end(); it++, kit++) {
		if (n == *it) 
			break;
	}
	assert(it != p->children.begin() && it != p->children.end());

	p->children.erase(it);
	p->keys.erase(kit - 1);
	p->num_keys--;

	s->next = n->next;
	delete n;

	return p;
}

template <typename K, typename V>
bpnode_inner<K,V>* bptree<K,V>::leaf_merge_right(bpnode_leaf<K,V>* n,
			bpnode_leaf<K,V>* s) noexcept {
	int i;
	bpnode_inner<K,V>* p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);

	for (i = 0; i <= p->num_keys; i++) {
		if (n == p->children[i])
			break;
	}
	assert(i < p->num_keys);
	n->keys.insert(n->keys.end(), s->keys.begin(), s->keys.end());
	n->values.insert(n->values.end(), s->values.begin(), s->values.end());
	n->num_keys += s->num_keys;

	p->children.erase(p->children.begin() + i + 1);
	p->keys.erase(p->keys.begin() + i);
	p->num_keys--;

	if (i > 0)
		p->keys.at(i - 1) = n->keys[0];
	n->next = s->next;
	delete s;

	return p;
}

template <typename K, typename V>
void bptree<K,V>::inner_borrow_left(bpnode_inner<K,V>* n, 
				bpnode_inner<K,V>* s) noexcept {
	int i;
	bpnode_inner<K,V>* p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);

	/* find n index in p children */
	for (i = 0; i <= p->num_keys; i++) {
		if (n == p->children[i])
			break;
	}
	assert(i > 0);

	/* move parent key to n's head, and link s children tail to n left children */
	n->keys.insert(n->keys.begin(), p->keys[i - 1]);
	n->children.insert(n->children.begin(), s->children.back());
	n->children[0]->parent = n;
	n->num_keys++;

	/* move s keys tail to parent */
	p->keys[i - 1] = s->keys.back();
	s->keys.pop_back();
	s->children.pop_back();
	s->num_keys--;
}

template <typename K, typename V>
void bptree<K,V>::inner_borrow_right(bpnode_inner<K,V>* n,
				bpnode_inner<K,V>* s) noexcept {
	int i;
	bpnode_inner<K,V>* p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);

	for (i = 0; i <= p->num_keys; i++) {
		if (n == p->children[i])
			break;
	}
	assert(i < p->num_keys);

	/* move parent key to n's tail, and link s children head to n tail children */
	n->keys.push_back(p->keys[i]);
	n->children.push_back(s->children[0]);
	n->children.back()->parent = n;
	n->num_keys++;

	/* move s head key to parent */
	p->keys[i] = s->keys[0];

	/* remove s head key and children */
	s->keys.erase(s->keys.begin());
	s->children.erase(s->children.begin());
	s->num_keys--;
}

template <typename K, typename V>
bpnode_inner<K,V>* bptree<K,V>::inner_merge_left(bpnode_inner<K,V>* n,
				bpnode_inner<K,V>* s) noexcept {
	int i;
	bpnode_inner<K,V>* p = dynamic_cast<bpnode_inner<K,V>*>(n->parent);
	assert(p && p->num_keys > 0);

	for (i = 0; i <= p->num_keys; i++) {
		if (n == p->children[i])
			break;
	}
	assert(i <= p->num_keys && i > 0);

	/* drag down parent key i-1 append to s */
	s->keys.push_back(p->keys[i - 1]);

	/* merge n to s tail, include key and children */
	s->keys.insert(s->keys.end(), n->keys.begin(), n->keys.end());
	s->children.insert(s->children.end(), n->children.begin(), n->children.end());
	for (auto &c : s->children)
		c->parent = s;
	s->num_keys += n->num_keys + 1;

	/* remove parent key i-1 */
	p->keys.erase(p->keys.begin() + i - 1);
	p->children.erase(p->children.begin() + i);
	p->num_keys--;

	delete n;
	return p;
}

#endif


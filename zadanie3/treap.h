/*
 * treap.hpp
 * Maciej Walczak
 */

#ifndef TREAP_HPP_
#define TREAP_HPP_

#include <random>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>
#include <algorithm>

template<typename T, typename CompareType, typename URNG>
class treap;

namespace treap_impl {

using priority_t = int;

struct node_base {
	node_base* m_left;
	node_base* m_right;
	node_base* m_parent;

	node_base() noexcept :
	m_left(nullptr), m_right(nullptr), m_parent(nullptr)
	{
	}
};

template<typename T>
struct node : public node_base {

	node(priority_t priority, const T& data) :
		m_data(data), m_priority(priority)
	{
	}

	node(priority_t priority, T&& data) :
		m_data(data), m_priority(priority)
	{
	}

	template<typename ... Args>
	node(priority_t priority, Args&&... args) :
		m_data(std::forward<Args>(args)...), m_priority(priority)
	{
	}

	node(const node& other) :
		node(other.m_data, other.m_priority)
	{
	}

	T m_data;
	priority_t m_priority;

};

node_base* find_lowest(node_base* ptr)
{
	while (ptr->m_left != nullptr) {
		ptr = ptr->m_left;
	}
	return ptr;
}

node_base* find_highest(node_base* ptr)
{
	while (ptr->m_right != nullptr) {
		ptr = ptr->m_right;
	}
	return ptr;
}

node_base* next(node_base* ptr)
{
	if (ptr->m_right != nullptr) {
		return find_lowest(ptr->m_right);
	}
	else {
		node_base* parent = ptr->m_parent;
		while (parent->m_right == ptr) {
			ptr = parent;
			parent = parent->m_parent;
		}
		//check if we reached end()
		if (ptr->m_right == parent) {
			return ptr;
		}
		else {
			return parent;
		}
	}
}

node_base* prev(node_base* ptr)
{
	if (ptr->m_left != nullptr) {
		return find_highest(ptr->m_left);
	}
	else {
		node_base* parent = ptr->m_parent;
		while (parent->m_left == ptr) {
			ptr = parent;
			parent = parent->m_parent;
		}
		return parent;
	}
}

template<typename T>
class iterator : public std::iterator<std::bidirectional_iterator_tag, T> {

	template<typename U>
	friend class const_iterator;

	template<typename U, typename CompareType, typename URNG>
	friend class ::treap;

public:

	iterator() :
		m_ptr(nullptr)
	{
	}

	explicit iterator(node_base* p) :
		m_ptr(p)
	{
	}

	typename std::iterator_traits<iterator>::reference operator*() const noexcept
	{
		return static_cast<node<T>*>(m_ptr)->m_data;
	}

	typename std::iterator_traits<iterator>::pointer operator->() const noexcept
	{
		return &static_cast<node<T>*>(m_ptr)->m_data;
	}

	iterator& operator++() noexcept
	{
		m_ptr = next(m_ptr);
		return *this;
	}

	iterator operator++(int) noexcept
	{
		iterator tmp = *this;
		m_ptr = next(m_ptr);
		return tmp;
	}

	iterator& operator--() noexcept
	{
		m_ptr = prev(m_ptr);
		return *this;
	}
	iterator operator--(int) noexcept
	{
		iterator tmp = *this;
		m_ptr = prev(m_ptr);
		return tmp;
	}

	bool operator==(iterator other) const noexcept
	{
		return m_ptr == other.m_ptr;
	}

	bool operator!=(iterator other) const noexcept
	{
		return m_ptr != other.m_ptr;
	}

private:
	node_base* m_ptr;
};

template<typename T>
class const_iterator : public std::iterator<std::bidirectional_iterator_tag, const T> {

	template<typename U, typename CompareType, typename URNG>
	friend class ::treap;

public:

	const_iterator() :
		m_ptr(nullptr)
	{
	}

	explicit const_iterator(const node_base* p) :
		m_ptr(p)
	{
	}

	const_iterator(const iterator<T>& it) :
		m_ptr(it.m_ptr)
	{
	}

	typename std::iterator_traits<const_iterator>::reference operator*() const noexcept
	{
		return static_cast<const node<T>*>(m_ptr)->m_data;
	}

	typename std::iterator_traits<const_iterator>::pointer operator->() const noexcept
	{
		return &static_cast<const node<T>*>(m_ptr)->m_data;
	}

	const_iterator& operator++() noexcept
	{
		m_ptr = next(const_cast<node_base*>(m_ptr));
		return *this;
	}

	const_iterator operator++(int) noexcept
	{
		const_iterator tmp = *this;
		m_ptr = next(const_cast<node_base*>(m_ptr));
		return tmp;
	}

	const_iterator& operator--() noexcept
	{
		m_ptr = prev(const_cast<node_base*>(m_ptr));
		return *this;
	}

	const_iterator operator--(int) noexcept
	{
		const_iterator tmp = *this;
		m_ptr = prev(const_cast<node_base*>(m_ptr));
		return tmp;
	}

	bool operator==(const_iterator other) const noexcept
	{
		return m_ptr == other.m_ptr;
	}

	bool operator!=(const_iterator other) const noexcept
	{
		return m_ptr != other.m_ptr;
	}

private:
	const node_base* m_ptr;
};

class default_urng {
	std::uniform_int_distribution<priority_t> m_dist { std::numeric_limits<priority_t>::lowest(),
			std::numeric_limits<priority_t>::max() };
	std::mt19937 m_eng { std::random_device { }() };

public:
	priority_t operator()()
	{
		return m_dist(m_eng);
	}
};

template<class ...> using void_t = void;

template<class, class = void_t<>>
struct check_urng : std::false_type {
};

template<class T>
struct check_urng<T, void_t<decltype(std::declval<T&>()())>> : std::true_type {
};

} // namespace treap_impl

template<typename T, typename CompareType, typename URNG>
class treap;

namespace adl_and_std_impl {

//needed for enabling both std::swap and ADL swap
using std::swap;

template<typename T, typename CompareType, typename URNG>
void swap(treap<T, CompareType, URNG>& a, treap<T, CompareType, URNG>& b) noexcept(
	noexcept(swap(std::declval<CompareType&>(), std::declval<CompareType&>())) &&
	noexcept(swap(std::declval<URNG&>(), std::declval<URNG&>())) &&
	noexcept(swap(
			std::declval<std::uniform_int_distribution<typename treap_impl::priority_t>&>(),
			std::declval<std::uniform_int_distribution<typename treap_impl::priority_t>&>()
		)) // I don't think uniform_int_distribution is guaranteed to be noexcept swappable
);

} // namespace adl_and_std_impl

template<typename T, typename CompareType = std::less<T>, typename URNG = treap_impl::default_urng>
class treap {

private:
//template<typename T2>

	using node = treap_impl::node<T>;
	using node_base = treap_impl::node_base;

	enum class insert_pos {
		LEFT = 1, RIGHT = 2, DUPLICATE = 0
	};

public:
	using value_type = T;
	using pointer = value_type*;
	using const_pointer = const value_type*;
	using reference = value_type&;
	using const_reference = const value_type&;

	using iterator = treap_impl::iterator<T>;
	using const_iterator = treap_impl::const_iterator<T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	using value_compare = CompareType;
	using random = URNG;
	using priority = treap_impl::priority_t;

private:
	CompareType m_compare;
	URNG m_urng;
	std::uniform_int_distribution<priority> m_distribution;
	node_base m_header;

public:
	treap() noexcept(
		std::is_nothrow_default_constructible<CompareType>::value &&
		std::is_nothrow_default_constructible<URNG>::value &&
		std::is_nothrow_default_constructible<std::uniform_int_distribution<priority>>::value
	)
	{
		m_header.m_left = &m_header; //when empty begin() == end()
	}

	explicit treap(CompareType cmp, URNG&& urng = URNG()) :
		m_compare(cmp), m_urng(urng)
	{
		m_header.m_left = &m_header; //when empty begin() == end()
	}

	treap(std::initializer_list<value_type> il, CompareType cmp = CompareType(), URNG&& urng =
		URNG()) :
		treap(il.begin(), il.end(), cmp, std::forward<URNG>(urng))
	{
	}

	//Chyba te enable_ify są niepotrzebne, napisałem je zanim się zorientowałem, że
	//"błędy", które miałem to bug w parserze eclipse. Nie przeszkadzają więc zostawię.

	template<typename = typename std::enable_if_t<treap_impl::check_urng<URNG>::value>>
	treap(std::initializer_list<value_type> il, URNG&& urng) :
		treap(il, CompareType(), std::forward<URNG>(urng))
	{
	}

	template<typename = typename std::enable_if_t<treap_impl::check_urng<URNG>::value>>
	explicit treap(URNG&& urng) :
		treap(CompareType(), std::forward<URNG>(urng))
	{
	}

	template<typename InputIt>
	treap(InputIt first, InputIt last, CompareType cmp = CompareType(), URNG&& urng = URNG()) :
		treap(cmp, std::forward<URNG>(urng))
	{
		insert(first, last);
	}

	template<typename InputIt, typename = typename std::enable_if_t<
		treap_impl::check_urng<URNG>::value>>
	treap(InputIt first, InputIt last, URNG&& urng) :
		treap(first, last, CompareType(), std::forward<URNG>(urng))
	{
	}

	/**
	 * Copy constructor.
	 */
	treap(const treap& other) :
		m_compare(other.m_compare), m_urng(other.m_urng), m_distribution(other.m_distribution)
	{
		if (!other.empty()) {
			root_ptr() = copy(static_cast<const node*>(other.root_ptr()), &m_header);
			leftmost_ptr() = find_lowest(root_ptr());
			rightmost_ptr() = find_highest(root_ptr());
		}
		else {
			fix_empty();
		}
	}

private:

	treap(node* nd)
	{
		root_ptr() = nd;
		leftmost_ptr() = find_lowest(root_ptr());
	}

public:

	/**
	 * Copy assignment.
	 */
	treap& operator=(const treap& other)
	{
		treap tmp(other);
		swap(*this, tmp);
		return *this;
	}

	/**
	 * Move constructor.
	 */
	treap(treap&& other) noexcept(
		noexcept(swap(std::declval<treap&>(), std::declval<treap&>())) &&
		std::is_nothrow_default_constructible<treap>::value
	) :
		treap()
	{
		swap(*this, other);
	}

	/**
	 * Move assignment.
	 */
	treap& operator=(treap&& other)
	{
		swap(*this, other);
		return *this;
	}

	/**
	 * Destructor.
	 */
	~treap()
	{
		erase_no_rebalance(static_cast<node*>(root_ptr()));
	}

	treap& operator=(std::initializer_list<value_type> ilist)
	{
		clear();
		insert(ilist.begin(), ilist.end());
		return *this;
	}

	friend void swap(treap& a, treap& b) noexcept(noexcept(adl_and_std_impl::swap(a, b)))
	{
		node_base tmp;

		if (a.empty()) {
			tmp.m_left = &b.m_header;
			tmp.m_parent = tmp.m_right = nullptr;
		}
		else {
			tmp = a.m_header;
			a.root_ptr()->m_parent = &b.m_header;
		}

		if (b.empty()) {
			a.fix_empty();
		}
		else {
			a.m_header = b.m_header;
			b.root_ptr()->m_parent = &a.m_header;
		}

		b.m_header = tmp;

		using std::swap;

		swap(a.m_compare, b.m_compare);
		swap(a.m_urng, b.m_urng);
		swap(a.m_distribution, b.m_distribution);
	}

	bool empty() const noexcept
	{
		return root_ptr() == nullptr;
	}

	reference lowest()
	{
		return data(leftmost_ptr());
	}

	const_reference lowest() const
	{
		return data(leftmost_ptr());
	}

	reference highest()
	{
		return data(rightmost_ptr());
	}

	const_reference highest() const
	{
		return data(rightmost_ptr());
	}

	std::pair<iterator, bool> insert(const value_type& elem)
	{
		return insert_impl(elem);
	}

	iterator insert(iterator hint, const value_type& elem)
	{
		return insert_hint_impl(hint, elem);
	}

	std::pair<iterator, bool> insert(value_type&& elem)
	{
		return insert_impl(std::move(elem));
	}

	iterator insert(iterator hint, value_type&& elem)
	{
		return insert_hint_impl(hint, std::move(elem));
	}

	template<typename InputIt>
	void insert(InputIt first, InputIt last)
	{
		for (; first != last; ++first) {
			std::pair<node_base*, insert_pos> pos = find_insert_pos(*first);
			if (pos.second == insert_pos::DUPLICATE) {
				continue;
			}
			insert_rebalance(pos.first, pos.second == insert_pos::LEFT, create_node(*first));
		}
	}

	void insert(std::initializer_list<value_type> ilist)
	{
		insert(ilist.begin(), ilist.end());
	}

	template<typename ... Args>
	std::pair<iterator, bool> emplace(Args&& ... args)
	{
		node* nd = create_node(std::forward<Args>(args)...);
		std::pair<node_base*, insert_pos> pos = find_insert_pos(nd->m_data);
		if (pos.second == insert_pos::DUPLICATE) {
			delete nd;
			return std::make_pair(iterator(pos.first), false);
		}
		insert_rebalance(pos.first, pos.second == insert_pos::LEFT, nd);
		return std::make_pair(iterator(nd), static_cast<bool>(pos.second));
	}

	template<typename ... Args>
	iterator emplace_hint(iterator hint, Args&& ... args)
	{
		node* nd = create_node(std::forward<Args>(args)...);
		std::pair<node_base*, insert_pos> pos = find_insert_pos_hint(hint, nd->m_data);
		if (pos.second == insert_pos::DUPLICATE) {
			delete nd;
			return iterator(pos.first);
		}
		insert_rebalance(pos.first, pos.second == insert_pos::LEFT, nd);
		return iterator(nd);
	}

	/**
	 * @return Whether or not `elem` was removed.
	 */
	bool erase(const value_type& elem)
	{
		iterator it = find(elem);
		if (it != end()) {
			erase(it);
			return true;
		}
		return false;
	}

	iterator erase(iterator it)
	{
		iterator tmp = it;
		++tmp;
		erase_rebalance(static_cast<node*>(it.m_ptr));
		return tmp;
	}

	iterator erase(iterator first, iterator last)
	{
		iterator tmp;
		while (first != last) {
			tmp = first;
			++tmp;
			erase_rebalance(static_cast<node*>(first.m_ptr));
			first = tmp;
		}
		return first;
	}

	void clear() noexcept
	{
		erase_no_rebalance(static_cast<node*>(root_ptr()));
		fix_empty();
	}

	iterator begin() noexcept
	{
		return iterator(begin_ptr());
	}

	const_iterator begin() const noexcept
	{
		return cbegin();
	}

	const_iterator cbegin() const noexcept
	{
		return const_iterator(begin_ptr());
	}

	iterator end() noexcept
	{
		return iterator(end_ptr());
	}

	const_iterator end() const noexcept
	{
		return cend();
	}

	const_iterator cend() const noexcept
	{
		return const_iterator(end_ptr());
	}

	reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const noexcept
	{
		return crbegin();
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return const_reverse_iterator(cend());
	}

	reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const noexcept
	{
		return crend();
	}

	const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(cbegin());
	}

	bool contains(const value_type& elem) const
	{
		return find(elem) != end();
	}

	iterator find(const value_type& elem)
	{
		return iterator(const_cast<node_base*>(find_impl(elem)));
	}

	const_iterator find(const value_type& elem) const
	{
		return const_iterator(find_impl(elem));
	}

	iterator upper_bound(const value_type& elem)
	{
		return iterator(const_cast<node_base*>(upper_bound_impl(elem)));
	}

	const_iterator upper_bound(const value_type& elem) const
	{
		return const_iterator(upper_bound_impl(elem));
	}

	iterator lower_bound(const value_type& elem)
	{
		return iterator(const_cast<node_base*>(lower_bound_impl(elem)));
	}

	const_iterator lower_bound(const value_type& elem) const
	{
		return const_iterator(lower_bound_impl(elem));
	}

	friend bool operator==(const treap& a, const treap& b)
	{
		return std::equal(a.begin(), a.end(), b.begin());
	}

	friend bool operator!=(const treap& a, const treap& b)
	{
		return !(a == b);
	}

private:
	node_base*& leftmost_ptr()
	{
		return m_header.m_left;
	}

	const node_base* leftmost_ptr() const
	{
		return m_header.m_left;
	}

	node_base*& rightmost_ptr()
	{
		return m_header.m_right;
	}

	const node_base* rightmost_ptr() const
	{
		return m_header.m_right;
	}

	node_base*& root_ptr()
	{
		return m_header.m_parent;
	}

	const node_base* root_ptr() const
	{
		return m_header.m_parent;
	}

	node_base* begin_ptr()
	{
		return m_header.m_left;
	}

	const node_base* begin_ptr() const
	{
		return m_header.m_left;
	}

	node_base* end_ptr()
	{
		return &m_header;
	}

	const node_base* end_ptr() const
	{
		return &m_header;
	}

	static node* right(node_base* nd)
	{
		return static_cast<node*>(nd->m_right);
	}

	static const node* right(const node_base* nd)
	{
		return static_cast<const node*>(nd->m_right);
	}

	static node* left(node_base* nd)
	{
		return static_cast<node*>(nd->m_left);
	}

	static const node* left(const node_base* nd)
	{
		return static_cast<const node*>(nd->m_left);
	}

	static node* parent(node_base* nd)
	{
		return static_cast<node*>(nd->m_parent);
	}

	static value_type& data(node_base* nd)
	{
		return static_cast<node*>(nd)->m_data;
	}

	static const value_type& data(const node_base* nd)
	{
		return static_cast<const node*>(nd)->m_data;
	}

	void fix_empty()
	{
		leftmost_ptr() = end_ptr(); //when empty begin() == end()
		root_ptr() = rightmost_ptr() = nullptr;
	}

	template<typename Arg>
	std::pair<iterator, bool> insert_impl(Arg&& arg)
	{
		std::pair<node_base*, insert_pos> pos = find_insert_pos(arg);
		if (pos.second == insert_pos::DUPLICATE) {
			return std::make_pair(iterator(pos.first), false);
		}
		node* nd = create_node(std::forward<Arg>(arg));
		insert_rebalance(pos.first, pos.second == insert_pos::LEFT, nd);
		return std::make_pair(iterator(nd), static_cast<bool>(pos.second));
	}

	template<typename Arg>
	iterator insert_hint_impl(iterator hint, Arg&& arg)
	{
		std::pair<node_base*, insert_pos> pos = find_insert_pos_hint(hint, arg);
		if (pos.second == insert_pos::DUPLICATE) {
			return iterator(pos.first);
		}
		node* nd = create_node(std::forward<Arg>(arg));
		insert_rebalance(pos.first, pos.second == insert_pos::LEFT, nd);
		return iterator(nd);
	}

	std::pair<node_base*, insert_pos> empty_pos()
	{
		return std::make_pair(&m_header, insert_pos::LEFT);
	}

	std::pair<node_base*, insert_pos> find_insert_pos_hint(iterator hint, const value_type& val)
	{
		if (empty()) {
			return empty_pos();
		}

		if (hint.m_ptr == end_ptr()) {
			if (m_compare(highest(), val)) {
				return std::make_pair(rightmost_ptr(), insert_pos::RIGHT);
			}
			else {
				return find_insert_pos(val);
			}
		}

		if (m_compare(val, *hint)) {
			if (hint.m_ptr == leftmost_ptr()) {
				return std::make_pair(leftmost_ptr(), insert_pos::LEFT);
			}
			iterator prev_ = hint;
			--prev_;
			if (m_compare(*prev_, val)) {
				if (prev_.m_ptr->m_right == nullptr) {
					return std::make_pair(prev_.m_ptr, insert_pos::RIGHT);
				}
				else {
					return std::make_pair(hint.m_ptr, insert_pos::LEFT);
				}
			}
		}

		return find_insert_pos(val);
	}

	std::pair<node_base*, insert_pos> find_insert_pos(const value_type& val)
	{
		if (empty()) {
			return empty_pos();
		}

		node_base* curr = root_ptr();
		node_base* prev_;
		bool cmp = true;
		while (curr != nullptr) {
			prev_ = curr;
			if ((cmp = m_compare(val, data(curr)))) {
				curr = left(curr);
			}
			else {
				curr = right(curr);
			}
		}

		//check if already present

		node_base* to_check = prev_;
		if (cmp) {
			if (prev_ == leftmost_ptr()) {
				return std::make_pair(prev_, insert_pos::LEFT);
			}
			to_check = prev(to_check);
		}
		if (m_compare(data(to_check), val)) {
			return std::make_pair(prev_, cmp ? insert_pos::LEFT : insert_pos::RIGHT);
		}
		else {
			return std::make_pair(prev_, insert_pos::DUPLICATE);
		}
	}

	template<typename Arg>
	node* create_node(Arg&& elem)
	{
		return new node(m_urng(), std::forward<Arg>(elem));
	}

	template<typename ... Args>
	node* create_node(Args&&... args)
	{
		return new node(m_urng(), std::forward<Args>(args)...);
	}

	void insert_rebalance(node_base* parent_, bool left, node* nd)
	{
		nd->m_parent = parent_;
		if (left) {
			parent_->m_left = nd;
			if (parent_ == leftmost_ptr()) {
				leftmost_ptr() = nd;
			}
			else if (parent_ == &m_header) { //first element always inserted to the left
				parent_->m_parent = nd;
				parent_->m_right = nd;
			}
		}
		else {
			parent_->m_right = nd;
			if (parent_ == rightmost_ptr()) {
				rightmost_ptr() = nd;
			}
		}
		while (nd != root_ptr() && parent(nd)->m_priority < nd->m_priority) {
			if (nd->m_parent->m_left == nd) {
				rotate_right(nd);
			}
			else {
				rotate_left(nd);
			}
		}
	}

	void erase_rebalance(node* nd)
	{
		auto drop = [this, nd]() -> bool {
			if (nd->m_left != nullptr) {
				if (nd->m_right != nullptr) {
					if (left(nd)->m_priority > right(nd)->m_priority) {
						rotate_right(left(nd));
					}
					else {
						rotate_left(right(nd));
					}
				}
				else {
					rotate_right(left(nd));
				}
			}
			else if (nd->m_right != nullptr) {
				rotate_left(right(nd));
			}
			else {
				return false;
			}
			return true;
		};

		if (nd == root_ptr()) {
			if (drop()) {
				root_ptr() = nd->m_parent;
			}
		}

		while (drop()) {
		}

		if (nd == root_ptr()) {
			fix_empty();
		}
		else {
			if (nd->m_parent->m_left == nd) {
				nd->m_parent->m_left = nullptr;
			}
			else {
				nd->m_parent->m_right = nullptr;
			}
			if (nd == leftmost_ptr()) {
				leftmost_ptr() = find_lowest(root_ptr());
			}
			if (nd == rightmost_ptr()) {
				rightmost_ptr() = find_highest(root_ptr());
			}
		}
		delete nd;
	}

	void rotate_right(node* pivot)
	{
		node* parent_ = parent(pivot);

		parent_->m_left = pivot->m_right;
		if (parent_->m_left != nullptr) {
			parent_->m_left->m_parent = parent_;
		}

		pivot->m_parent = parent_->m_parent;
		if (parent_ == root_ptr()) {
			root_ptr() = pivot;
		}
		else {
			if (pivot->m_parent->m_left == parent_) {
				pivot->m_parent->m_left = pivot;
			}
			else {
				pivot->m_parent->m_right = pivot;
			}
		}

		pivot->m_right = parent_;
		parent_->m_parent = pivot;
	}

	void rotate_left(node* pivot)
	{
		node* parent_ = parent(pivot);

		parent_->m_right = pivot->m_left;
		if (parent_->m_right != nullptr) {
			parent_->m_right->m_parent = parent_;
		}

		pivot->m_parent = parent_->m_parent;
		if (parent_ == root_ptr()) {
			root_ptr() = pivot;
		}
		else {
			if (pivot->m_parent->m_left == parent_) {
				pivot->m_parent->m_left = pivot;
			}
			else {
				pivot->m_parent->m_right = pivot;
			}
		}

		pivot->m_left = parent_;
		parent_->m_parent = pivot;
	}

	const node_base* find_impl(const value_type& elem) const
	{
		const node_base* nd = lower_bound_impl(elem);
		if (nd == end_ptr() || m_compare(elem, data(nd))) {
			return end_ptr();
		}
		else {
			return nd;
		}
	}

	const node_base* lower_bound_impl(const value_type& elem) const
	{
		const node_base* x = root_ptr();
		const node_base* y = end_ptr();
		while (x != nullptr) {
			if (!m_compare(data(x), elem)) {
				y = x;
				x = x->m_left;
			}
			else {
				x = right(x);
			}
		}
		return y;
	}

	const node_base* upper_bound_impl(const value_type& elem) const
	{
		const node_base* x = root_ptr();
		const node_base* y = end_ptr();
		while (x != nullptr) {
			if (m_compare(elem, data(x))) {
				y = x;
				x = left(x);
			}
			else {
				x = right(x);
			}
		}
		return y;
	}

	static void erase_no_rebalance(node* nd)
	{
		node* tmp;
		while (nd != nullptr) {
			erase_no_rebalance(right(nd));
			tmp = left(nd);
			delete nd;
			nd = tmp;
		}
	}

	static node* copy(const node* other, node_base* parent)
	{
		node* top = new node(*other); //may throw
		top->m_parent = parent;

		try {
			if (other->m_right != nullptr) {
				top->m_right = copy(right(other), top); //may throw
			}

			node* prev = top;
			other = left(other);
			while (other != nullptr) {
				node* curr = new node(*other); //may throw
				curr->m_parent = prev;
				prev->m_left = curr;

				if (other->m_right != nullptr) {
					curr->m_right = copy(right(other), curr); //may throw
				}

				other = left(other);
				prev = curr;
			}
			return top;
		}
		catch (...) {
			erase_no_rebalance(top);
			throw;
		}
	}
};

#endif /* TREAP_HPP_ */

#pragma once
#include <array>
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include "Common.hpp"
#include "Math/Circle.hpp"
#include "Math/Vector.hpp"
#include "Math/Rectangle.hpp"

// So that started as a simple QuadTree for my game, i've tried extending it to 2^N-Tree
// with compile time template so it's a little clunky.

// S is the bucket size,
// T is the type holded by the tree
// Body is the type returned by the Predicat, by default it is a point.
// Body needs to match an overload for Rectangle2<t>::in.
// F is the float type it must be a group and be continuous.
template<
	size_t S,
	typename T,
	typename Body = T,
	typename F = double,
	size_t D = 2
>
class QuadTree {
public:
	static constexpr size_t pow_d = xstd::constexpr_pow2(D);

	using ThisQuadTree = QuadTree<S, T, Body, F, D>;
	using Predicat = std::function<Body(T)>;
	using _T = T;
	static constexpr size_t bucket_size = S;

	using vector = std::vector<T>;
	using rec = Rectangle_t<D, F>;


	QuadTree() = default;

	QuadTree(rec scope, Predicat pred = [](auto x) { return x; })
		: scope_(scope), pred(pred) {}
	QuadTree(const ThisQuadTree& that) {
		this->operator=(that);
	}
	QuadTree(ThisQuadTree&& that) {
		this->operator=(that);
	}
	~QuadTree() {
		if (!leaf()) for (auto& x : cells) delete x;
	}

	ThisQuadTree& operator=(const ThisQuadTree& that) {
		if (cells[0]) for (auto& x : cells) delete x;
		scope_ = that.scope_;

		if (that.leaf()) {
			size_ = that.size_;
			std::memcpy(items.data(), that.items.data(), sizeof(T) * that.size_);
			return *this;
		}

		for (size_t i = 0; i < pow_d; ++i)
			cells[i] = new ThisQuadTree(that.cells[i]->scope(), pred);
		return *this;
	}
	ThisQuadTree& operator=(ThisQuadTree&& that) {
		if (this == &that) return;

		if (cells[0]) for (auto& x : cells) delete x;
		scope_ = that.scope_;

		if (that.leaf()) {
			size_ = that.size_;
			std::memcpy(items.data(), that.items.data(), sizeof(T) * that.size_);
			return *this;
		}

		for (size_t i = 0; i < pow_d; ++i) cells[i] = that.cells[i];

		for (size_t i = 0; i < pow_d; ++i) that.cells[i] = nullptr; // just to be extra sure...
		return *this;
	}


	void add(T t) {
		if (!scope_.in(pred(t))) return;

		if (leaf()) {
			if (size_ != S) {
				items[size_] = t;
				size_++;
				return;
			}
			size_ = 0;

			auto new_cells = scope_.divide();
			for (size_t i = 0; i < pow_d; ++i)
				cells[i] = new ThisQuadTree(new_cells[i], pred);

			for (size_t i = 0u; i < S; ++i)
				for (size_t j = 0; j < pow_d; ++j) cells[j]->add(items[i]);

			add(t);
			return;
		}

		for (size_t i = 0; i < pow_d; ++i) cells[i]->add(t);
	}

	ThisQuadTree& getLeafAt(T p) {
		if (leaf()) {
			return *this;
		}

		for (auto& x : cells) if (x->scope_.in(p)) return x->getLeafAt(p);

		//you shoud _not_ make it here
		_ASSERT(0);

		return *this;
	}

	void clear() noexcept {
		if (leaf()) {
			items = {};
			return;
		}

		for (auto& x : cells) {
			delete x;
			x = nullptr;
		}
		size_ = 0u;
	}

	vector get() const {
		if (leaf())
			return vector(&items[0], &items[0] + size_);

		vector results(sizeElems());

		size_t offset{ 0 };
		for (auto& x : cells) {
			results.insert(
				std::begin(results) + offset,
				std::begin(x->get()),
				std::end(x->get())
			);
			offset += x->get().size();
		}
		return results;
	}

	void noAllocQueryCircle(
		const Circle<D, F>& c, vector& result, std::vector<const ThisQuadTree*>& open
	) const {
		auto point = pred(c.c);
		

		if (is_fully_in(scope_, c)) {
			result.insert(std::end(result), std::begin(items), std::begin(items) + size_);
		}

		open.push_back(this);

		while (!open.empty()) {
			const ThisQuadTree* q = open.front();
			open.front() = open.back();
			open.pop_back();

			if (!q->leaf()) {
				for (const auto& x : q->get_cells()) {
					if (is_fully_in(x->scope(), c)) {
						const auto& res = x->get();
						result.insert(std::end(result), std::begin(res), std::end(res));
					}
					else if (is_in(x->scope(), c)) {
						open.push_back(x);
					}
				}
			}
			else if (is_in(q->scope(), c)) {
				const auto& g = q->array();
				result.insert(std::end(result), &g[0], &g[0] + q->size());
			}
		}
	}

	vector queryCircle(T p, double r) const {
		vector re;
		std::vector<const ThisQuadTree*> op;
		op.reserve(nNodes());
		re.reserve(S * nLeafs());
		noAllocQueryCircle({ p, r }, re, op);
		return re;
	}

	size_t sizeElems() const {
		if (!cells[0]) return size_;
		size_t sum{ 0 };
		for (auto& x : cells) sum += x->sizeElems();
		return sum;
	}

	std::array<T, S> array() const {
		return items;
	}

	size_t size() const {
		return size_;
	}

	bool leaf() const {
		return !cells[0];
	}

	rec scope() const {
		return scope_;
	}

	size_t nLeafs() const {
		if (!cells[0]) return 1;
		size_t n{ 0 };

		for (auto& x : cells) n += x->nLeafs();
		return n;
	}

	size_t nNodes() const {
		size_t n{ 1 };
		if (!cells[0]) return n;
		for (auto& x : cells) n += x->nNodes();
		return n;
	}

	size_t maxDepth() const {
		if (!cells[0]) return 0;

		size_t child_d = cells[0]->maxDepth();

		for (auto& x : cells) child_d = std::max(child_d, x->maxDepth());

		return 1 + child_d;
	}

	const ThisQuadTree* get_cell(size_t i) const noexcept { return cells[i]; }
	ThisQuadTree* get_cell(size_t i) noexcept { return cells[i]; }
	const std::array<ThisQuadTree*, pow_d>& get_cells() const noexcept { return cells; }
	std::array<ThisQuadTree*, pow_d>& get_cells() noexcept { return cells; }

	void print(std::string pre = "") const {
		printf("%s %u\n", pre.c_str(), sizeElems());
		if (!leaf()) for (auto& x : cells) x->print(pre + " ");
	}

private:

	std::array<ThisQuadTree*, pow_d> cells;

	std::array<T, S> items;
	size_t size_{ 0u };

	rec scope_{};

	Predicat pred{ [](auto x) { return x; } };
};
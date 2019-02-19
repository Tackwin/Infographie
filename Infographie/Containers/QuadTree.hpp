#pragma once
#include <array>
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>
#include "Math/Circle.hpp"
#include "Math/Vector.hpp"
#include "Math/Rectangle.hpp"

// S is the bucket size,
// T is the type holded by the tree
// Body is the type returned by the Predicat, by default it is a point.
// Body needs to match an overload for Rectangle2<t>::in.
// F is the float type it must be a group and be continuous.
template<
	size_t S,
	typename T,
	typename Body = T,
	typename F = double
>
class QuadTree {
public:
	using ThisQuadTree = QuadTree<S, T, Body, F>;
	using Predicat = std::function<Body(T)>;
	using _T = T;
	static constexpr size_t bucket_size = S;

	using vector = std::vector<T>;
	using rec = Rectangle<F>;


	QuadTree() = default;

	QuadTree(rec scope, Predicat pred = [](auto x) { return x; })
		: scope_(scope), pred(pred) {}
	QuadTree(const ThisQuadTree& that) {
		this->operator=(that);
	}
	QuadTree(const ThisQuadTree&& that) {
		this->operator=(that);
	}
	~QuadTree() {
		if (!leaf()) {
			delete a_;
			delete b_;
			delete c_;
			delete d_;
		}
	}

	ThisQuadTree& operator=(const ThisQuadTree& that) {
		if (a_) {
			delete a_;
			delete b_;
			delete c_;
			delete d_;
		}
		scope_ = that.scope_;

		if (that.leaf()) {
			size_ = that.size_;
			std::memcpy(items, that.items, sizeof(T) * that.size_);
			return *this;
		}

		a_ = new ThisQuadTree(that.a_, pred);
		b_ = new ThisQuadTree(that.b_, pred);
		c_ = new ThisQuadTree(that.c_, pred);
		d_ = new ThisQuadTree(that.d_, pred);
		return *this;
	}
	ThisQuadTree& operator=(const ThisQuadTree&& that) {
		if (a_) {
			delete a_;
			delete b_;
			delete c_;
			delete d_;
		}
		scope_ = that.scope_;

		if (that.leaf()) {
			size_ = that.size_;
			std::memcpy(items, that.items, sizeof(T) * that.size_);
			return *this;
		}

		a_ = that.a_;
		b_ = that.b_;
		c_ = that.c_;
		d_ = that.d_;

		that.a_ = nullptr; // just to be extra sure...
		that.b_ = nullptr; // just to be extra sure...
		that.c_ = nullptr; // just to be extra sure...
		that.d_ = nullptr; // just to be extra sure...
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

			auto[a, b, c, d] = scope_.divide();
			a_ = new ThisQuadTree(a, pred);
			b_ = new ThisQuadTree(b, pred);
			c_ = new ThisQuadTree(c, pred);
			d_ = new ThisQuadTree(d, pred);

			for (size_t i = 0u; i < S; ++i) {
				a_->add(items[i]);
				b_->add(items[i]);
				c_->add(items[i]);
				d_->add(items[i]);
			}

			add(t);
			return;
		}

		a_->add(t);
		b_->add(t);
		c_->add(t);
		d_->add(t);
	}

	ThisQuadTree& getLeafAt(T p) {
		if (leaf()) {
			return *this;
		}

		if (a_->scope_().in(p)) return a_->getLeafAt(p);
		if (b_->scope_().in(p)) return b_->getLeafAt(p);
		if (c_->scope_().in(p)) return b_->getLeafAt(p);
		if (d_->scope_().in(p)) return d_->getLeafAt(p);

		//you shoud _not_ make it here
		_ASSERT(0);

		return *this;
	}

	void clear() noexcept {
		if (leaf()) {
			items = {};
			return;
		}

		delete a_;
		delete b_;
		delete c_;
		delete d_;

		a_ = b_ = c_ = d_ = nullptr;
		size_ = 0u;
	}

	vector get() const {
		if (leaf())
			return vector(&items[0], &items[0] + size_);

		vector results(sizeElems());

		auto A = a_->get();
		results.insert(std::begin(results), std::begin(A), std::end(A));

		auto B = b_->get();
		results.insert(std::begin(results) + A.size(), std::begin(B), std::end(B));

		auto C = c_->get();
		results.insert(
			std::begin(results) + A.size() + B.size(), std::begin(C), std::end(C)
		);

		auto D = d_->get();
		results.insert(
			std::begin(results) + A.size() + B.size() + C.size(),
			std::begin(D), std::end(D)
		);

		return results;
	}

	void noAllocQueryCircle(
		const Circle<F>& c, vector& result, std::vector<const ThisQuadTree*>& open
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
				if (is_fully_in(q->a()->scope(), c)) {
					const auto& res = q->a()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (is_in(q->a()->scope(), c)) {
					open.push_back(q->a());
				}
				if (is_fully_in(q->b()->scope(), c)) {
					const auto& res = q->b()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (is_in(q->b()->scope(), c)) {
					open.push_back(q->b());
				}
				if (is_fully_in(q->c()->scope(), c)) {
					const auto& res = q->c()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (is_in(q->c()->scope(), c)) {
					open.push_back(q->c());
				}
				if (is_fully_in(q->d()->scope(), c)) {
					const auto& res = q->d()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (is_in(q->d()->scope(), c)) {
					open.push_back(q->d());
				}
			}
			else if (is_in(q->scope(), c)) {
				const auto& g = q->array();
				result.insert(std::end(result), &g[0], &g[0] + q->size());
			}
		}
	}

	void queryCircle(T p, double r) const {
		vector re;
		std::vector<const ThisQuadTree*> op;
		op.reserve(nNodes());
		re.reserve(S * nLeafs());
		noAllocQueryCircle(p, r, re, op);
		return re;
	}

	size_t sizeElems() const {
		if (!a_) return size_;
		return a_->sizeElems() + b_->sizeElems() + c_->sizeElems() + d_->sizeElems();
	}

	std::array<T, S> array() const {
		return items;
	}

	size_t size() const {
		return size_;
	}

	bool leaf() const {
		return !a_;
	}

	rec scope() const {
		return scope_;
	}

	size_t nLeafs() const {
		if (!a_) return 1;
		return a_->nLeafs() + b_->nLeafs() + c_->nLeafs() + d_->nLeafs();
	}

	size_t nNodes() const {
		if (!a_) return 1;
		return 1 + a_->nNodes() + b_->nNodes() + c_->nNodes() + d_->nNodes();
	}

	size_t maxDepth() const {
		if (!a_) return 0;

		return 1 + std::max({
			a_->maxDepth(),
			b_->maxDepth(),
			c_->maxDepth(),
			d_->maxDepth()
			});
	}

	ThisQuadTree* a() const { return a_; };
	ThisQuadTree* b() const { return b_; };
	ThisQuadTree* c() const { return c_; };
	ThisQuadTree* d() const { return d_; };

	void print(std::string pre = "") const {
		printf("%s %u\n", pre.c_str(), sizeElems());
		if (!leaf()) {
			a_->print(pre + "  ");
			b_->print(pre + "  ");
			c_->print(pre + "  ");
			d_->print(pre + "  ");
		}
	}

private:
	std::array<T, S> items;
	size_t size_{ 0u };

	rec scope_{ 0, 0, 0, 0 };

	Predicat pred{ [](auto x) { return x; } };

	ThisQuadTree* a_ = nullptr, *b_ = nullptr;
	ThisQuadTree* c_ = nullptr, *d_ = nullptr;
};
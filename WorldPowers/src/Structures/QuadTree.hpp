#pragma once
#include <array>
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>
#include "./../Math/Vector.hpp"
#include "./../Math/Rectangle.hpp"

template<size_t S, typename T>
class QuadTree {
public:
	using _T = T;
	static constexpr size_t _S = S;

	using vector = std::vector<Vector<2U, T>>;
	using point = Vector<2U, T>;
	using rec = Rectangle<T>;

	QuadTree() = default;

	QuadTree(rec scope) :
		_scope(scope)
	{

	}
	QuadTree(const QuadTree& that) {
		this->operator=(that);
	}
	QuadTree(const QuadTree&& that) {
		this->operator=(that);
	}
	~QuadTree() {
		delete _a;
		delete _b;
		delete _c;
		delete _d;
	}

	QuadTree<S, T>& operator=(const QuadTree& that) {
		if (_a) {
			delete _a;
			delete _b;
			delete _c;
			delete _d;
		}
		_scope = that._scope;

		if (that.leaf()) {
			_size = that._size;
			std::memcpy(_items, that._items, sizeof(T) * that._size);
			return *this;
		}

		_a = new QuadTree(that._a);
		_b = new QuadTree(that._b);
		_c = new QuadTree(that._c);
		_d = new QuadTree(that._d);
		return *this;
	}
	QuadTree<S, T>& operator=(const QuadTree&& that) {
		if (_a) {
			delete _a;
			delete _b;
			delete _c;
			delete _d;
		}
		_scope = that._scope;

		if (that.leaf()) {
			_size = that._size;
			std::memcpy(_items, that._items, sizeof(T) * that._size);
			return *this;
		}

		_a = that._a;
		_b = that._b;
		_c = that._c;
		_d = that._d;

		that._a = nullptr; // just to be extra sure...
		that._b = nullptr; // just to be extra sure...
		that._c = nullptr; // just to be extra sure...
		that._d = nullptr; // just to be extra sure...
		return *this;
	}


	void add(point t) {
		if (!_scope.contains(t)) return;

		if (leaf()) {
			if (_size != S) {
				_items[_size] = t;
				_size++;
				return;
			}
			_size = 0;

			auto[a, b, c, d] = _scope.divide();
			_a = new QuadTree(a);
			_b = new QuadTree(b);
			_c = new QuadTree(c);
			_d = new QuadTree(d);

			for (size_t i = 0u; i < S; ++i) {
				_a->add(_items[i]);
				_b->add(_items[i]);
				_c->add(_items[i]);
				_d->add(_items[i]);
			}

			add(t);
			return;
		}

		_a->add(t);
		_b->add(t);
		_c->add(t);
		_d->add(t);
	}

	QuadTree<S, T>& getLeafAt(point p) {
		if (leaf()) {
			return *this;
		}

		if (_a->scope().contains(p)) return _a->getLeafAt(p);
		if (_b->scope().contains(p)) return _b->getLeafAt(p);
		if (_c->scope().contains(p)) return _b->getLeafAt(p);
		if (_d->scope().contains(p)) return _d->getLeafAt(p);

		//you shoud _not_ make it here
		_ASSERT(0);

		return *this;
	}

	vector get() const {
		if (leaf())
			return vector(&_items[0], &_items[0] + _size);

		std::vector<Vector<2U, T>> results(sizeElems());

		auto A = _a->get();
		results.insert(std::begin(results), std::begin(A), std::end(A));

		auto B = _b->get();
		results.insert(std::begin(results) + A.size(), std::begin(B), std::end(B));

		auto C = _c->get();
		results.insert(
			std::begin(results) + A.size() + B.size(), std::begin(C), std::end(C)
		);

		auto D = _d->get();
		results.insert(
			std::begin(results) + A.size() + B.size() + C.size(),
			std::begin(D), std::end(D)
		);

		return results;
	}

	__declspec(noinline) void noAllocQueryCircle(
		point p, double r, vector& result, std::vector<const QuadTree<S, T>*>& open
	) const {
		if (scope().containedInCircle(p, r)) {
			result.insert(std::end(result), std::begin(_items), std::begin(_items) + _size);
		}

		open.push_back(this);

		while (!open.empty()) {
			const QuadTree<S, T>* q = open.front();
			open.front() = open.back();
			open.pop_back();

			if (!q->leaf()) {
				if (q->a()->scope().containedInCircle(p, r)) {
					const auto& res = q->a()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (q->a()->scope().circleIntersect(p, r)) {
					open.push_back(q->a());
				}
				if (q->b()->scope().containedInCircle(p, r)) {
					const auto& res = q->b()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (q->b()->scope().circleIntersect(p, r)) {
					open.push_back(q->b());
				}
				if (q->c()->scope().containedInCircle(p, r)) {
					const auto& res = q->c()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (q->c()->scope().circleIntersect(p, r)) {
					open.push_back(q->c());
				}
				if (q->d()->scope().containedInCircle(p, r)) {
					const auto& res = q->d()->get();
					result.insert(std::end(result), std::begin(res), std::end(res));
				}
				else if (q->d()->scope().circleIntersect(p, r)) {
					open.push_back(q->d());
				}
			}
			else if (q->scope().circleIntersect(p, r)) {
				const auto& g = q->array();
				result.insert(std::end(result), &g[0], &g[0] + q->size());
			}
		}
	}

	void queryCircle(point p, double r) const {
		vector re;
		std::vector<const QuadTree<S, T>*> op;
		op.reserve(nNodes());
		re.reserve(S * nLeafs());
		noAllocQueryCircle(p, r, re, op);
		return re;
	}

	size_t sizeElems() const {
		if (!_a) return _size;
		return _a->sizeElems() + _b->sizeElems() + _c->sizeElems() + _d->sizeElems();
	}

	std::array<point, S> array() const {
		return _items;
	}

	size_t size() const {
		return _size;
	}

	bool leaf() const {
		return !_a;
	}

	rec scope() const {
		return _scope;
	}

	size_t nLeafs() const {
		if (!_a) return 1;
		return _a->nLeafs() + _b->nLeafs() + _c->nLeafs() + _d->nLeafs();
	}

	size_t nNodes() const {
		if (!_a) return 1;
		return 1 + _a->nNodes() + _b->nNodes() + _c->nNodes() + _d->nNodes();
	}

	size_t maxDepth() const {
		if (!_a) return 0;

		return 1 + std::max({
			_a->maxDepth(),
			_b->maxDepth(),
			_c->maxDepth(),
			_d->maxDepth()
		});
	}

	QuadTree<S, T>* a() const { return _a; };
	QuadTree<S, T>* b() const { return _b; };
	QuadTree<S, T>* c() const { return _c; };
	QuadTree<S, T>* d() const { return _d; };

	void print(std::string pre = "") const {
		printf("%s %u\n", pre.c_str(), sizeElems());
		if (!leaf()) {
			_a->print(pre + "  ");
			_b->print(pre + "  ");
			_c->print(pre + "  ");
			_d->print(pre + "  ");
		}
	}

private:
	std::array<point, S> _items;
	size_t _size = 0u;

	rec _scope;

	QuadTree* _a = nullptr, *_b = nullptr;
	QuadTree* _c = nullptr, *_d = nullptr;
};
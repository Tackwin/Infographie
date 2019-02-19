#pragma once
#include <unordered_set>
#include "Common.hpp"
#include "Order/OrderedPair.hpp"
#include "Utils/UUID.hpp"

template<typename T>
struct Node {
	std::unordered_set<UUID> edges;
	UUID me;
	T data;
};

template<typename T>
using Graph = std::unordered_map<UUID, Node<T>>;

template<typename T> void link_nodes(Node<T>& A, Node<T>& B) noexcept {
	A.edges.emplace(B.me);
	B.edges.emplace(A.me);
}
template<typename T> void graph_unlink_nodes(Node<T>& A, Node<T>& B) noexcept {
	auto it = A.edges.find(B.me);
	auto jt = B.edges.find(A.me);

	assert(it != std::end(A.edges));
	assert(jt != std::end(B.edges));

	A.edges.erase(B.me);
	B.edges.erase(A.me);
}
template<typename T> bool is_linked(const Node<T>& A, const Node<T>& B) noexcept {
	return A.edges.count(B.me) != 0 || B.edges.count(A.me) != 0;
}
template<typename T>
bool is_remotely_linked(const Graph<T>& graph, const Node<T>& A, const Node<T>& B) noexcept {
	bool connected = false;
	walk_connexe_coponent(graph, A.me, [&](auto x) { if (x == B.me) connected = true; });
	if (connected) return true;
	walk_connexe_coponent(graph, B.me, [&](auto x) { if (x == A.me) connected = true; });
	return connected;
}

template<typename T>
void add_node(Graph<T>& graph, const T& data, std::unordered_set<UUID> neighboor) noexcept
{
	Node<T> n;
	auto id = n.me;
	n.data = data;
	for (auto& x : neighboor) {
		if (graph.count(x)) link_nodes(n, graph.at(x));
	}
	graph[id] = std::move(n);
}

template<typename T>
void walk_connexe_coponent(
	const Graph<T>& graph, UUID origin, std::function<void(UUID)> pred
) noexcept
{
	std::unordered_set<UUID> close;
	std::vector<UUID> open;
	open.push_back(origin);
	while (!open.empty()) {
		auto n = open.back();
		open.pop_back();

		if (close.count(n) != 0) continue;
		pred(n);
		close.emplace(n);

		if (graph.count(n) == 0) continue;
		auto& node = graph.at(n);

		for (auto x : node.edges) {
			open.push_back(x);
		}
	}
}

template<typename T>
void graph_min_spanning_tree(
	Graph<T>& graph,
	std::function<float(const Node<T>&, const Node<T>&)> cost,
	bool cache_cost = false
) noexcept {
	std::unordered_set<UUID> Q;
	std::unordered_map<UUID, std::optional<UUID>> E;
	std::unordered_map<UUID, float> C;
	std::unordered_map<OrderedPair<UUID>, float> cost_cache;
	for (auto&[id, _] : graph) {
		Q.insert(id);
		C.insert({ id, FLT_MAX });
		E.insert({ id, std::nullopt });
	}

	C.at(std::begin(graph)->first) = FLT_MAX / 2.f;

	auto min_key = [&]{
		float min = FLT_MAX;
		UUID min_index{ UUID::zero() };

		for (auto&[id, w] : C) {
			if (Q.count(id) && C.at(id) < min) {
				min = C.at(id);
				min_index = id;
			}
		}
		return min_index;
	};

	while (!Q.empty()) {
		auto v = min_key();

		Q.erase(v);
		if (E.at(v)) {
			link_nodes(graph.at(v), graph.at(*E.at(v)));
		}

		if (cache_cost) {
			for (auto&[id, w] : graph) {
				if (!Q.count(id)) continue;
				auto p = OrderedPair<UUID>(id, v);

				if (!cost_cache.count(p)) {
					cost_cache[p] = cost(graph.at(v), w);
				}
				auto candidate = cost_cache.at(p);
				if (candidate < C.at(id)) {
					C.at(id) = candidate;
					E.at(id) = v;
				}
			}
		}
		else {
			for (auto&[id, w] : graph) {
				if (!Q.count(id)) continue;

				auto candidate = cost(graph.at(v), w);
				if (candidate < C.at(id)) {
					C.at(id) = candidate;
					E.at(id) = v;
				}
			}
		}
	}
}

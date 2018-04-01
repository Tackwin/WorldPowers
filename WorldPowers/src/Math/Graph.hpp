#pragma once

#include <unordered_map>
#include <unordered_set>

struct Graph {

	std::unordered_map<size_t, std::unordered_set<size_t>> adjacents;

	decltype(adjacents)::mapped_type& operator[](const decltype(adjacents)::key_type& k) {
		return adjacents[k];
	}

};
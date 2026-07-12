#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

namespace Hazel {
namespace Utils {
class Graph {
private:
    // vertex -> set of vertices it has an outgoing edge to.
    std::unordered_map<std::string, std::unordered_set<std::string>> adjacency;

    bool has_cycle_from(std::string vertex, std::unordered_map<std::string, std::int64_t>& color) const;
    std::vector<std::string> find_cycle_from(std::string vertex, std::unordered_map<std::string, std::int64_t>& color,std::vector<std::string>& path) const;
    std::int64_t longest_path_from(std::string vertex, std::unordered_map<std::string, std::int64_t>& memo) const;

public:
    Graph() = default;

    // Inserts a new vertex. Returns false if the vertex already exists
    // (nothing changes in that case), true if it was newly added.
    bool insert_vertex(std::string vertex);
    bool create_edge(std::string from, std::string to);
    bool has_vertex(std::string vertex) const;
    bool has_edge(std::string from, std::string to) const;
    bool is_acyclic() const;
    std::vector<std::string> find_cycle() const;
    std::int64_t max_path_length() const;
    std::size_t vertex_count() const;
    std::size_t edge_count() const;
    std::vector<std::string> vertices() const;

};
}
}
#include "utils/graph.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

namespace Hazel {
namespace Utils {
bool Graph::insert_vertex(std::string vertex) {
    auto result = this->adjacency.emplace(vertex, std::unordered_set<std::string>{});
    return result.second;
}
bool Graph::create_edge(std::string from, std::string to) {
    insert_vertex(from);
    insert_vertex(to);
    return this->adjacency[from].insert(to).second;
}
bool Graph::has_vertex(std::string vertex) const {
    return this->adjacency.find(vertex) != this->adjacency.end();
}
bool Graph::has_edge(std::string from, std::string to) const {
    auto it = this->adjacency.find(from);
    if (it == this->adjacency.end()) {
        return false;
    }
    return it->second.find(to) != it->second.end();
}
bool Graph::is_acyclic() const {
    // color: 0 = unvisited (white), 1 = in progress (gray), 2 = done (black)
    std::unordered_map<std::string, std::int64_t> color;
    color.reserve(this->adjacency.size());
    for (const auto& entry : this->adjacency) {
        color[entry.first] = 0;
    }
    for (const auto& entry : this->adjacency) {
        if (color[entry.first] == 0) {
            if (has_cycle_from(entry.first, color)) {
                return false;
            }
        }
    }
    return true;
}
std::vector<std::string> Graph::find_cycle() const {
    std::unordered_map<std::string, std::int64_t> color;
    color.reserve(this->adjacency.size());
    for (const auto& entry : this->adjacency) {
        color[entry.first] = 0;
    }
    std::vector<std::string> path;
    for (const auto& entry : this->adjacency) {
        if (color[entry.first] == 0) {
            std::vector<std::string> cycle = find_cycle_from(entry.first, color, path);
            if (!cycle.empty()) {
                return cycle;
            }
        }
    }
    return {};
}
std::int64_t Graph::max_path_length() const {
    //Length of the longest path in graph. Returns -1 if the graph has a cycle.
    if (!is_acyclic()) {
        return -1;
    }
    std::unordered_map<std::string, std::int64_t> memo;
    std::int64_t best = 0;
    for (const auto& entry : this->adjacency) {
        best = std::max(best, longest_path_from(entry.first, memo));
    }
    return best;
}
std::size_t Graph::vertex_count() const { 
    return this->adjacency.size(); 
}
std::size_t Graph::edge_count() const {
    std::size_t count = 0;
    for (const auto& entry : this->adjacency) {
        count += entry.second.size();
    }
    return count;
}
std::vector<std::string> Graph::vertices() const {
    std::vector<std::string> result;
    result.reserve(this->adjacency.size());
    for (const auto& entry : this->adjacency) {
        result.push_back(entry.first);
    }
    return result;
}


bool Graph::has_cycle_from(std::string vertex, std::unordered_map<std::string, std::int64_t>& color) const {
    color[vertex] = 1; // gray
    auto it = this->adjacency.find(vertex);
    if (it != this->adjacency.end()) {
        for (const auto& next : it->second) {
            if (color[next] == 1) {
                return true;                 // back edge -> cycle
            }
            if (color[next] == 0 && has_cycle_from(next, color)) {
                return true;
            }
        }
    }
    color[vertex] = 2; // black
    return false;
}
std::vector<std::string> Graph::find_cycle_from(std::string vertex, std::unordered_map<std::string, std::int64_t>& color, std::vector<std::string>& path) const {
    color[vertex] = 1; // gray
    path.push_back(vertex);
    auto it = this->adjacency.find(vertex);
    if (it != this->adjacency.end()) {
        for (const auto& next : it->second) {
            if (color[next] == 1) {
                auto pos = std::find(path.begin(), path.end(), next);
                std::vector<std::string> cycle(pos, path.end());
                cycle.push_back(next); // close the loop
                return cycle;
            }
            if (color[next] == 0) {
                std::vector<std::string> cycle = find_cycle_from(next, color, path);
                if (!cycle.empty()) {
                    return cycle;
                }
            }
        }
    }
    color[vertex] = 2; // black
    path.pop_back();
    return {};
}
std::int64_t Graph::longest_path_from(std::string vertex, std::unordered_map<std::string, std::int64_t>& memo) const {
    auto found = memo.find(vertex);
    if (found != memo.end()) {
        return found->second;
    }
    std::int64_t best = 0;
    auto it = this->adjacency.find(vertex);
    if (it != this->adjacency.end()) {
        for (const auto& next : it->second) {
            best = std::max(best, 1 + longest_path_from(next, memo));
        }
    }
    memo[vertex] = best;
    return best;
}
}
}


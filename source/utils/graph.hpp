#ifndef SOURCE_UTILS_GRAPH_HPP
#define SOURCE_UTILS_GRAPH_HPP

#include <algorithm>
#include <compare>
#include <deque>
#include <iterator>
#include <map>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/utils/macros/assert.hpp"

namespace utils
{
    template <typename T>
    class DirectedGraph
    {
      public:
        DirectedGraph() = default;

        auto add_node(const T& node) -> void;

        auto add_edge(const T& from, const T& to) -> void;

        auto check_for_cycle() const -> std::optional<std::string>;

        auto get_reachable_nodes(const std::vector<T>& initial) const -> std::vector<T>;

        auto operator<=>(const DirectedGraph<T>& other) const = default;

        auto data() const -> const std::unordered_map<T, std::unordered_set<T>>&
        {
            return edges;
        }

      private:
        enum class VisitStatus
        {
            UNVISITED,
            CURRENTLY_PROCESSED,
            FINISHED_PROCESSING
        };

        auto dfs(const T& node,
                 std::unordered_map<T, VisitStatus>& visit_status,
                 std::unordered_map<T, T>& parents) const -> std::optional<T>;

        static auto reconstruct_cycle(const T& start_node, const std::unordered_map<T, T>& parents) -> std::string;

        std::unordered_map<T, std::unordered_set<T>> edges;
    };
}

template <typename T>
auto utils::DirectedGraph<T>::add_node(const T& node) -> void
{
    if (!edges.contains(node))
    {
        edges[node] = {};
    }
}

template <typename T>
auto utils::DirectedGraph<T>::add_edge(const T& from, const T& to) -> void
{
    edges[from].insert(to);

    // Make sure `to` is in `edges` even if it does not have outgoing edges
    // to prevent out-of-bounds problems.
    add_node(to);
}

// Returns `true` if `node` is part of a cycle.
template <typename T>
auto utils::DirectedGraph<T>::dfs(const T& node,
                                  std::unordered_map<T, VisitStatus>& visit_status,
                                  std::unordered_map<T, T>& parents) const -> std::optional<T>
{
    ASSERT(!visit_status.contains(node)); // `node` must be unvisited.

    visit_status[node] = VisitStatus::CURRENTLY_PROCESSED;

    for (const auto& neighbor : edges.at(node))
    {
        const auto neighbor_visit_status =
            visit_status.contains(neighbor) ? visit_status.at(neighbor) : VisitStatus::UNVISITED;

        switch (neighbor_visit_status)
        {
        case VisitStatus::UNVISITED:
        {
            parents[neighbor] = node;

            const auto node_in_cycle  = dfs(neighbor, visit_status, parents);
            const auto cycle_detected = node_in_cycle.has_value();

            if (cycle_detected)
            {
                return *node_in_cycle;
            }

            break;
        }

        case VisitStatus::CURRENTLY_PROCESSED:
        {
            parents[neighbor] = node;

            return neighbor;
        }

        case VisitStatus::FINISHED_PROCESSING:
            break;
        }
    }

    visit_status[node] = VisitStatus::FINISHED_PROCESSING;

    return std::nullopt;
}

template <typename T>
auto utils::DirectedGraph<T>::reconstruct_cycle(const T& start_node,
                                                const std::unordered_map<T, T>& parents) -> std::string
{
    const auto start_position = parents.find(start_node);
    auto current              = start_position;
    std::vector<T> cycle;

    do
    {
        ASSERT(current != parents.end());

        const auto& [node, parent] = *current;
        cycle.push_back(node);
        current = parents.find(parent);

    } while (current != start_position);

    std::ranges::reverse(cycle);

    std::vector<T> sorted_cycle;

    const auto actual_cycle_size = cycle.size() + 1;
    sorted_cycle.reserve(actual_cycle_size); // The first node also appears at the end of the cycle.

    // For aesthetic and testing reasons, start the cycle from the lexicographically smallest node
    // while maintaining the same ordering.
    const auto min_node_position = std::ranges::min_element(cycle);
    sorted_cycle.insert(sorted_cycle.end(), min_node_position, cycle.end());
    sorted_cycle.insert(sorted_cycle.end(), cycle.begin(), min_node_position + 1);

    ASSERT(sorted_cycle.size() == actual_cycle_size);       // Actual cycle size is as expected
    ASSERT(sorted_cycle.size() == sorted_cycle.capacity()); // No redundant space in vector.

    const std::string_view DELIMITER = " -> ";

    return sorted_cycle                                  //
           | std::ranges::to<std::vector<std::string>>() //
           | std::views::join_with(DELIMITER)            //
           | std::ranges::to<std::string>();             //
}

template <typename T>
auto utils::DirectedGraph<T>::check_for_cycle() const -> std::optional<std::string>
{
    std::unordered_map<T, VisitStatus> visit_status;
    std::unordered_map<T, T> parents;

    for (const auto& node : std::views::keys(edges))
    {
        const auto node_already_visited = visit_status.contains(node);

        if (node_already_visited)
        {
            continue;
        }

        const auto node_inside_cycle = dfs(node, visit_status, parents);
        const auto cycle_detected    = node_inside_cycle.has_value();

        if (cycle_detected)
        {
            return reconstruct_cycle(*node_inside_cycle, parents);
        }
    }

    return std::nullopt;
}

template <typename T>
auto utils::DirectedGraph<T>::get_reachable_nodes(const std::vector<T>& initial) const -> std::vector<T>
{
    auto queue   = initial | std::ranges::to<std::deque>();
    auto reached = initial | std::ranges::to<std::unordered_set>();

    while (!queue.empty())
    {
        const auto& node = queue.front();

        if (!edges.contains(node))
        {
            queue.pop_front();
            continue;
        }

        for (const auto& neighbor : edges.at(node))
        {
            if (!reached.contains(neighbor))
            {
                queue.push_back(neighbor);
                reached.insert(neighbor);
            }
        }

        queue.pop_front();
    }

    return reached | std::ranges::to<std::vector>();
}

#endif // SOURCE_UTILS_GRAPH_HPP

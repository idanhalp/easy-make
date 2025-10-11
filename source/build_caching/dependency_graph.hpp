#ifndef SOURCE_BUILD_CACHING_DEPENDENCY_GRAPH_HPP
#define SOURCE_BUILD_CACHING_DEPENDENCY_GRAPH_HPP

#include <filesystem>
#include <vector>

#include "source/utils/graph.hpp"

namespace build_caching
{
    auto get_included_files(const std::filesystem::path& path) -> std::vector<std::filesystem::path>;

    auto get_dependency_graph(const std::filesystem::path& path_to_root,
                              const std::vector<std::filesystem::path>& code_files)
        -> utils::DirectedGraph<std::filesystem::path>;
}

#endif // SOURCE_BUILD_CACHING_DEPENDENCY_GRAPH_HPP

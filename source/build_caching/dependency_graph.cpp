#include "source/build_caching/dependency_graph.hpp"

#include <fstream>
#include <regex>
#include <vector>

auto build_caching::get_included_files(const std::filesystem::path& path) -> std::vector<std::filesystem::path>
{
    if (!std::filesystem::exists(path))
    {
        return {};
    }

    std::ifstream file(path);

    if (!file.is_open())
    {
        return {};
    }

    const std::regex include_regex(R"(^\s*#\s*include\s*\"([^\"]+)\")");
    std::string line;
    std::smatch match;
    std::vector<std::filesystem::path> includes;

    while (std::getline(file, line))
    {
        if (std::regex_search(line, match, include_regex))
        {
            includes.push_back(match[1].str());
        }
    }

    return includes;
}

auto build_caching::get_dependency_graph(const std::filesystem::path& path_to_root,
                                         const std::vector<std::filesystem::path>& code_files)
    -> utils::DirectedGraph<std::filesystem::path>
{
    utils::DirectedGraph<std::filesystem::path> graph;

    // There is an edge from file `f_1` to `f_2` if `f_2` includes `f_1`.
    // This way if `f_2` changes we can check for all the files that are
    // reachable from it and see that `f_1` also requires recompilation.

    for (const auto& file : code_files)
    {
        for (const auto& include : get_included_files(path_to_root / file))
        {
            // Currently, assume that `include` is a full path
            // from the project's root.
            // TODO: Handle include paths.

            graph.add_edge(include, file);
        }
    }

    return graph;
}

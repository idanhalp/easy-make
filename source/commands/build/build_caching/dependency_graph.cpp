#include "source/commands/build/build_caching/dependency_graph.hpp"

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

auto build_caching::resolve_include(const std::filesystem::path& include_path,
                                    const std::filesystem::path& including_file,
                                    const std::filesystem::path& path_to_root,
                                    const std::vector<std::string>& include_directories)
    -> std::optional<std::filesystem::path>
{
    const auto is_valid_file = [](const std::filesystem::path& p)
    { return std::filesystem::exists(p) && std::filesystem::is_regular_file(p); };

    const auto including_file_directory = including_file.parent_path();

    // Check relative to the including file's directory first.
    if (is_valid_file(path_to_root / including_file_directory / include_path))
    {
        return (including_file_directory / include_path).lexically_normal();
    }

    for (const auto& include_directory : include_directories)
    {
        if (is_valid_file(path_to_root / include_directory / include_path))
        {
            return (std::filesystem::path(include_directory) / include_path).lexically_normal();
        }
    }

    return std::nullopt; // Could not resolve include.
}

auto build_caching::get_dependency_graph(const std::filesystem::path& path_to_root,
                                         const std::vector<std::filesystem::path>& code_files,
                                         const std::vector<std::string>& include_directories)
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
            const auto actual_include = resolve_include(include, file, path_to_root, include_directories);
            const auto include_resolved_successfully = actual_include.has_value();

            if (include_resolved_successfully)
            {
                graph.add_edge(*actual_include, file);
            }
        }
    }

    return graph;
}

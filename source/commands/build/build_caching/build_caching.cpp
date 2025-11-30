#include "source/commands/build/build_caching/build_caching.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <iterator> // std::make_move_iterator
#include <ranges>
#include <set>
#include <stdexcept>

#include "third_party/nlohmann/json.hpp"

#include "source/commands/build/build_caching/dependency_graph.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/graph.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/utils.hpp"

using build_caching::DependencyGraph;

auto build_caching::hash_file_contents(const std::filesystem::path& path) -> std::uint64_t
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", path.native()));
    }

    const auto FNV_OFFSET_BASIS = 1'469'598'103'934'665'603ULL;
    const auto FNV_PRIME        = 1'099'511'628'211ULL;

    char byte;
    auto result = FNV_OFFSET_BASIS;

    while (file.get(byte))
    {
        result ^= static_cast<unsigned char>(byte);
        result *= FNV_PRIME;
    }

    return result;
}

auto build_caching::get_old_file_hashes(const std::string_view configuration_name,
                                        const std::filesystem::path& path_to_root)
    -> std::unordered_map<std::filesystem::path, std::uint64_t>
{
    const auto build_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::BUILD_DATA_FILE_NAME;

    if (!std::filesystem::exists(build_data_file_path))
    {
        return {};
    }

    std::ifstream data_file(build_data_file_path);

    if (!data_file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", build_data_file_path.native()));
    }

    nlohmann::json json;
    std::unordered_map<std::filesystem::path, std::uint64_t> old_hashes;

    data_file >> json;

    for (const auto& entry : json)
    {
        const auto path  = entry.at("path").get<std::filesystem::path>();
        const auto hash  = entry.at("hash").get<std::uint64_t>();
        old_hashes[path] = hash;
    }

    return old_hashes;
}

auto build_caching::get_old_dependency_graph(const std::string_view configuration_name,
                                             const std::filesystem::path& path_to_root) -> DependencyGraph
{
    const auto dependency_graph_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::DEPENDENCY_GRAPH_DATA_FILE_NAME;

    if (!std::filesystem::is_regular_file(dependency_graph_data_file_path))
    {
        return {};
    }

    auto data_file = std::ifstream(dependency_graph_data_file_path);

    if (!data_file.is_open())
    {
        return {};
    }

    nlohmann::json json;
    data_file >> json;
    ASSERT(json.is_object());

    DependencyGraph dependency_graph;

    for (const auto& [node, neighbors] : json.items())
    {
        ASSERT(!node.empty());
        ASSERT(neighbors.is_array());
        ASSERT(std::ranges::all_of(neighbors, &nlohmann::json::is_string));

        for (const auto& neighbor : neighbors)
        {
            dependency_graph.add_edge(node, neighbor);
        }
    }

    return dependency_graph;
}

auto build_caching::get_new_file_hashes(const std::vector<std::filesystem::path>& code_files)
    -> std::unordered_map<std::filesystem::path, std::uint64_t>
{
    std::unordered_map<std::filesystem::path, std::uint64_t> file_hashes;

    for (const auto& file : code_files)
    {
        file_hashes[file] = hash_file_contents(file);
    }

    return file_hashes;
}

auto build_caching::get_files_to_delete(const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
                                        const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
    -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> files_to_delete;

    for (const auto& file : std::views::keys(old_file_hashes))
    {
        if (!new_file_hashes.contains(file))
        {
            files_to_delete.push_back(file);
        }
    }

    return files_to_delete;
}

auto build_caching::get_changed_files(std::string_view configuration_name,
                                      const std::filesystem::path& path_to_root,
                                      const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
                                      const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
    -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> changed_files;

    for (const auto& [file, contents_hash] : new_file_hashes)
    {
        const auto object_file_path =
            path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / utils::get_object_file_name(file);

        const auto old_object_file_exists = old_file_hashes.contains(file) && std::filesystem::exists(object_file_path);
        const auto file_contents_changed =
            old_file_hashes.contains(file) && (old_file_hashes.at(file) != contents_hash);

        if ((utils::is_source_file(file) && !old_object_file_exists) || file_contents_changed)
        {
            changed_files.push_back(file);
        }
    }

    return changed_files;
}

static auto
get_files_affected_by_removal(const DependencyGraph& old_dependency_graph,
                              const DependencyGraph& new_dependency_graph) -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> affected_files;

    for (const auto& [file, dependent_files] : old_dependency_graph.data())
    {
        const auto file_was_removed = !new_dependency_graph.data().contains(file);

        if (file_was_removed)
        {
            affected_files.insert(affected_files.end(), dependent_files.begin(), dependent_files.end());
        }
    }

    return affected_files;
}

auto build_caching::get_files_to_compile(const DependencyGraph& old_dependency_graph,
                                         const DependencyGraph& new_dependency_graph,
                                         const std::vector<std::filesystem::path>& changed_files)
    -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> files_to_compile;

    auto files_affected_by_removal = get_files_affected_by_removal(old_dependency_graph, new_dependency_graph);
    files_to_compile.insert(files_to_compile.end(),
                            std::make_move_iterator(files_affected_by_removal.begin()),
                            std::make_move_iterator(files_affected_by_removal.end()));

    auto files_affected_by_change = new_dependency_graph.get_reachable_nodes(changed_files);
    files_to_compile.insert(files_to_compile.end(),
                            std::make_move_iterator(files_affected_by_change.begin()),
                            std::make_move_iterator(files_affected_by_change.end()));

    return files_to_compile                             //
           | std::views::filter(&utils::is_source_file) //
           | std::ranges::to<std::set>()                // Sort and remove duplicates.
           | std::ranges::to<std::vector>();            //
}

auto build_caching::write_to_build_data_file(const std::string_view configuration_name,
                                             const std::filesystem::path& path_to_root,
                                             const std::unordered_map<std::filesystem::path, std::uint64_t>& hashes)
    -> void
{
    std::filesystem::create_directories(path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name);

    const auto build_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::BUILD_DATA_FILE_NAME;

    std::ofstream data_file(build_data_file_path, std::ios::trunc); // Override file if it exists.

    if (!data_file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", build_data_file_path.native()));
    }

    nlohmann::json json;

    for (const auto& [path, hash] : hashes)
    {
        nlohmann::json hash_info = {
            {"path", path},
            {"hash", hash}
        };

        json.push_back(std::move(hash_info));
    }

    data_file << json.dump(); // Write to file.
}

auto build_caching::write_to_dependency_graph_data_file(const std::string_view configuration_name,
                                                        const std::filesystem::path& path_to_root,
                                                        const DependencyGraph& graph) -> void
{
    std::filesystem::create_directories(path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name);

    const auto data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::DEPENDENCY_GRAPH_DATA_FILE_NAME;

    auto data_file = std::ofstream(data_file_path, std::ios::trunc); // Override file if it exists.

    if (!data_file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", data_file_path.native()));
    }

    nlohmann::json json;

    for (const auto& [node, neighbors] : graph.data())
    {
        auto array = nlohmann::json::array();

        for (const auto& neighbor : neighbors)
        {
            array.push_back(neighbor);
        }

        json[node] = std::move(array);
    }

    if (json.empty()) // Calling `dump` on empty object actually writes `null`.
    {
        std::filesystem::remove(data_file_path);
    }
    else
    {
        data_file << json.dump(); // Write to file.
    }
}

static auto create_circular_dependencies_error_message(const std::string_view cycle) -> std::string
{
    return std::format("Error: Circular header dependency detected.\n\n"
                       "The following headers form a cycle:\n"
                       "{}\n\n"
                       "Consider restructuring the code to break the circular dependency.",
                       cycle);
}

auto build_caching::handle_build_caching(const Configuration& configuration,
                                         const std::filesystem::path& path_to_root,
                                         const std::vector<std::filesystem::path>& code_files)
    -> std::expected<Info, std::string>
{
    ASSERT(configuration.name.has_value());

    // Gather information about the previous state.
    const auto old_file_hashes      = get_old_file_hashes(*configuration.name, path_to_root);
    const auto old_dependency_graph = get_old_dependency_graph(*configuration.name, path_to_root);

    // Gather information about the current state.
    const auto new_file_hashes = get_new_file_hashes(code_files);
    const auto new_dependency_graph =
        get_dependency_graph(path_to_root, code_files, configuration.include_directories.value_or({}));

    // Make sure new state does not contain any circular includes.
    const auto cycle_in_new_dependency_graph = new_dependency_graph.check_for_cycle();
    const auto detected_cycle                = cycle_in_new_dependency_graph.has_value();

    if (detected_cycle)
    {
        return std::unexpected(create_circular_dependencies_error_message(*cycle_in_new_dependency_graph));
    }

    // Decide which files to delete and which file to recompile.
    const auto files_to_delete = get_files_to_delete(old_file_hashes, new_file_hashes);
    const auto changed_files   = get_changed_files(*configuration.name, path_to_root, old_file_hashes, new_file_hashes);
    const auto files_to_compile = get_files_to_compile(old_dependency_graph, new_dependency_graph, changed_files);

    write_to_build_data_file(*configuration.name, path_to_root, new_file_hashes);
    write_to_dependency_graph_data_file(*configuration.name, path_to_root, new_dependency_graph);

    return Info{
        .files_to_delete  = files_to_delete,
        .files_to_compile = files_to_compile,
    };
}

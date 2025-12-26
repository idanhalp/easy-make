#include "source/commands/build/build_caching/build_caching.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <iterator> // std::istreambuf_iterator, std::make_move_iterator
#include <print>
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

static const auto FNV_OFFSET_BASIS = 1'469'598'103'934'665'603ULL;

// Hashes `s` using the FNV hash function.
static auto hash_string(const std::string_view s, const std::uint64_t initial_value) -> std::uint64_t
{
    const auto FNV_PRIME = 1'099'511'628'211ULL;
    auto result          = initial_value;

    for (const auto c : s)
    {
        result ^= c;
        result *= FNV_PRIME;
    }

    return result;
}

auto build_caching::hash_file_contents(const std::filesystem::path& path, std::string& buffer) -> std::uint64_t
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", path.native()));
    }

    // Read the file into the buffer, preserving capacity.
    const auto file_size = std::filesystem::file_size(path);
    buffer.reserve(std::max(buffer.capacity(), file_size));
    buffer.resize(file_size);
    file.read(buffer.data(), file_size);

    return hash_string(buffer, FNV_OFFSET_BASIS);
}

/// @brief  Hashes the critical fields in a configuration.
/// @param  configuration contents to hash.
/// @return An integer hash value.
/// @note   Only hashes fields that would require a clean rebuild if changed.
///         Non-critical fields are intentionally ignored.
auto build_caching::hash_configuration(const Configuration& configuration) -> std::uint64_t
{
    auto result = FNV_OFFSET_BASIS;

    if (configuration.compiler.has_value())
    {
        result = hash_string(*configuration.compiler, result);
    }

    if (configuration.standard.has_value())
    {
        result = hash_string(*configuration.standard, result);
    }

    if (configuration.optimization.has_value())
    {
        result = hash_string(*configuration.optimization, result);
    }

    if (configuration.warnings.has_value())
    {
        for (const auto& warning : *configuration.warnings)
        {
            result = hash_string(warning, result);
        }
    }

    if (configuration.defines.has_value())
    {
        for (const auto& definition : *configuration.defines)
        {
            result = hash_string(definition, result);
        }
    }

    if (configuration.include_directories.has_value())
    {
        for (const auto& included_directory : *configuration.include_directories)
        {
            result = hash_string(included_directory, result);
        }
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

auto build_caching::get_old_configuration_hash(const std::string_view configuration_name,
                                               const std::filesystem::path& path_to_root) -> std::uint64_t
{
    const auto hash_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::CONFIGURATION_HASH_DATA_FILE_NAME;

    // Note: we return 0 to safely trigger recompilation.
    // Since 0 indicates "no previous hash exists" it will always differ from any valid cached hash,
    // ensuring a clean rebuild.
    const auto DEFAULT_VALUE = 0;

    if (!std::filesystem::exists(hash_data_file_path))
    {
        return DEFAULT_VALUE;
    }

    auto data_file = std::ifstream(hash_data_file_path);

    if (!data_file.is_open())
    {
        std::println("Failed to open '{}'.", hash_data_file_path.native());

        return DEFAULT_VALUE;
    }

    nlohmann::json json;

    try
    {
        data_file >> json;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        std::println("Error: Invalid JSON in '{}' - {}", params::CONFIGURATIONS_FILE_NAME.native(), e.what());

        return DEFAULT_VALUE;
    }

    ASSERT(json.contains("hash"));
    ASSERT(json["hash"].is_number_integer());

    return json["hash"].get<std::uint64_t>();
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
    std::string buffer;
    [[maybe_unused]] auto previous_capacity = buffer.capacity();

    for (const auto& file : code_files)
    {
        file_hashes[file] = hash_file_contents(file, buffer);

        // Using a buffer is only advantageous if its size never decreases.
        ASSERT(previous_capacity <= buffer.capacity());
        previous_capacity = buffer.capacity();
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

// Remove header files and sort `files`.
static auto sanitize_code_files(const std::vector<std::filesystem::path>& files) -> std::vector<std::filesystem::path>
{
    auto result = files;

    std::erase_if(result, &utils::is_header_file);
    std::ranges::sort(result);

    return result;
}

auto build_caching::get_files_to_compile(const DependencyGraph& old_dependency_graph,
                                         const DependencyGraph& new_dependency_graph,
                                         const std::vector<std::filesystem::path>& changed_files)
    -> std::vector<std::filesystem::path>
{
    auto files_affected_by_removal      = get_files_affected_by_removal(old_dependency_graph, new_dependency_graph);
    auto files_affected_by_code_changes = new_dependency_graph.get_reachable_nodes(changed_files);

    std::vector<std::filesystem::path> files_to_compile;
    files_to_compile.reserve(files_affected_by_removal.size() + files_affected_by_code_changes.size());

    files_to_compile.insert(files_to_compile.end(),
                            std::make_move_iterator(files_affected_by_removal.begin()),
                            std::make_move_iterator(files_affected_by_removal.end()));

    files_to_compile.insert(files_to_compile.end(),
                            std::make_move_iterator(files_affected_by_code_changes.begin()),
                            std::make_move_iterator(files_affected_by_code_changes.end()));

    return sanitize_code_files(files_to_compile);
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

auto build_caching::write_to_configuration_hash_data_file(const std::string_view configuration_name,
                                                          const std::filesystem::path& path_to_root,
                                                          const uint64_t value) -> void
{
    std::filesystem::create_directories(path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name);

    const auto hash_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::CONFIGURATION_HASH_DATA_FILE_NAME;

    auto data_file = std::ofstream(hash_data_file_path, std::ios::trunc); // Override file if it exists.

    if (!data_file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", hash_data_file_path.native()));
    }

    auto json = nlohmann::json{
        {"hash", value}
    };

    data_file << json.dump();
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
    const auto old_file_hashes        = get_old_file_hashes(*configuration.name, path_to_root);
    const auto old_configuration_hash = get_old_configuration_hash(*configuration.name, path_to_root);
    const auto old_dependency_graph   = get_old_dependency_graph(*configuration.name, path_to_root);

    // Gather information about the current state.
    const auto new_file_hashes        = get_new_file_hashes(code_files);
    const auto new_configuration_hash = hash_configuration(configuration);
    const auto new_dependency_graph =
        get_dependency_graph(path_to_root, code_files, configuration.include_directories.value_or({}));

    // Make sure new state does not contain any circular includes.
    const auto cycle_in_new_dependency_graph = new_dependency_graph.check_for_cycle();
    const auto detected_cycle                = cycle_in_new_dependency_graph.has_value();

    if (detected_cycle)
    {
        return std::unexpected(create_circular_dependencies_error_message(*cycle_in_new_dependency_graph));
    }

    // Update data files.
    write_to_build_data_file(*configuration.name, path_to_root, new_file_hashes);
    write_to_configuration_hash_data_file(*configuration.name, path_to_root, new_configuration_hash);
    write_to_dependency_graph_data_file(*configuration.name, path_to_root, new_dependency_graph);

    const auto files_to_delete = get_files_to_delete(old_file_hashes, new_file_hashes);

    // Decide which files to compile.
    // If some critical change happened to the configuration (e.g different optimization level or warning),
    // All files need to be compiled.
    const auto detected_critical_changes_to_configuration = old_configuration_hash != new_configuration_hash;

    if (detected_critical_changes_to_configuration)
    {
        return Info{
            .files_to_delete  = files_to_delete,
            .files_to_compile = sanitize_code_files(code_files), // Compile all the source files.
        };
    };

    // The configuration did not change meaningfully; compile only files affected by changes and removals.
    const auto changed_files = get_changed_files(*configuration.name, path_to_root, old_file_hashes, new_file_hashes);
    const auto files_to_compile = get_files_to_compile(old_dependency_graph, new_dependency_graph, changed_files);

    return Info{
        .files_to_delete  = files_to_delete,
        .files_to_compile = files_to_compile,
    };
}

#include "source/build_caching/build_caching.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <ranges>
#include <stdexcept>

#include "third_party/nlohmann/json.hpp"

#include "source/build_caching/dependency_graph.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/graph.hpp"
#include "source/utils/utils.hpp"

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

auto build_caching::get_files_to_compile(const std::filesystem::path& path_to_root,
                                         const std::vector<std::filesystem::path>& code_files,
                                         const std::vector<std::filesystem::path>& changed_files,
                                         const std::vector<std::string>& include_directories)
    -> std::expected<std::vector<std::filesystem::path>, std::string>
{
    const auto dependency_graph      = get_dependency_graph(path_to_root, code_files, include_directories);
    const auto cycle                 = dependency_graph.check_for_cycle();
    const auto cycle_exists_in_graph = cycle.has_value();

    if (cycle_exists_in_graph)
    {
        const auto error = std::format("Error: Circular header dependency detected.\n\n"
                                       "The following headers form a cycle:\n"
                                       "{}\n\n"
                                       "Consider restructuring the code to break the circular dependency.",
                                       *cycle);

        return std::unexpected(error);
    }

    // Find all files affected by changes (including transitive dependencies).
    const auto affected_files = dependency_graph.get_reachable_nodes(changed_files);
    auto files_to_compile =
        affected_files | std::views::filter(&utils::is_source_file) | std::ranges::to<std::vector>();

    // Make sure the files are compiled in lexicographic order.
    std::ranges::sort(files_to_compile);

    return files_to_compile;
}

auto build_caching::write_info_to_build_data_file(
    const std::string_view configuration_name,
    const std::filesystem::path& path_to_root,
    const std::unordered_map<std::filesystem::path, std::uint64_t>& hashes) -> void
{
    std::filesystem::create_directories(path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name);

    const auto build_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::BUILD_DATA_FILE_NAME;

    std::ofstream data_file(build_data_file_path, std::ios::trunc); // Override file if it exists.

    if (!data_file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", build_data_file_path.native()));
    }

    // Use `ordered_json` so "path" appears before "hash" - purely cosmetic.
    nlohmann::ordered_json json;

    for (const auto& [path, hash] : hashes)
    {
        json.push_back(nlohmann::ordered_json{{"path", path}, {"hash", hash}});
    }

    data_file << json.dump(); // Write to file.
}

auto build_caching::handle_build_caching(const Configuration& configuration,
                                         const std::filesystem::path& path_to_root,
                                         const std::vector<std::filesystem::path>& code_files)
    -> std::expected<Info, std::string>
{
    const auto old_file_hashes = get_old_file_hashes(*configuration.name, path_to_root);
    const auto new_file_hashes = get_new_file_hashes(code_files);
    const auto files_to_delete = get_files_to_delete(old_file_hashes, new_file_hashes);
    const auto changed_files   = get_changed_files(*configuration.name, path_to_root, old_file_hashes, new_file_hashes);
    const auto files_to_compile =
        get_files_to_compile(path_to_root, code_files, changed_files, configuration.include_directories.value_or({}));

    const auto circular_header_dependency_detected = !files_to_compile.has_value();

    if (circular_header_dependency_detected)
    {
        return std::unexpected(files_to_compile.error());
    }

    write_info_to_build_data_file(*configuration.name, path_to_root, new_file_hashes);

    return Info{.files_to_delete = files_to_delete, .files_to_compile = *files_to_compile};
}

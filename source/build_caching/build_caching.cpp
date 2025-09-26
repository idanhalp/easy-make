#include "source/build_caching/build_caching.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <ranges>
#include <stdexcept>

#include "third_party/json.hpp"

#include "source/parameters/parameters.hpp"

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

auto build_caching::get_new_file_hashes(const std::vector<std::filesystem::path>& source_files)
    -> std::unordered_map<std::filesystem::path, std::uint64_t>
{
    std::unordered_map<std::filesystem::path, std::uint64_t> file_hashes;

    for (const auto& file : source_files)
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

auto build_caching::get_files_to_compile(
    const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
    const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
    -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> files_to_compile;

    for (const auto& [file, contents_hash] : new_file_hashes)
    {
        const auto old_object_file_exists = old_file_hashes.contains(file);
        const auto file_contents_changed  = old_object_file_exists && old_file_hashes.at(file) != contents_hash;

        if (!old_object_file_exists || file_contents_changed)
        {
            files_to_compile.push_back(file);
        }
    }

    std::ranges::sort(files_to_compile);

    return files_to_compile;
}

auto build_caching::write_info_to_build_data_file(
    const std::string_view configuration_name,
    const std::filesystem::path& path_to_root,
    const std::unordered_map<std::filesystem::path, std::uint64_t>& hashes) -> void
{
    std::filesystem::create_directory(path_to_root / params::BUILD_DIRECTORY_NAME);
    std::filesystem::create_directory(path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name);

    const auto build_data_file_path =
        path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name / params::BUILD_DATA_FILE_NAME;

    std::ofstream data_file(build_data_file_path, std::ios::trunc); // Override file if it exists.

    if (!data_file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open '{}'.", build_data_file_path.native()));
    }

    // `ordered_json` instead of `json` because it is very important
    // that the "path" key is before "hash".
    nlohmann::ordered_json json;

    for (const auto& [path, hash] : hashes)
    {
        json.push_back(nlohmann::ordered_json{{"path", path}, {"hash", hash}});
    }

    data_file << json.dump(4); // Write to file.
}

auto build_caching::handle_build_caching(const std::string_view configuration_name,
                                         const std::filesystem::path& path_to_root,
                                         const std::vector<std::filesystem::path>& source_files) -> Info
{
    const auto old_file_hashes  = get_old_file_hashes(configuration_name, path_to_root);
    const auto new_file_hashes  = get_new_file_hashes(source_files);
    const auto files_to_delete  = get_files_to_delete(old_file_hashes, new_file_hashes);
    const auto files_to_compile = get_files_to_compile(old_file_hashes, new_file_hashes);

    write_info_to_build_data_file(configuration_name, path_to_root, new_file_hashes);

    return {.files_to_delete = files_to_delete, .files_to_compile = files_to_compile};
}

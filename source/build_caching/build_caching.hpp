#ifndef SOURCE_BUILD_CACHING_BUILD_CACHING_HPP
#define SOURCE_BUILD_CACHING_BUILD_CACHING_HPP

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace build_caching
{
    struct Info
    {
        std::vector<std::filesystem::path> files_to_delete;
        std::vector<std::filesystem::path> files_to_compile;
    };

    auto hash_file_contents(const std::filesystem::path& path) -> std::uint64_t;

    auto get_old_file_hashes(std::string_view configuration_name, const std::filesystem::path& path_to_root)
        -> std::unordered_map<std::filesystem::path, std::uint64_t>;

    auto get_new_file_hashes(const std::vector<std::filesystem::path>& source_files)
        -> std::unordered_map<std::filesystem::path, std::uint64_t>;

    auto get_files_to_delete(const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
                             const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
        -> std::vector<std::filesystem::path>;

    auto get_files_to_compile(const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
                              const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
        -> std::vector<std::filesystem::path>;

    auto write_info_to_build_data_file(std::string_view configuration_name,
                                       const std::filesystem::path& path_to_root,
                                       const std::unordered_map<std::filesystem::path, std::uint64_t>& info) -> void;

    auto handle_build_caching(std::string_view configuration_name,
                              const std::filesystem::path& path_to_root,
                              const std::vector<std::filesystem::path>& source_files) -> Info;
}

#endif // SOURCE_BUILD_CACHING_BUILD_CACHING_HPP

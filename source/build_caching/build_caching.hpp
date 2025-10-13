#ifndef SOURCE_BUILD_CACHING_BUILD_CACHING_HPP
#define SOURCE_BUILD_CACHING_BUILD_CACHING_HPP

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"

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

    auto get_new_file_hashes(const std::vector<std::filesystem::path>& code_files)
        -> std::unordered_map<std::filesystem::path, std::uint64_t>;

    auto get_files_to_delete(const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
                             const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
        -> std::vector<std::filesystem::path>;

    auto get_changed_files(std::string_view configuration_name,
                           const std::filesystem::path& path_to_root,
                           const std::unordered_map<std::filesystem::path, std::uint64_t>& old_file_hashes,
                           const std::unordered_map<std::filesystem::path, std::uint64_t>& new_file_hashes)
        -> std::vector<std::filesystem::path>;

    auto get_files_to_compile(const std::filesystem::path& path_to_root,
                              const std::vector<std::filesystem::path>& code_files,
                              const std::vector<std::filesystem::path>& changed_files,
                              const std::vector<std::string>& include_directories)
        -> std::expected<std::vector<std::filesystem::path>, std::string>;

    auto write_info_to_build_data_file(std::string_view configuration_name,
                                       const std::filesystem::path& path_to_root,
                                       const std::unordered_map<std::filesystem::path, std::uint64_t>& info) -> void;

    auto handle_build_caching(const Configuration& configuration,
                              const std::filesystem::path& path_to_root,
                              const std::vector<std::filesystem::path>& code_files) -> std::expected<Info, std::string>;
}

#endif // SOURCE_BUILD_CACHING_BUILD_CACHING_HPP

#ifndef SOURCE_UTILS_UTILS_HPP
#define SOURCE_UTILS_UTILS_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace utils
{
    auto check_if_configurations_file_exists(const std::filesystem::path& path) -> bool;

    auto get_object_file_name(const std::filesystem::path& path) -> std::string;

    auto get_ordinal_indicator(int index) -> const char*;

    auto is_header_file(const std::filesystem::path& path) -> bool;

    auto is_source_file(const std::filesystem::path& path) -> bool;

    auto is_code_file(const std::filesystem::path& path) -> bool;

    auto count_digits(int x) -> int;
}

#endif // SOURCE_UTILS_UTILS_HPP

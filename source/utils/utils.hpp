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
}

#endif // SOURCE_UTILS_UTILS_HPP

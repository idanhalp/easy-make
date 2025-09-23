#ifndef SOURCE_UTILS_UTILS_HPP
#define SOURCE_UTILS_UTILS_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace utils
{
    const std::string_view CONFIGURATIONS_FILE_NAME = "easy-make-configurations.json";
    const std::string_view BUILD_DIRECTORY_NAME     = "easy-make-build";

    auto check_if_configurations_file_exists(const std::filesystem::path& path) -> bool;
    auto get_chosen_configuration(const std::vector<const char*>& arguments) -> std::optional<std::string>;
}

#endif // SOURCE_UTILS_UTILS_HPP

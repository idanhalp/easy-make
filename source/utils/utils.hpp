#ifndef SOURCE_UTILS_UTILS_HPP
#define SOURCE_UTILS_UTILS_HPP

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace utils
{
    const std::string_view CONFIGURATIONS_FILE_NAME = "easy-make-configurations.json";

    auto check_if_configurations_file_exists(std::string_view path) -> bool;
    auto get_chosen_configuration(const std::vector<const char *> &arguments) -> std::optional<std::string>;
}

#endif // SOURCE_UTILS_UTILS_HPP

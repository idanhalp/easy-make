#ifndef SOURCE_UTILS_UTILS_HPP
#define SOURCE_UTILS_UTILS_HPP

#include <string_view>

namespace utils
{
    const std::string_view CONFIGURATIONS_FILE_NAME = "easy-make-configurations.json";

    auto check_if_configurations_file_exists(std::string_view path) -> bool;
}

#endif // SOURCE_UTILS_UTILS_HPP

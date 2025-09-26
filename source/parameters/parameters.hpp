#ifndef SOURCE_PARAMETERS_PARAMETERS_HPP
#define SOURCE_PARAMETERS_PARAMETERS_HPP

#include <filesystem>
#include <string_view>

namespace params
{
    const std::filesystem::path CONFIGURATIONS_FILE_NAME = "easy-make-configurations.json";
    const std::filesystem::path BUILD_DIRECTORY_NAME     = "easy-make-build";
    const std::string_view BUILD_DATA_FILE_NAME          = "build-data.json";
}

#endif // SOURCE_PARAMETERS_PARAMETERS_HPP

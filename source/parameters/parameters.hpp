#ifndef SOURCE_PARAMETERS_PARAMETERS_HPP
#define SOURCE_PARAMETERS_PARAMETERS_HPP

#include <filesystem>
#include <string_view>

namespace params
{
    const std::filesystem::path CONFIGURATIONS_FILE_NAME     = "easy-make-configurations.json";
    const std::filesystem::path BUILD_DIRECTORY_NAME         = "easy-make-build";
    const std::string_view BUILD_DATA_FILE_NAME              = "build-data.json";
    const std::string_view DEPENDENCY_GRAPH_DATA_FILE_NAME   = "dependencies.json";
    const std::string_view CONFIGURATION_HASH_DATA_FILE_NAME = "configuration-hash.json";
    const auto ENABLE_MSVC                                   = false;
}

#endif // SOURCE_PARAMETERS_PARAMETERS_HPP

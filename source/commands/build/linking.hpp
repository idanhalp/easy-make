#ifndef SOURCE_COMMANDS_BUILD_LINKING_HPP
#define SOURCE_COMMANDS_BUILD_LINKING_HPP

#include <filesystem>

#include "source/configuration_parsing/configuration.hpp"

auto link_object_files(const Configuration& configuration,
                       const std::filesystem::path& path_to_root,
                       bool is_quiet) -> bool;

#endif // SOURCE_COMMANDS_BUILD_LINKING_HPP

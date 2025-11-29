#ifndef SOURCE_COMMANDS_BUILD_LINKING_HPP
#define SOURCE_COMMANDS_BUILD_LINKING_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "source/configuration_parsing/configuration.hpp"

auto link_object_files(const Configuration& configuration,
                       const std::filesystem::path& path_to_root,
                       const std::vector<std::string>& flags,
                       bool is_quiet) -> bool;

#endif // SOURCE_COMMANDS_BUILD_LINKING_HPP

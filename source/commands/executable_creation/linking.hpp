#ifndef SOURCE_EXECUTABLE_CREATION_LINKING_HPP
#define SOURCE_EXECUTABLE_CREATION_LINKING_HPP

#include <filesystem>

#include "source/configuration_parsing/configuration.hpp"

auto link_object_files(const Configuration& configuration, const std::filesystem::path& path_to_root) -> bool;

#endif // SOURCE_EXECUTABLE_CREATION_LINKING_HPP

#ifndef SOURCE_COMMANDS_BUILD_COMPILATION_HPP
#define SOURCE_COMMANDS_BUILD_COMPILATION_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "source/configuration_parsing/configuration.hpp"

auto create_compilation_flags_string(const Configuration& configuration) -> std::string;

auto compile_files(const Configuration& configuration,
                   const std::filesystem::path& path_to_root,
                   const std::vector<std::filesystem::path>& files_to_compile,
                   bool is_quiet) -> int;

#endif // SOURCE_COMMANDS_BUILD_COMPILATION_HPP

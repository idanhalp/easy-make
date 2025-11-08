#ifndef SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP
#define SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

auto check_names_validity(const std::vector<Configuration>& configurations) -> std::optional<std::string>;

auto check_parents_validity(const std::unordered_map<std::string, Configuration>& name_to_configuration)
    -> std::optional<std::string>;

auto get_actual_configuration(const std::string& configuration_name, const std::vector<Configuration>& configurations)
    -> std::expected<Configuration, std::string>;

auto get_code_files(const Configuration& configuration,
                    const std::filesystem::path& path_to_root) -> std::vector<std::filesystem::path>;

namespace commands
{
    auto create_executable(const CompileCommandInfo& info,
                           const std::vector<Configuration>& configurations,
                           const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP

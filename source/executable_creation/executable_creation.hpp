#ifndef SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP
#define SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"

auto check_names_validity(const std::vector<Configuration>& configurations) -> std::optional<std::string>;

auto check_parents_validity(const std::unordered_map<std::string, Configuration>& name_to_configuration)
    -> std::optional<std::string>;

auto get_actual_configuration(std::string configuration_name, const std::vector<Configuration>& configurations)
    -> std::expected<Configuration, std::string>;

auto get_code_files(const Configuration& configuration,
                    const std::filesystem::path& path_to_root) -> std::vector<std::filesystem::path>;

auto create_compilation_flags_string(const Configuration& configuration) -> std::string;

auto create_executable(std::string_view configuration_name,
                       const std::filesystem::path& path_to_root,
                       const std::vector<Configuration>& configurations) -> int;

#endif // SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP

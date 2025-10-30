#ifndef SOURCE_CONFIGURATIONS_PARSING_CONFIGURATIONS_PARSING_HPP
#define SOURCE_CONFIGURATIONS_PARSING_CONFIGURATIONS_PARSING_HPP

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

#include "source/configuration_parsing/configuration.hpp"

auto parse_configurations(const std::filesystem::path& path_to_root)
    -> std::expected<std::vector<Configuration>, std::string>;

#endif // SOURCE_CONFIGURATIONS_PARSING_CONFIGURATIONS_PARSING_HPP

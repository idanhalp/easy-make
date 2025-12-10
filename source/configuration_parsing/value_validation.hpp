#ifndef SOURCE_CONFIGURATION_PARSING_VALUE_VALIDATION_HPP
#define SOURCE_CONFIGURATION_PARSING_VALUE_VALIDATION_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "third_party/nlohmann/json.hpp"

#include "source/configuration_parsing/configuration.hpp"

auto validate_configuration_values(const std::vector<Configuration>& configurations,
                                   const std::filesystem::path& path_to_root) -> std::optional<std::string>;

#endif // SOURCE_CONFIGURATION_PARSING_VALUE_VALIDATION_HPP

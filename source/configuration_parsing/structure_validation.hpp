#ifndef SOURCE_CONFIGURATION_PARSING_STRUCTURE_VALIDATION_HPP
#define SOURCE_CONFIGURATION_PARSING_STRUCTURE_VALIDATION_HPP

#include <optional>
#include <string>

#include "third_party/nlohmann/json.hpp"

auto validate_json_structure(const nlohmann::json& json) -> std::optional<std::string>;

#endif // SOURCE_CONFIGURATION_PARSING_STRUCTURE_VALIDATION_HPP

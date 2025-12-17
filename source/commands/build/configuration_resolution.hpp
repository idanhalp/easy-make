#ifndef SOURCE_COMMANDS_BUILD_CONFIGURATION_RESOLUTION_HPP
#define SOURCE_COMMANDS_BUILD_CONFIGURATION_RESOLUTION_HPP

#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "source/configuration_parsing/configuration.hpp"

enum class ConfigurationType
{
    ALL,
    COMPLETE,
    INCOMPLETE,
};

auto get_resolved_configuration(const std::vector<Configuration>& configurations,
                                std::string_view target_name) -> std::expected<Configuration, std::string>;

auto get_resolved_configurations(const std::vector<Configuration>& configurations,
                                 ConfigurationType configuration_type) -> std::vector<Configuration>;

#endif // SOURCE_COMMANDS_BUILD_CONFIGURATION_RESOLUTION_HPP

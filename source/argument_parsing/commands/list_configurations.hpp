#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_LIST_CONFIGURATIONS_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_LIST_CONFIGURATIONS_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_list_configurations_command_arguments(std::span<const char* const> arguments)
    -> std::expected<ListConfigurationsCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_LIST_CONFIGURATIONS_HPP

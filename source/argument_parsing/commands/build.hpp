#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_BUILD_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_BUILD_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_build_command_arguments(std::span<const char* const> arguments)
    -> std::expected<BuildCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_BUILD_HPP

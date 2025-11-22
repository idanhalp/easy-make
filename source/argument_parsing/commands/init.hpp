#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_INIT_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_INIT_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_init_command_arguments(std::span<const char* const> arguments)
    -> std::expected<InitCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_INIT_HPP

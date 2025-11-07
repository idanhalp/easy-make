#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_COMPILE_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_COMPILE_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_compile_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CompileCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_COMPILE_HPP

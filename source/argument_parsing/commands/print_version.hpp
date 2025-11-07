#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_PRINT_VERSION_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_PRINT_VERSION_HPP

#include <expected>
#include <span>
#include <string>
#include <string_view>

#include "source/argument_parsing/command_info.hpp"

auto parse_print_version_command_arguments(std::span<const char* const> arguments)
    -> std::expected<PrintVersionCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_PRINT_VERSION_HPP

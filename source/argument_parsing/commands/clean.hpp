#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_CLEAN_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_CLEAN_HPP

#include <expected>
#include <span>
#include <string>
#include <string_view>

#include "source/argument_parsing/command_info.hpp"

auto parse_clean_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CleanCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_CLEAN_HPP

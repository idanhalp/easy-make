#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_CLEAN_ALL_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_CLEAN_ALL_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_clean_all_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CleanAllCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_CLEAN_ALL_HPP

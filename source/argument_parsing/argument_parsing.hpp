#ifndef SOURCE_ARGUMENT_PARSING_ARGUMENT_PARSING_HPP
#define SOURCE_ARGUMENT_PARSING_ARGUMENT_PARSING_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_arguments(std::span<const char* const> arguments) -> std::expected<CommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_ARGUMENT_PARSING_HPP

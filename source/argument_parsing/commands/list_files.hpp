#ifndef SOURCE_ARGUMENT_PARSING_COMMANDS_LIST_FILES_HPP
#define SOURCE_ARGUMENT_PARSING_COMMANDS_LIST_FILES_HPP

#include <expected>
#include <span>
#include <string>

#include "source/argument_parsing/command_info.hpp"

auto parse_list_files_command_arguments(std::span<const char* const> arguments)
    -> std::expected<ListFilesCommandInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_COMMANDS_LIST_FILES_HPP

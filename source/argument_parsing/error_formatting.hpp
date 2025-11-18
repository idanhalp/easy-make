#ifndef SOURCE_ARGUMENT_PARSING_ERROR_FORMATTING_HPP
#define SOURCE_ARGUMENT_PARSING_ERROR_FORMATTING_HPP

#include <string>
#include <string_view>
#include <vector>

auto create_missing_configuration_name_error(std::string_view command_name) -> std::string;

auto create_multiple_configuration_names_error(std::string_view command_name,
                                               std::string_view name_1,
                                               std::string_view name_2) -> std::string;

auto create_unknown_flag_error(std::string_view command_name,
                               const std::string& flag,
                               const std::vector<std::string>& valid_flags) -> std::string;

auto create_duplicate_flag_error(std::string_view command_name, std::string_view flag) -> std::string;

auto create_unknown_argument_error(std::string_view command_name,
                                   const std::string& argument,
                                   const std::vector<std::string>& valid_arguments) -> std::string;

auto create_unknown_command_error(const std::string& command,
                                  const std::vector<std::string>& valid_commands) -> std::string;

#endif // SOURCE_ARGUMENT_PARSING_ERROR_FORMATTING_HPP

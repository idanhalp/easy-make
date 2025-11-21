#include "source/argument_parsing/error_formatting.hpp"

#include <format>

#include "source/utils/find_closest_word.hpp"

auto create_missing_configuration_name_error(const std::string_view command_name) -> std::string
{
    return std::format("Error: Must specify a configuration name when using '{}' command.", command_name);
}

auto create_multiple_configuration_names_error(const std::string_view command_name,
                                               const std::string_view name_1,
                                               const std::string_view name_2) -> std::string
{
    const auto same_name_provided_multiple_times = name_1 == name_2;

    if (same_name_provided_multiple_times)
    {
        return std::format("Error: Command '{}' requires one configuration name, "
                           "instead got '{}' more than once.",
                           command_name,
                           name_1);
    }
    else
    {
        return std::format("Error: Command '{}' requires one configuration name, "
                           "instead got both '{}' and '{}'.",
                           command_name,
                           name_1,
                           name_2);
    }
}

auto create_unknown_flag_error(const std::string_view command_name,
                               const std::string_view flag,
                               const std::span<const std::string_view> valid_flags) -> std::string
{
    const auto closest_flag = utils::find_closest_word(flag, valid_flags);

    if (closest_flag.has_value())
    {
        return std::format("Error: Unknown flag '{}' provided to command '{}'. "
                           "Did you mean '{}'?",
                           flag,
                           command_name,
                           *closest_flag);
    }
    else
    {
        return std::format("Error: Unknown flag '{}' provided to command '{}'.", flag, command_name);
    }
}

auto create_duplicate_flag_error(const std::string_view command_name, const std::string_view flag) -> std::string
{
    return std::format("Error: Flag '{}' was provided to command '{}' more than once.", flag, command_name);
}

auto create_unknown_argument_error(const std::string_view command_name,
                                   const std::string_view argument,
                                   const std::vector<std::string>& valid_arguments) -> std::string
{
    const auto closest_argument = utils::find_closest_word(argument, valid_arguments);

    if (closest_argument.has_value())
    {
        return std::format("Error: Unknown argument '{}' provided to command '{}'. "
                           "Did you mean '{}'?",
                           argument,
                           command_name,
                           *closest_argument);
    }
    else
    {
        return std::format("Error: Unknown argument '{}' provided to command '{}'.", argument, command_name);
    }
}

auto create_unknown_command_error(const std::string_view command,
                                  const std::span<const std::string_view> valid_commands) -> std::string
{
    const auto closest_command = utils::find_closest_word(command, valid_commands);

    if (closest_command.has_value())
    {
        return std::format("Error: Unknown command '{}'. Did you mean '{}'?", command, *closest_command);
    }
    else
    {
        return std::format("Error: Unknown command '{}'.", command);
    }
}

#include "source/argument_parsing/commands/build.hpp"

#include <format>
#include <string_view>

#include <source/utils/macros/assert.hpp>

auto parse_build_command_arguments(std::span<const char* const> arguments)
    -> std::expected<BuildCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "build").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    BuildCommandInfo info{};
    auto configuration_name_specified = false;

    for (const std::string_view argument : relevant_arguments)
    {
        const auto is_flag = argument.starts_with("--");

        if (is_flag)
        {
            return std::unexpected(
                std::format("Error: Unknown flag '{}' provided to command '{}'.", argument, command_name));
        }

        // Argument is not a flag - assume it is a configuration name.

        if (configuration_name_specified)
        {
            const auto& name_1       = info.configuration_name;
            const auto& name_2       = argument;
            const auto error_message = (name_1 == name_2)
                                           ? std::format("Error: Command '{}' requires one configuration name, "
                                                         "instead got '{}' twice.",
                                                         command_name,
                                                         name_1)
                                           : std::format("Error: Command '{}' requires one configuration name, "
                                                         "instead got both '{}' and '{}'.",
                                                         command_name,
                                                         name_1,
                                                         name_2);

            return std::unexpected(error_message);
        }

        info.configuration_name      = argument;
        configuration_name_specified = true;
    }

    if (!configuration_name_specified)
    {
        return std::unexpected(
            std::format("Error: Must specify a configuration name when using '{}' command.", command_name));
    }

    return info;
}

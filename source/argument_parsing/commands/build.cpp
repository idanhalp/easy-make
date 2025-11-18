#include "source/argument_parsing/commands/build.hpp"

#include <string_view>

#include "source/argument_parsing/error_formatting.hpp"
#include "source/utils/macros/assert.hpp"

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
            return std::unexpected(create_unknown_flag_error(command_name, std::string(argument), {}));
        }

        // Argument is not a flag - assume it is a configuration name.

        if (configuration_name_specified)
        {
            const auto& name_1 = info.configuration_name;
            const auto& name_2 = argument;

            return std::unexpected(create_multiple_configuration_names_error(command_name, name_1, name_2));
        }

        info.configuration_name      = argument;
        configuration_name_specified = true;
    }

    if (!configuration_name_specified)
    {
        return std::unexpected(create_missing_configuration_name_error(command_name));
    }

    return info;
}

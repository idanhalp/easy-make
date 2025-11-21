#include "source/argument_parsing/commands/build.hpp"

#include <string_view>

#include "source/argument_parsing/error_formatting.hpp"
#include "source/argument_parsing/utils.hpp"
#include "source/utils/macros/assert.hpp"

auto parse_build_command_arguments(std::span<const char* const> arguments)
    -> std::expected<BuildCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "build").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    BuildCommandInfo info{};
    auto configuration_name_provided = false;

    for (const std::string_view argument : relevant_arguments)
    {
        if (utils::is_flag(argument))
        {
            return std::unexpected(create_unknown_flag_error(command_name, std::string(argument), {}));
        }

        // Assume `argument` is a configuration name.
        // If `configuration_name_provided` was already set before handling the current argument,
        // it means that multiple configuration names were provided.
        const auto multiple_configuration_names_provided = configuration_name_provided;

        if (multiple_configuration_names_provided)
        {
            const auto& name_1 = info.configuration_name;
            const auto& name_2 = argument;

            return std::unexpected(create_multiple_configuration_names_error(command_name, name_1, name_2));
        }

        info.configuration_name     = argument;
        configuration_name_provided = true;
    }

    if (!configuration_name_provided)
    {
        return std::unexpected(create_missing_configuration_name_error(command_name));
    }

    return info;
}

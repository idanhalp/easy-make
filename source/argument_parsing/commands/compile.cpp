#include "source/argument_parsing/commands/compile.hpp"

#include <format>
#include <string_view>

#include <source/utils/macros/assert.hpp>

auto parse_compile_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CompileCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "compile").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    CompileCommandInfo info{};
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
            const auto& name_1 = info.configuration_name;
            const auto& name_2 = argument;

            return std::unexpected(std::format("Error: Command '{}' requires one configuration name, "
                                               "instead got both '{}' and '{}'.",
                                               command_name,
                                               name_1,
                                               name_2));
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

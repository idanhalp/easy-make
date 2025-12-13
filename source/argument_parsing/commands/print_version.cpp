#include "source/argument_parsing/commands/print_version.hpp"

#include <format>

#include "source/utils/macros/assert.hpp"

auto parse_print_version_command_arguments(std::span<const char* const> arguments)
    -> std::expected<PrintVersionCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "version").
    ASSERT(arguments.size() >= 2);
    const auto command_name     = std::string_view(arguments[1]);
    const auto actual_arguments = std::span(arguments.begin() + 2, arguments.end());

    if (!actual_arguments.empty())
    {
        return std::unexpected(std::format("Error: Command '{}' doesn't accept any arguments or flags.", command_name));
    }

    return PrintVersionCommandInfo{};
}

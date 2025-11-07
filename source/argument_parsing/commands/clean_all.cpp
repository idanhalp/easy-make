#include "source/argument_parsing/commands/clean_all.hpp"

#include <format>

#include "source/utils/macros/assert.hpp"

auto parse_clean_all_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CleanAllCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "compile").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    if (!relevant_arguments.empty())
    {
        return std::unexpected(std::format("Error: Command '{}' doesn't accept any arguments or flags.", command_name));
    }

    return CleanAllCommandInfo{};
}

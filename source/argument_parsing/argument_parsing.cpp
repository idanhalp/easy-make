#include "source/argument_parsing/argument_parsing.hpp"

#include <algorithm>
#include <flat_set>
#include <string>
#include <string_view>

#include "source/argument_parsing/commands/build.hpp"
#include "source/argument_parsing/commands/clean.hpp"
#include "source/argument_parsing/commands/clean_all.hpp"
#include "source/argument_parsing/commands/list_configurations.hpp"
#include "source/argument_parsing/commands/list_files.hpp"
#include "source/argument_parsing/commands/print_version.hpp"
#include "source/argument_parsing/error_formatting.hpp"
#include "source/utils/macros/assert.hpp"

using namespace std::literals;

static const auto BUILD_COMMAND               = "build"sv;
static const auto CLEAN_COMMAND               = "clean"sv;
static const auto CLEAN_ALL_COMMAND           = "clean-all"sv;
static const auto LIST_CONFIGURATIONS_COMMAND = "list-configs"sv;
static const auto LIST_FILES_COMMAND          = "list-files"sv;
static const auto PRINT_VERSION_COMMAND       = "version"sv;

static const std::flat_set COMMANDS = {
    BUILD_COMMAND,
    CLEAN_COMMAND,
    CLEAN_ALL_COMMAND,
    LIST_CONFIGURATIONS_COMMAND,
    LIST_FILES_COMMAND,
    PRINT_VERSION_COMMAND,
};

auto parse_arguments(const std::span<const char* const> arguments) -> std::expected<CommandInfo, std::string>
{
    // Ensure that the number of command strings matches the number of variant command types.
    ASSERT(COMMANDS.size() == (std::variant_size_v<CommandInfo>));

    // Ensure commands are unique.
    ASSERT(std::ranges::is_sorted(COMMANDS));
    ASSERT(std::ranges::adjacent_find(COMMANDS, std::ranges::equal_to{}) == COMMANDS.end());

    // `arguments[0]` is the program's name.
    // `arguments[1]` is the command.
    const auto command_is_specified = arguments.size() >= 2;

    if (!command_is_specified)
    {
        return std::unexpected("Error: Must specify a command.");
    }

    const auto command = std::string_view(arguments[1]);

    if (command == CLEAN_COMMAND)
    {
        return parse_clean_command_arguments(arguments);
    }
    else if (command == CLEAN_ALL_COMMAND)
    {
        return parse_clean_all_command_arguments(arguments);
    }
    else if (command == BUILD_COMMAND)
    {
        return parse_build_command_arguments(arguments);
    }
    else if (command == LIST_CONFIGURATIONS_COMMAND)
    {
        return parse_list_configurations_command_arguments(arguments);
    }
    else if (command == LIST_FILES_COMMAND)
    {
        return parse_list_files_command_arguments(arguments);
    }
    else if (command == PRINT_VERSION_COMMAND)
    {
        return parse_print_version_command_arguments(arguments);
    }

    // All valid commands should have been handled above.
    // If we reach this point, the command is unknown.
    ASSERT(!std::ranges::contains(COMMANDS, command));

    return std::unexpected(create_unknown_command_error(command, COMMANDS));
}

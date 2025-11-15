#include "source/argument_parsing/argument_parsing.hpp"

#include <algorithm>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "source/argument_parsing/commands/build.hpp"
#include "source/argument_parsing/commands/clean.hpp"
#include "source/argument_parsing/commands/clean_all.hpp"
#include "source/argument_parsing/commands/list_configurations.hpp"
#include "source/argument_parsing/commands/list_files.hpp"
#include "source/argument_parsing/commands/print_version.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/macros/assert.hpp"

static const std::string BUILD_COMMAND               = "build";
static const std::string CLEAN_COMMAND               = "clean";
static const std::string CLEAN_ALL_COMMAND           = "clean-all";
static const std::string LIST_CONFIGURATIONS_COMMAND = "list-configs";
static const std::string LIST_FILES_COMMAND          = "list-files";
static const std::string PRINT_VERSION_COMMAND       = "version";

static const std::vector<std::string> COMMANDS = {
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

    // The given command is invalid.
    // Check if a similar valid command exists, and return a relevant error.

    const auto closest_command = utils::find_closest_word(std::string(command), COMMANDS);
    const auto error_message =
        closest_command.has_value()
            ? std::format("Error: Unknown command '{}'. Did you mean '{}'?", command, *closest_command)
            : std::format("Error: Unknown command '{}'.", command);

    return std::unexpected(error_message);
}

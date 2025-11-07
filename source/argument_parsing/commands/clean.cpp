#include "source/argument_parsing/commands/clean.hpp"

#include <format>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "source/argument_parsing/utils.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/macros/assert.hpp"

static const std::string QUIET_FLAG = "--quiet";

static const std::vector<std::string> FLAGS = {QUIET_FLAG};

// Validates `flag` and updates `info` if recognized.
// Returns `std::nullopt` on success, or an error message otherwise.
static auto parse_flag(const std::string_view flag,
                       const std::string_view command_name,
                       CleanCommandInfo& info) -> std::optional<std::string>
{
    if (flag == QUIET_FLAG)
    {
        info.is_quiet = true;

        return std::nullopt;
    }

    const auto closest_flag = utils::find_closest_word(std::string(flag), FLAGS);
    const auto error_message =
        closest_flag.has_value()
            ? std::format("Error: Unknown flag '{}' provided to command '{}'. Did you mean '{}'?", flag, command_name,
                          *closest_flag)
            : std::format("Error: Unknown flag '{}' provided to command '{}'.", flag, command_name);

    return error_message;
}

auto parse_clean_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CleanCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "clean").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    CleanCommandInfo info{};
    auto configuration_name_specified = false;

    for (const std::string_view argument : relevant_arguments)
    {
        if (utils::is_flag(argument))
        {
            const auto flag_parse_error = parse_flag(argument, command_name, info);
            const auto flag_is_valid    = !flag_parse_error.has_value();

            if (flag_is_valid)
            {
                continue;
            }
            else
            {
                return std::unexpected(*flag_parse_error);
            }
        }

        // Argument is not a flag - assume it is a configuration name.

        if (configuration_name_specified)
        {
            const auto& name_1 = info.configuration_name;
            const auto& name_2 = argument;

            return std::unexpected(std::format("Error: Command '{}' requires one configuration name, "
                                               "instead got both '{}' and '{}'.",
                                               command_name, name_1, name_2));
        }

        info.configuration_name      = argument;
        configuration_name_specified = true;
    }

    if (!configuration_name_specified)
    {
        return std::unexpected(
            std::format("Error: Must specify a configuration name when using '{}' command.", command_name));
    }

    const auto duplicate_flag        = utils::check_for_duplicate_flags(arguments);
    const auto duplicate_flag_exists = duplicate_flag.has_value();

    if (duplicate_flag_exists)
    {
        return std::unexpected(std::format("Error: Flag '{}' was provided to command '{}' more than once.",
                                           *duplicate_flag, command_name));
    }

    return info;
}

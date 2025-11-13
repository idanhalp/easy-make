#include "source/argument_parsing/commands/clean_all.hpp"

#include <format>
#include <string>
#include <string_view>
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
                       CleanAllCommandInfo& info) -> std::optional<std::string>
{
    if (flag == QUIET_FLAG)
    {
        info.is_quiet = true;

        return std::nullopt;
    }

    const auto closest_flag = utils::find_closest_word(std::string(flag), FLAGS);
    const auto error_message =
        closest_flag.has_value()
            ? std::format("Error: Unknown flag '{}' provided to command '{}'. Did you mean '{}'?",
                          flag,
                          command_name,
                          *closest_flag)
            : std::format("Error: Unknown flag '{}' provided to command '{}'.", flag, command_name);

    return error_message;
}

auto parse_clean_all_command_arguments(std::span<const char* const> arguments)
    -> std::expected<CleanAllCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "clean-all").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    CleanAllCommandInfo info{};

    for (const std::string_view argument : relevant_arguments)
    {
        if (!utils::is_flag(argument))
        {
            return std::unexpected(
                std::format("Error: Unknown argument '{}' provided to command '{}'.", argument, command_name));
        }

        const auto flag_parse_error = parse_flag(argument, command_name, info);
        const auto flag_is_valid    = !flag_parse_error.has_value();

        if (!flag_is_valid)
        {
            return std::unexpected(*flag_parse_error);
        }
    }

    const auto duplicate_flag        = utils::check_for_duplicate_flags(arguments);
    const auto duplicate_flag_exists = duplicate_flag.has_value();

    if (duplicate_flag_exists)
    {
        return std::unexpected(std::format(
            "Error: Flag '{}' was provided to command '{}' more than once.", *duplicate_flag, command_name));
    }

    return info;
}

#include "source/argument_parsing/commands/list.hpp"

#include <format>
#include <string>
#include <string_view>
#include <vector>

#include "source/argument_parsing/utils.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/macros/assert.hpp"

static const std::string COMPLETE_CONFIGURATINS_ONLY_FLAG   = "--complete-only";
static const std::string COUNT_FLAG                         = "--count";
static const std::string INCOMPLETE_CONFIGURATINS_ONLY_FLAG = "--incomplete-only";
static const std::string PORCELAIN_FLAG                     = "--porcelain";
static const std::string SORTED_FLAG                        = "--sorted";

static const std::vector<std::string> FLAGS = {
    COMPLETE_CONFIGURATINS_ONLY_FLAG, COUNT_FLAG, INCOMPLETE_CONFIGURATINS_ONLY_FLAG, PORCELAIN_FLAG, SORTED_FLAG,
};

// Validates `flag` and updates `info` if recognized.
// Returns `std::nullopt` on success, or an error message otherwise.
static auto parse_flag(const std::string_view flag,
                       const std::string_view command_name,
                       ListCommandInfo& info) -> std::optional<std::string>
{
    if (flag == COMPLETE_CONFIGURATINS_ONLY_FLAG)
    {
        info.complete_configurations_only = true;
        return std::nullopt;
    }

    if (flag == COUNT_FLAG)
    {
        info.count = true;
        return std::nullopt;
    }

    if (flag == INCOMPLETE_CONFIGURATINS_ONLY_FLAG)
    {
        info.incomplete_configurations_only = true;
        return std::nullopt;
    }

    if (flag == PORCELAIN_FLAG)
    {
        info.porcelain_output = true;
        return std::nullopt;
    }

    if (flag == SORTED_FLAG)
    {
        info.sorted_output = true;
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

static auto check_for_conflicting_flags(const ListCommandInfo& info,
                                        const std::string_view command_name) -> std::optional<std::string>
{
    if (info.complete_configurations_only && info.incomplete_configurations_only)
    {
        return std::format("Error: Both '{}' and '{}' flags were supplied to command '{}'.",
                           COMPLETE_CONFIGURATINS_ONLY_FLAG, INCOMPLETE_CONFIGURATINS_ONLY_FLAG, command_name);
    }

    if (info.count && info.porcelain_output)
    {
        return std::format("Error: Cannot provide '{}' flag to command '{}' with '{}' flag", PORCELAIN_FLAG,
                           command_name, COUNT_FLAG);
    }

    if (info.count && info.sorted_output)
    {
        return std::format("Error: Cannot provide '{}' flag to command '{}' when '{}' flag is provided.", SORTED_FLAG,
                           command_name, COUNT_FLAG);
    }

    return std::nullopt;
}

auto parse_list_command_arguments(std::span<const char* const> arguments) -> std::expected<ListCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "list").
    ASSERT(arguments.size() >= 2);
    const auto command_name       = std::string_view(arguments[1]);
    const auto relevant_arguments = std::span(arguments.begin() + 2, arguments.end());

    ListCommandInfo info{};

    for (const std::string_view argument : relevant_arguments)
    {
        if (!utils::is_flag(argument))
        {
            return std::unexpected(
                std::format("Error: Unknown argument '{}' provided to command '{}'.", argument, command_name));
        }

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

    const auto duplicate_flag        = utils::check_for_duplicate_flags(arguments);
    const auto duplicate_flag_exists = duplicate_flag.has_value();

    if (duplicate_flag_exists)
    {
        return std::unexpected(std::format("Error: Flag '{}' was provided to command '{}' more than once.",
                                           *duplicate_flag, command_name));
    }

    const auto conflicting_flags       = check_for_conflicting_flags(info, command_name);
    const auto conflicting_flags_exist = conflicting_flags.has_value();

    if (conflicting_flags_exist)
    {
        return std::unexpected(*conflicting_flags);
    }

    return info;
}

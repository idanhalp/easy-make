#include "source/argument_parsing/commands/list_configurations.hpp"

#include <algorithm>
#include <flat_set>
#include <format>
#include <string>
#include <string_view>

#include "source/argument_parsing/error_formatting.hpp"
#include "source/argument_parsing/utils.hpp"
#include "source/utils/macros/assert.hpp"

using namespace std::literals;

static const auto COMPLETE_CONFIGURATIONS_ONLY_FLAG   = "--complete-only"sv;
static const auto COUNT_FLAG                          = "--count"sv;
static const auto INCOMPLETE_CONFIGURATIONS_ONLY_FLAG = "--incomplete-only"sv;
static const auto PORCELAIN_FLAG                      = "--porcelain"sv;
static const auto SORTED_FLAG                         = "--sorted"sv;

static const std::flat_set FLAGS = {
    COMPLETE_CONFIGURATIONS_ONLY_FLAG,
    COUNT_FLAG,
    INCOMPLETE_CONFIGURATIONS_ONLY_FLAG,
    PORCELAIN_FLAG,
    SORTED_FLAG,
};

// Validates `flag` and updates `info` if recognized.
// Returns `std::nullopt` on success, or an error message otherwise.
static auto parse_flag(const std::string_view flag,
                       const std::string_view command_name,
                       ListConfigurationsCommandInfo& info) -> std::optional<std::string>
{
    if (flag == COMPLETE_CONFIGURATIONS_ONLY_FLAG)
    {
        info.complete_configurations_only = true;
        return std::nullopt;
    }

    if (flag == COUNT_FLAG)
    {
        info.count = true;
        return std::nullopt;
    }

    if (flag == INCOMPLETE_CONFIGURATIONS_ONLY_FLAG)
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

    // Make sure we did not forget to handle a valid flag.
    ASSERT(!FLAGS.contains(flag));

    return create_unknown_flag_error(command_name, flag, FLAGS);
}

static auto check_for_conflicting_flags(const ListConfigurationsCommandInfo& info,
                                        const std::string_view command_name) -> std::optional<std::string>
{
    if (info.complete_configurations_only && info.incomplete_configurations_only)
    {
        return create_conflicting_flags_error(
            command_name, COMPLETE_CONFIGURATIONS_ONLY_FLAG, INCOMPLETE_CONFIGURATIONS_ONLY_FLAG);
    }
    else if (info.count && info.porcelain_output)
    {
        return create_conflicting_flags_error(command_name, COUNT_FLAG, PORCELAIN_FLAG);
    }
    else if (info.count && info.sorted_output)
    {
        return create_conflicting_flags_error(command_name, COUNT_FLAG, SORTED_FLAG);
    }
    else
    {
        return std::nullopt;
    }
}

auto parse_list_configurations_command_arguments(std::span<const char* const> arguments)
    -> std::expected<ListConfigurationsCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "list-configs").
    ASSERT(arguments.size() >= 2);
    const auto command_name     = std::string_view(arguments[1]);
    const auto actual_arguments = std::span(arguments.begin() + 2, arguments.end());

    ASSERT(std::ranges::all_of(FLAGS, &utils::is_flag)); // Make sure all the flags are valid.
    ListConfigurationsCommandInfo info{};

    for (const std::string_view argument : actual_arguments)
    {
        if (!utils::is_flag(argument))
        {
            return std::unexpected(create_unknown_argument_error(command_name, argument, {}));
        }

        const auto flag_parse_error = parse_flag(argument, command_name, info);
        const auto flag_is_valid    = !flag_parse_error.has_value();

        if (!flag_is_valid)
        {
            return std::unexpected(*flag_parse_error);
        }
    }

    const auto duplicate_flag        = utils::check_for_duplicate_flags(actual_arguments);
    const auto duplicate_flag_exists = duplicate_flag.has_value();

    if (duplicate_flag_exists)
    {
        return std::unexpected(create_duplicate_flag_error(command_name, *duplicate_flag));
    }

    const auto conflicting_flags       = check_for_conflicting_flags(info, command_name);
    const auto conflicting_flags_exist = conflicting_flags.has_value();

    if (conflicting_flags_exist)
    {
        return std::unexpected(*conflicting_flags);
    }

    return info;
}

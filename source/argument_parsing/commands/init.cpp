#include "source/argument_parsing/commands/init.hpp"

#include <algorithm>
#include <flat_set>
#include <format>
#include <string_view>

#include "source/argument_parsing/error_formatting.hpp"
#include "source/argument_parsing/utils.hpp"
#include "source/utils/macros/assert.hpp"

using namespace std::literals;

static const auto OVERWRITE_FLAG = "--overwrite"sv;
static const auto QUIET_FLAG     = "--quiet"sv;

static const std::flat_set FLAGS = {
    OVERWRITE_FLAG,
    QUIET_FLAG,
};

// Validates `flag` and updates `info` if recognized.
// Returns `std::nullopt` on success, or an error message otherwise.
static auto parse_flag(const std::string_view flag,
                       const std::string_view command_name,
                       InitCommandInfo& info) -> std::optional<std::string>
{
    if (flag == OVERWRITE_FLAG)
    {
        info.overwrite_existing_configuration_file = true;
        return std::nullopt;
    }

    if (flag == QUIET_FLAG)
    {
        info.is_quiet = true;
        return std::nullopt;
    }

    // Make sure we did not forget to handle a valid flag.
    ASSERT(!FLAGS.contains(flag));

    return create_unknown_flag_error(command_name, flag, FLAGS);
}

auto parse_init_command_arguments(const std::span<const char* const> arguments)
    -> std::expected<InitCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "init").
    ASSERT(arguments.size() >= 2);
    const auto command_name     = std::string_view(arguments[1]);
    const auto actual_arguments = std::span(arguments.begin() + 2, arguments.end());

    ASSERT(std::ranges::all_of(FLAGS, &utils::is_flag)); // Make sure all the flags are valid.
    InitCommandInfo info{};

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

    return info;
}

#include "source/argument_parsing/commands/list_files.hpp"

#include <algorithm>
#include <flat_set>
#include <format>
#include <string>
#include <string_view>

#include "source/argument_parsing/error_formatting.hpp"
#include "source/argument_parsing/utils.hpp"
#include "source/utils/macros/assert.hpp"

using namespace std::literals;

static const auto COUNT_FLAG             = "--count"sv;
static const auto HEADER_FILES_ONLY_FLAG = "--header-only"sv;
static const auto PORCELAIN_FLAG         = "--porcelain"sv;
static const auto SOURCE_FILES_ONLY_FLAG = "--source-only"sv;

static const std::flat_set FLAGS = {
    COUNT_FLAG,
    HEADER_FILES_ONLY_FLAG,
    PORCELAIN_FLAG,
    SOURCE_FILES_ONLY_FLAG,
};

// Validates `flag` and updates `info` if recognized.
// Returns `std::nullopt` on success, or an error message otherwise.
static auto parse_flag(const std::string_view flag,
                       const std::string_view command_name,
                       ListFilesCommandInfo& info) -> std::optional<std::string>
{
    if (flag == COUNT_FLAG)
    {
        info.count = true;
        return std::nullopt;
    }

    if (flag == HEADER_FILES_ONLY_FLAG)
    {
        info.header_only = true;
        return std::nullopt;
    }

    if (flag == PORCELAIN_FLAG)
    {
        info.porcelain_output = true;
        return std::nullopt;
    }

    if (flag == SOURCE_FILES_ONLY_FLAG)
    {
        info.source_only = true;
        return std::nullopt;
    }

    // Make sure we did not forget to handle a valid flag.
    ASSERT(!FLAGS.contains(flag));

    return create_unknown_flag_error(command_name, flag, FLAGS);
}

static auto check_for_conflicting_flags(const ListFilesCommandInfo& info,
                                        const std::string_view command_name) -> std::optional<std::string>
{
    if (info.header_only && info.source_only)
    {
        return std::format("Error: Both '{}' and '{}' flags were supplied to command '{}'.",
                           HEADER_FILES_ONLY_FLAG,
                           SOURCE_FILES_ONLY_FLAG,
                           command_name);
    }

    if (info.count && info.porcelain_output)
    {
        return std::format("Error: Cannot provide '{}' flag to command '{}' when '{}' flag is provided.",
                           PORCELAIN_FLAG,
                           command_name,
                           COUNT_FLAG);
    }

    return std::nullopt;
}

auto parse_list_files_command_arguments(std::span<const char* const> arguments)
    -> std::expected<ListFilesCommandInfo, std::string>
{
    // The first 2 elements are the program name and the command (which is "list-files").
    ASSERT(arguments.size() >= 2);
    const auto command_name     = std::string_view(arguments[1]);
    const auto actual_arguments = std::span(arguments.begin() + 2, arguments.end());

    ASSERT(std::ranges::all_of(FLAGS, &utils::is_flag)); // Make sure all the flags are valid.
    ListFilesCommandInfo info{};
    auto configuration_name_provided = false;

    for (const std::string_view argument : actual_arguments)
    {
        if (utils::is_flag(argument))
        {
            const auto flag_parse_error = parse_flag(argument, command_name, info);
            const auto flag_is_valid    = !flag_parse_error.has_value();

            if (!flag_is_valid)
            {
                return std::unexpected(*flag_parse_error);
            }

            continue;
        }

        // `argument` is a configuration name.
        // If `configuration_name_provided` was already set before handling the current argument,
        // it means that multiple configuration names were provided.
        const auto multiple_configuration_names_provided = configuration_name_provided;

        if (multiple_configuration_names_provided)
        {
            const auto& name_1 = info.configuration_name;
            const auto& name_2 = argument;

            return std::unexpected(create_multiple_configuration_names_error(command_name, name_1, name_2));
        }

        info.configuration_name     = argument;
        configuration_name_provided = true;
    }

    if (!configuration_name_provided)
    {
        return std::unexpected(create_missing_configuration_name_error(command_name));
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

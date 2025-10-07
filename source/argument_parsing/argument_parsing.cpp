#include "source/argument_parsing/argument_parsing.hpp"

#include <format>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

#include "source/utils/find_closest_word.hpp"

static const auto CLEAN_FLAG         = "--clean";
static const auto CLEAN_ALL_FLAG     = "--clean-all";
static const auto PRINT_VERSION_FLAG = "--version";

ArgumentInfo::ArgumentInfo()
    : configuration_name(""), clean_configuration(false), clean_all_configurations(false), print_version(false)
{
}

static auto check_for_incompatible_flags(const ArgumentInfo& argument_info) -> std::optional<std::string>
{
    std::vector<std::string_view> flags;

    if (argument_info.clean_configuration)
    {
        flags.push_back(CLEAN_FLAG);
    }

    if (argument_info.clean_all_configurations)
    {
        flags.push_back(CLEAN_ALL_FLAG);
    }

    if (argument_info.print_version)
    {
        flags.push_back(PRINT_VERSION_FLAG);
    }

    // If there exist two or more flags that are incompatible with each other,
    // return an error about some arbitrary pair.
    if (flags.size() >= 2)
    {
        return std::format("Error: Flags '{}' and '{}' cannot be used together.", flags[0], flags[1]);
    }
    else
    {
        return std::nullopt;
    }
}

static auto
check_for_flags_incompatible_with_name_argument(const bool configuration_name_specified,
                                                const ArgumentInfo& argument_info) -> std::optional<std::string>
{
    if (configuration_name_specified && argument_info.clean_all_configurations)
    {
        return std::format("Error: Cannot specify a configuration name ('{}') when '{}' is used.",
                           argument_info.configuration_name, CLEAN_ALL_FLAG);
    }
    else if (configuration_name_specified && argument_info.print_version)
    {
        return std::format("Error: Cannot specify a configuration name ('{}') when '{}' is used.",
                           argument_info.configuration_name, PRINT_VERSION_FLAG);
    }
    else if (!configuration_name_specified && argument_info.clean_configuration)
    {
        return std::format("Error: Must specify a configuration name when '{}' is used.", CLEAN_FLAG);
    }
    else if (!configuration_name_specified && !argument_info.clean_configuration &&
             !argument_info.clean_all_configurations && !argument_info.print_version)
    {
        return "Error: Must specify a configuration name when building.";
    }
    else
    {
        return std::nullopt;
    }
}

auto parse_arguments(const std::span<const char* const> arguments) -> std::expected<ArgumentInfo, std::string>
{
    ArgumentInfo argument_info;
    auto name_already_specified = false;

    for (const std::string_view argument : arguments | std::views::drop(1)) // First argument is the program's name.
    {
        const auto is_flag                        = argument.starts_with("--");
        const auto argument_is_configuration_name = !is_flag;

        if (argument_is_configuration_name)
        {
            if (name_already_specified)
            {
                const auto& name_1 = argument_info.configuration_name;
                const auto& name_2 = argument;

                return std::unexpected(
                    std::format("Error: More than one configuration name specified ('{}' and '{}')", name_1, name_2));
            }

            argument_info.configuration_name = argument;
            name_already_specified           = true;
        }
        else if (argument == CLEAN_FLAG)
        {
            const auto already_specified_clean_flag = argument_info.clean_configuration;

            if (already_specified_clean_flag)
            {
                return std::unexpected(std::format("Error: Flag '{}' was specified more than once.", CLEAN_FLAG));
            }

            argument_info.clean_configuration = true;
        }
        else if (argument == CLEAN_ALL_FLAG)
        {
            const auto already_specified_clean_all_flag = argument_info.clean_all_configurations;

            if (already_specified_clean_all_flag)
            {
                return std::unexpected(std::format("Error: Flag '{}' was specified more than once.", CLEAN_ALL_FLAG));
            }

            argument_info.clean_all_configurations = true;
        }
        else if (argument == PRINT_VERSION_FLAG)
        {
            const auto already_specified_print_version_flag = argument_info.print_version;

            if (already_specified_print_version_flag)
            {
                return std::unexpected(
                    std::format("Error: Flag '{}' was specified more than once.", PRINT_VERSION_FLAG));
            }

            argument_info.print_version = true;
        }
        else
        {
            const std::vector<std::string> FLAGS = {CLEAN_FLAG, CLEAN_ALL_FLAG, PRINT_VERSION_FLAG};
            const auto closest_flag              = utils::find_closest_word(std::string(argument), FLAGS);
            const auto error_message =
                closest_flag.has_value()
                    ? std::format("Error: Unknown argument '{}'. Did you mean '{}'?", argument, *closest_flag)
                    : std::format("Error: Unknown argument '{}'.", argument);

            return std::unexpected(error_message);
        }
    }

    const auto incompatible_flags_error = check_for_incompatible_flags(argument_info);
    const auto incompatible_flags_exist = incompatible_flags_error.has_value();

    if (incompatible_flags_exist)
    {
        return std::unexpected(*incompatible_flags_error);
    }

    const auto flag_incompatible_with_name_error =
        check_for_flags_incompatible_with_name_argument(name_already_specified, argument_info);
    const auto flag_incompatible_with_name_exists = flag_incompatible_with_name_error.has_value();

    if (flag_incompatible_with_name_exists)
    {
        return std::unexpected(*flag_incompatible_with_name_error);
    }

    return argument_info;
}

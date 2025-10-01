#include "source/argument_parsing/argument_parsing.hpp"

#include <format>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

static const std::string_view CLEAN_FLAG         = "--clean";
static const std::string_view CLEAN_ALL_FLAG     = "--clean-all";
static const std::string_view PRINT_VERSION_FLAG = "--version";

ArgumentInfo::ArgumentInfo()
    : configuration_name("default"), clean_configuration(false), clean_all_configurations(false), print_version(false)
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
    if (!configuration_name_specified)
    {
        return std::nullopt;
    }
    else if (argument_info.clean_all_configurations)
    {
        return std::format("Error: Cannot specify a configuration name ('{}') when '{}' is used.",
                           argument_info.configuration_name, CLEAN_ALL_FLAG);
    }
    else if (argument_info.print_version)
    {
        return std::format("Error: Cannot specify a configuration name ('{}') when '{}' is used.",
                           argument_info.configuration_name, PRINT_VERSION_FLAG);
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
            return std::unexpected(std::format("Error: Unknown argument '{}'.", argument));
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

#include "source/argument_parsing/argument_parsing.hpp"

#include <format>
#include <ranges>
#include <string_view>

static const auto CLEAN_FLAG         = "--clean";
static const auto PRINT_VERSION_FLAG = "--version";

ArgumentInfo::ArgumentInfo() : configuration_name("default"), clean_object_files(false), print_version(false) {}

auto parse_arguments(const std::span<const char* const> arguments) -> std::expected<ArgumentInfo, std::string>
{
    ArgumentInfo argument_info;
    auto name_already_supplied = false;

    for (const std::string_view argument : arguments | std::views::drop(1)) // First argument is the program's name.
    {
        const auto is_flag                        = argument.starts_with("--");
        const auto argument_is_configuration_name = !is_flag;

        if (argument_is_configuration_name)
        {
            if (name_already_supplied)
            {
                const auto& name_1 = argument_info.configuration_name;
                const auto& name_2 = argument;

                return std::unexpected(
                    std::format("Error: More than one configuration name supplied ('{}' and '{}')", name_1, name_2));
            }

            argument_info.configuration_name = argument;
            name_already_supplied            = true;
        }
        else if (argument == CLEAN_FLAG)
        {
            const auto already_supplied_clean_flag = argument_info.clean_object_files;

            if (already_supplied_clean_flag)
            {
                return std::unexpected(std::format("Error: Flag '{}' was supplied more than once.", CLEAN_FLAG));
            }

            argument_info.clean_object_files = true;
        }
        else if (argument == PRINT_VERSION_FLAG)
        {
            const auto already_supplied_print_version_flag = argument_info.clean_object_files;

            if (already_supplied_print_version_flag)
            {
                return std::unexpected(
                    std::format("Error: Flag '{}' was supplied more than once.", PRINT_VERSION_FLAG));
            }

            argument_info.print_version = true;
        }
        else
        {
            return std::unexpected(std::format("Error: Unknown argument '{}'.", argument));
        }
    }

    return argument_info;
}

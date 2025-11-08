#include <cstdlib>
#include <filesystem>
#include <type_traits>
#include <vector>

#include "source/argument_parsing/argument_parsing.hpp"
#include "source/commands/build/build.hpp"
#include "source/commands/clean/clean.hpp"
#include "source/commands/clean_all/clean_all.hpp"
#include "source/commands/list/list.hpp"
#include "source/commands/print_version/print_version.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

auto main(const int num_of_arguments, const char* arguments[]) -> int
{
    const auto current_path              = std::filesystem::absolute(std::filesystem::current_path());
    const auto configuration_file_exists = utils::check_if_configurations_file_exists(current_path);

    if (!configuration_file_exists)
    {
        utils::print_error("The file '{}' could not be located in '{}'.",
                           params::CONFIGURATIONS_FILE_NAME.native(),
                           current_path.native());

        return EXIT_FAILURE;
    }

    const auto configurations              = parse_configurations(current_path);
    const auto configuration_file_is_valid = configurations.has_value();

    if (!configuration_file_is_valid)
    {
        utils::print_error("{}", configurations.error());

        return EXIT_FAILURE;
    }

    const auto command_info        = parse_arguments(std::span(arguments, num_of_arguments));
    const auto arguments_are_valid = command_info.has_value();

    if (!arguments_are_valid)
    {
        utils::print_error("{}", command_info.error());

        return EXIT_FAILURE;
    }

    const auto exit_code = std::visit(
        [&](auto&& info)
        {
            using CommandType = std::decay_t<decltype(info)>;

            if constexpr (std::is_same_v<CommandType, BuildCommandInfo>)
            {
                return commands::build(info, *configurations, current_path);
            }
            else if constexpr (std::is_same_v<CommandType, CleanCommandInfo>)
            {
                return commands::clean(info, *configurations, current_path);
            }
            else if constexpr (std::is_same_v<CommandType, CleanAllCommandInfo>)
            {
                return commands::clean_all(info, *configurations, current_path);
            }
            else if constexpr (std::is_same_v<CommandType, ListCommandInfo>)
            {
                return commands::list(info, *configurations);
            }
            else if constexpr (std::is_same_v<CommandType, PrintVersionCommandInfo>)
            {
                return commands::print_version(info);
            }
        },
        *command_info);

    return exit_code;
}

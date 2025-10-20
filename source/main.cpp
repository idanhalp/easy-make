#include <cstdlib>
#include <filesystem>
#include <vector>

#include "source/argument_parsing/argument_parsing.hpp"
#include "source/commands/clean/clean.hpp"
#include "source/commands/clean_all/clean_all.hpp"
#include "source/commands/print_version/print_version.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/executable_creation/executable_creation.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

auto main(const int num_of_arguments, const char* arguments[]) -> int
{
    const auto current_path              = std::filesystem::absolute(std::filesystem::current_path());
    const auto configuration_file_exists = utils::check_if_configurations_file_exists(current_path);

    if (!configuration_file_exists)
    {
        utils::print_error("The file '{}' could not be located in '{}'.", params::CONFIGURATIONS_FILE_NAME.native(),
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

    const auto argument_info       = parse_arguments(std::span(arguments, num_of_arguments));
    const auto arguments_are_valid = argument_info.has_value();

    if (!arguments_are_valid)
    {
        utils::print_error("{}", argument_info.error());

        return EXIT_FAILURE;
    }

    if (argument_info->clean_configuration)
    {
        return commands::clean(argument_info->configuration_name, *configurations, current_path);
    }
    if (argument_info->clean_all_configurations)
    {
        return commands::clean_all(*configurations, current_path);
    }
    else if (argument_info->print_version)
    {
        return commands::print_version();
    }
    else
    {
        return create_executable(argument_info->configuration_name, current_path, *configurations);
    }
}

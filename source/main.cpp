#include <cstdlib>
#include <filesystem>
#include <print>
#include <vector>

#include "source/argument_parsing/argument_parsing.hpp"
#include "source/commands/clean/clean.hpp"
#include "source/commands/print_version/print_version.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/executable_creation/executable_creation.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/utils.hpp"

auto main(const int num_of_arguments, const char* arguments[]) -> int
{
    const auto current_path              = std::filesystem::absolute(std::filesystem::current_path());
    const auto configuration_file_exists = utils::check_if_configurations_file_exists(current_path);

    if (!configuration_file_exists)
    {
        std::println("The file '{}' could not be located in '{}'.", params::CONFIGURATIONS_FILE_NAME.native(),
                     current_path.string());

        return EXIT_FAILURE;
    }

    const auto configurations      = parse_configurations(current_path);
    const auto argument_info       = parse_arguments(std::span(arguments, num_of_arguments));
    const auto arguments_are_valid = argument_info.has_value();

    if (!arguments_are_valid)
    {
        std::println("{}", argument_info.error());

        return EXIT_FAILURE;
    }

    if (argument_info->clean_object_files)
    {
        return commands::clean(argument_info->configuration_name, current_path);
    }
    else if (argument_info->print_version)
    {
        return commands::print_version();
    }
    else
    {
        return create_executable(argument_info->configuration_name, current_path, configurations);
    }

    return EXIT_SUCCESS;
}

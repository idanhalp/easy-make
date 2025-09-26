#include <cstdlib>
#include <filesystem>
#include <print>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/executable_creation/executable_creation.hpp"
#include "source/utils/utils.hpp"

auto main(const int num_of_args, const char* arguments[]) -> int
{
    const auto current_path              = std::filesystem::absolute(std::filesystem::current_path());
    const auto configuration_file_exists = utils::check_if_configurations_file_exists(current_path);

    if (!configuration_file_exists)
    {
        std::println("The file '{}' could not be located in '{}'.", utils::CONFIGURATIONS_FILE_NAME.native(),
                     current_path.string());

        return EXIT_FAILURE;
    }

    const auto configurations       = parse_configurations(current_path);
    const auto chosen_configuration = utils::get_chosen_configuration(std::vector(arguments, arguments + num_of_args));
    const auto arguments_are_valid  = chosen_configuration.has_value();

    if (!arguments_are_valid)
    {
        // Print something...
        return EXIT_FAILURE;
    }

    const auto executable_creation_error_message =
        create_executable(*chosen_configuration, current_path, configurations);
    const auto created_executable_successfully = !executable_creation_error_message.has_value();

    if (!created_executable_successfully)
    {
        std::println("{}", *executable_creation_error_message);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

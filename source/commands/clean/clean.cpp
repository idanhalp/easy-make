#include "source/commands/clean/clean.hpp"

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <string_view>
#include <vector>

#include "source/executable_creation/executable_creation.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/print.hpp"

static auto print_incomplete_configuration_error_message(const std::string_view error_message,
                                                         const bool verbose) -> void
{
    if (verbose)
    {
        utils::print_error("{} Cannot delete an incomplete configuration.", error_message);
    }
}

static auto print_clean_failure_error_message(const std::string& configuration_name,
                                              const std::vector<Configuration>& configurations,
                                              const bool verbose) -> void
{
    if (!verbose)
    {
        return;
    }

    const auto get_name            = [](const Configuration& configuration) { return *configuration.name; };
    const auto configuration_names = configurations | std::views::transform(get_name) | std::ranges::to<std::vector>();
    const auto closest_name        = utils::find_closest_word(std::string(configuration_name), configuration_names);
    const auto found_closer_name =
        !std::ranges::contains(configuration_names, configuration_name) && closest_name.has_value();

    if (found_closer_name)
    {
        utils::print_error("Error: Nothing to delete for configuration '{}'. Did you mean '{}'?", configuration_name,
                           *closest_name);
    }
    else
    {
        utils::print_error("Error: Nothing to delete for configuration '{}'.", configuration_name);
    }
}

auto commands::clean(const std::string& configuration_name,
                     const std::vector<Configuration>& configurations,
                     const std::filesystem::path& path_to_root,
                     const bool verbose) -> int
{
    const auto path_to_object_files_directory = path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name;
    const auto build_directory_exists         = std::filesystem::remove_all(path_to_object_files_directory);
    auto executable_deleted                   = false;

    // Find the relevant configuration and delete the executable.

    for (const auto& configuration : configurations)
    {
        ASSERT(configuration.name.has_value());

        if (*configuration.name != configuration_name)
        {
            continue;
        }

        const auto actual_configuration      = get_actual_configuration(*configuration.name, configurations);
        const auto configuration_is_complete = actual_configuration.has_value();

        if (!configuration_is_complete)
        {
            print_incomplete_configuration_error_message(actual_configuration.error(), verbose);

            return EXIT_FAILURE;
        }

        ASSERT(actual_configuration->output_name.has_value());

        const auto output_path = actual_configuration->output_path.value_or("");
        const auto path_to_executable =
            std::filesystem::path(path_to_root) / output_path / *actual_configuration->output_name;

        executable_deleted = std::filesystem::remove(path_to_executable);
        break;
    }

    if (build_directory_exists || executable_deleted)
    {
        return EXIT_SUCCESS;
    }
    else
    {
        print_clean_failure_error_message(configuration_name, configurations, verbose);

        return EXIT_FAILURE;
    }
}

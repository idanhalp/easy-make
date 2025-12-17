#include "source/commands/clean/clean.hpp"

#include <print>
#include <string_view>

#include "source/commands/build/configuration_resolution.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/print.hpp"

auto commands::clean(const CleanCommandInfo& info,
                     const std::vector<Configuration>& configurations,
                     const std::filesystem::path& path_to_root) -> int
{
    const auto configuration_to_delete          = get_resolved_configuration(configurations, info.configuration_name);
    const auto configuration_to_delete_is_valid = configuration_to_delete.has_value();

    if (!configuration_to_delete_is_valid)
    {
        if (!info.is_quiet)
        {
            const std::string_view ERROR_PREFIX = "Error: ";
            std::string_view error              = configuration_to_delete.error();

            if (error.starts_with(ERROR_PREFIX))
            {
                error.remove_prefix(ERROR_PREFIX.length());
            }

            utils::print_error("Command 'clean' failed: {}", error);
        }

        return EXIT_FAILURE;
    }

    // Remove object files.
    const auto path_to_object_files_directory = path_to_root / params::BUILD_DIRECTORY_NAME / info.configuration_name;
    const auto build_directory_deleted        = std::filesystem::remove_all(path_to_object_files_directory);

    // Remove executable.
    const auto output_path        = configuration_to_delete->output_path.value_or("");
    const auto path_to_executable = path_to_root / output_path / *configuration_to_delete->output_name;
    const auto executable_deleted = std::filesystem::remove(path_to_executable);

    if (info.is_quiet)
    {
        return (build_directory_deleted || executable_deleted) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    if (!build_directory_deleted && !executable_deleted)
    {
        utils::print_error("Nothing to clean for configuration '{}'.", info.configuration_name);

        return EXIT_FAILURE;
    }

    if (build_directory_deleted)
    {
        std::println("Deleted object files for configuration '{}'.", info.configuration_name);
    }

    if (executable_deleted)
    {
        std::println("Deleted executable for configuration '{}'.", info.configuration_name);
    }

    return EXIT_SUCCESS;
}

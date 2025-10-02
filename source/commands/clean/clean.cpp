#include "source/commands/clean/clean.hpp"

#include <filesystem>

#include "source/parameters/parameters.hpp"

auto commands::clean(const std::string_view configuration_name,
                     const std::vector<Configuration>& configurations,
                     const std::filesystem::path& path_to_root) -> int
{
    const auto path_to_object_files_directory = path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name;
    const auto directory_exists               = std::filesystem::remove_all(path_to_object_files_directory);

    // Find the relevant configuration and delete the executable.

    for (const auto& configuration : configurations)
    {
        if (*configuration.name != configuration_name)
        {
            continue;
        }

        const auto path_to_executable =
            configuration.output_path.has_value()
                ? std::filesystem::path(*configuration.output_path) / *configuration.output_path
                : std::filesystem::path(".") / *configuration.output_name;

        std::filesystem::remove(path_to_executable);
        break;
    }

    return directory_exists ? EXIT_SUCCESS : EXIT_FAILURE;
}

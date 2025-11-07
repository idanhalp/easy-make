#include "source/commands/clean_all/clean_all.hpp"

#include <filesystem>

#include "source/parameters/parameters.hpp"

auto commands::clean_all([[maybe_unused]] const CleanAllCommandInfo& info,
                         const std::vector<Configuration>& configurations,
                         const std::filesystem::path& path_to_root) -> int
{
    // Currently `info` is unused.
    // Adding it as a parameter enables us to easily
    // add arguments and flags in the future.

    const auto path_to_build_directory = path_to_root / params::BUILD_DIRECTORY_NAME;
    const auto directory_exists        = std::filesystem::remove_all(path_to_build_directory);

    for (const auto& configuration : configurations)
    {
        if (!configuration.output_name.has_value())
        {
            continue;
        }

        const auto output_path        = configuration.output_path.value_or("");
        const auto path_to_executable = std::filesystem::path(path_to_root) / output_path / *configuration.output_name;

        std::filesystem::remove(path_to_executable);
    }

    return directory_exists ? EXIT_SUCCESS : EXIT_FAILURE;
}

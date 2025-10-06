#include "source/commands/clean_all/clean_all.hpp"

#include <filesystem>

#include "source/parameters/parameters.hpp"

auto commands::clean_all(const std::vector<Configuration>& configurations,
                         const std::filesystem::path& path_to_root) -> int
{
    const auto path_to_build_directory = path_to_root / params::BUILD_DIRECTORY_NAME;
    const auto directory_exists        = std::filesystem::remove_all(path_to_build_directory);

    for (const auto& configuration : configurations)
    {
        if (!configuration.output_name.has_value())
        {
            continue;
        }

        const auto path_to_executable =
            configuration.output_path.has_value()
                ? std::filesystem::path(*configuration.output_path) / *configuration.output_path
                : std::filesystem::path(".") / *configuration.output_name;

        std::filesystem::remove(path_to_executable);
    }

    return directory_exists ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include "source/commands/clean_all/clean_all.hpp"

#include <filesystem>
#include <print>

#include "source/parameters/parameters.hpp"

auto commands::clean_all(const CleanAllCommandInfo& info,
                         const std::vector<Configuration>& configurations,
                         const std::filesystem::path& path_to_root) -> int
{
    const auto path_to_build_directory = path_to_root / params::BUILD_DIRECTORY_NAME;
    const auto build_directory_exists  = std::filesystem::remove_all(path_to_build_directory);

    if (build_directory_exists && !info.is_quiet)
    {
        std::println("Removed '{}'.", params::BUILD_DIRECTORY_NAME.native());
    }

    for (const auto& configuration : configurations)
    {
        if (!configuration.output_name.has_value())
        {
            continue;
        }

        const auto output_path        = configuration.output_path.value_or("");
        const auto path_to_executable = std::filesystem::path(path_to_root) / output_path / *configuration.output_name;
        const auto executable_removed = std::filesystem::remove(path_to_executable);

        if (executable_removed && !info.is_quiet)
        {
            std::println("Removed '{}'.", path_to_executable.native());
        }
    }

    return build_directory_exists ? EXIT_SUCCESS : EXIT_FAILURE;
}

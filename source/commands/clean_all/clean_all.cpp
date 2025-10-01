#include "source/commands/clean_all/clean_all.hpp"

#include <filesystem>

#include "source/parameters/parameters.hpp"

auto commands::clean_all(const std::filesystem::path& path_to_root) -> int
{
    const auto path_to_build_directory = path_to_root / params::BUILD_DIRECTORY_NAME;
    const auto directory_exists        = std::filesystem::remove_all(path_to_build_directory);

    return directory_exists ? EXIT_SUCCESS : EXIT_FAILURE;
}

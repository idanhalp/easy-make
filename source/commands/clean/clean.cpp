#include "source/commands/clean/clean.hpp"

#include <filesystem>

#include "source/parameters/parameters.hpp"

auto commands::clean(const std::string_view configuration_name, const std::filesystem::path& path_to_root) -> int
{
    const auto path_to_object_files_directory = path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name;
    const auto directory_exists               = std::filesystem::remove_all(path_to_object_files_directory);

    return directory_exists ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include "source/commands/init/init.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <print>

#include "source/parameters/parameters.hpp"

static const auto default_configuration_file_content = R"(
[
  {
    "name": "example-config",
    "compiler": "g++",
    "standard": "23",
    "warnings": ["-Wall", "-Wextra", "-Werror"],
    "optimization": "2",
    "sources": {
      "directories": ["."]
    },
    "output": {
      "name": "easy-make"
    }
  }
]
)";

static auto print_status(const InitCommandInfo& info,
                         const std::filesystem::path& path_to_root,
                         const bool configurations_file_exists) -> void
{
    if (info.is_quiet)
    {
        return;
    }
    else if (!configurations_file_exists)
    {
        std::println(
            "Creating a new '{}' file at '{}'.", params::CONFIGURATIONS_FILE_NAME.native(), path_to_root.native());
    }
    else if (info.overwrite_existing_configuration_file)
    {
        std::println("Overwriting existing '{}' file at '{}'.",
                     params::CONFIGURATIONS_FILE_NAME.native(),
                     path_to_root.native());
    }
    else
    {
        std::println(
            "'{}' file already exists at '{}'.", params::CONFIGURATIONS_FILE_NAME.native(), path_to_root.native());
    }
}

auto commands::init(const InitCommandInfo& info, const std::filesystem::path& path_to_root) -> int
{
    const auto configurations_file_path   = path_to_root / params::CONFIGURATIONS_FILE_NAME;
    const auto configurations_file_exists = std::filesystem::exists(configurations_file_path);

    print_status(info, path_to_root, configurations_file_exists);

    const auto should_create_configurations_file =
        !configurations_file_exists || info.overwrite_existing_configuration_file;

    if (!should_create_configurations_file)
    {
        return EXIT_FAILURE;
    }

    auto configurations_file = std::ofstream(configurations_file_path);
    configurations_file << default_configuration_file_content;

    return EXIT_SUCCESS;
}

#include "source/executable_creation/executable_creation.hpp"

#include <algorithm>
#include <cstdlib> // `std::system`
#include <format>
#include <iterator>
#include <print>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

#include "source/build_caching/build_caching.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/utils.hpp"

auto get_actual_configuration(std::string_view configuration_name, const std::vector<Configuration>& configurations)
    -> std::expected<Configuration, std::string>
{
    const auto original_configuration = std::ranges::find_if(
        configurations, [&](const Configuration& configuration) { return configuration.name == configuration_name; });
    const auto configuration_exists = original_configuration != configurations.end();

    if (!configuration_exists)
    {
        return std::unexpected(std::format("'{}' does not contain a configuration named '{}'.",
                                           params::CONFIGURATIONS_FILE_NAME.native(), configuration_name));
    }

    const auto default_configuration = std::ranges::find_if(
        configurations, [&](const Configuration& configuration) { return configuration.name == "default"; });
    const auto default_configuration_exists = default_configuration != configurations.end();
    auto actual_configuration               = *original_configuration;

    if (!original_configuration->compiler.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->compiler.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.compiler = default_configuration->compiler;
        }
        else
        {
            return std::unexpected(std::format("Could not resolve 'compiler' for '{}'.", configuration_name));
        }
    }

    if (!original_configuration->standard.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->standard.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.standard = default_configuration->standard;
        }
    }

    if (!original_configuration->warnings.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->warnings.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.warnings = default_configuration->warnings;
        }
    }

    if (!original_configuration->optimization.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->optimization.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.optimization = default_configuration->optimization;
        }
    }

    if (!original_configuration->defines.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->defines.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.defines = default_configuration->defines;
        }
    }

    if (!original_configuration->include_directories.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->include_directories.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.include_directories = default_configuration->include_directories;
        }
    }

    if (!original_configuration->source_files.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->source_files.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.source_files = default_configuration->source_files;
        }
    }

    if (!original_configuration->source_directories.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->source_directories.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.source_directories = default_configuration->source_directories;
        }
    }

    if (!original_configuration->excluded_files.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->excluded_files.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.excluded_files = default_configuration->excluded_files;
        }
    }

    if (!original_configuration->excluded_directories.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->excluded_directories.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.excluded_directories = default_configuration->excluded_directories;
        }
    }

    if (!original_configuration->output_name.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->output_name.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.output_name = default_configuration->output_name;
        }
        else
        {
            return std::unexpected(std::format("Could not resolve 'output.name' for '{}'.", configuration_name));
        }
    }

    if (!original_configuration->output_path.has_value())
    {
        const auto should_fall_back_to_default_value =
            default_configuration_exists && default_configuration->output_path.has_value();

        if (should_fall_back_to_default_value)
        {
            actual_configuration.output_path = default_configuration->output_path;
        }
    }

    return actual_configuration;
}

auto get_source_files(const Configuration& configuration,
                      const std::filesystem::path& path_to_root) -> std::vector<std::filesystem::path>
{
    std::unordered_set<std::filesystem::path> files_to_compile;

    if (configuration.source_files.has_value())
    {
        for (const auto& file : *configuration.source_files)
        {
            const auto full_path_to_file = path_to_root / file;

            if (std::filesystem::exists(full_path_to_file))
            {
                files_to_compile.insert(file);
            }
        }
    }

    if (configuration.source_directories.has_value())
    {
        for (const auto& directory : *configuration.source_directories)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path_to_root / directory))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                const auto is_source_file = entry.path().extension() == ".cpp" || entry.path().extension() == ".cc" ||
                                            entry.path().extension() == ".cxx";

                if (is_source_file)
                {
                    auto relative_path = std::filesystem::relative(entry, path_to_root);
                    files_to_compile.insert(std::move(relative_path));
                }
            }
        }
    }

    if (configuration.excluded_files.has_value())
    {
        for (const auto& file : *configuration.excluded_files)
        {
            files_to_compile.erase(file);
        }
    }

    if (configuration.excluded_directories.has_value())
    {
        for (const auto& directory : *configuration.excluded_directories)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path_to_root / directory))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                const auto is_source_file = entry.path().extension() == ".cpp" || entry.path().extension() == ".cc" ||
                                            entry.path().extension() == ".cxx";

                if (is_source_file)
                {
                    auto relative_path = std::filesystem::relative(entry, path_to_root);
                    files_to_compile.erase(std::move(relative_path));
                }
            }
        }
    }

    return files_to_compile | std::ranges::to<std::vector>();
}

auto create_compilation_flags_string(const Configuration& configuration) -> std::string
{
    std::string result;

    if (configuration.standard.has_value())
    {
        std::format_to(std::back_inserter(result), "-std={} ", *configuration.standard);
    }

    if (configuration.warnings.has_value())
    {
        for (const auto& warning : *configuration.warnings)
        {
            std::format_to(std::back_inserter(result), "{} ", warning);
        }
    }

    if (configuration.optimization.has_value())
    {
        std::format_to(std::back_inserter(result), "{} ", *configuration.optimization);
    }

    if (configuration.defines.has_value())
    {
        for (const auto& define : *configuration.defines)
        {
            std::format_to(std::back_inserter(result), "-D{} ", define);
        }
    }

    if (configuration.include_directories.has_value())
    {
        for (const auto& directory : *configuration.include_directories)
        {
            std::format_to(std::back_inserter(result), "-I{} ", directory);
        }
    }

    result.pop_back(); // remove trailing whitespace.

    return result;
}

static auto
create_object_files(const Configuration& configuration,
                    const std::filesystem::path& path_to_root,
                    const std::vector<std::filesystem::path>& files_to_compile) -> std::optional<std::string>
{
    const auto compilation_flags = create_compilation_flags_string(configuration);
    const auto object_files_path = path_to_root / params::BUILD_DIRECTORY_NAME / *configuration.name;

    switch (files_to_compile.size())
    {
    case 0:
        std::println("No files to compile.");
        break;

    case 1:
        std::println("Compiling one file.");
        break;

    default:
        std::println("Compiling {} files.", files_to_compile.size());
        break;
    }

    for (const auto& [index, file_name] : std::views::enumerate(files_to_compile) | std::views::as_const)
    {
        const auto object_file_name    = utils::get_object_file_name(file_name);
        const auto compilation_command = std::format("{} {} -c {} -o {}/{}", *configuration.compiler, compilation_flags,
                                                     file_name.native(), object_files_path.native(), object_file_name);

        std::println("{}) Compiling '{}'.", index + 1, file_name.native());

        const auto compiled_successfully = std::system(compilation_command.c_str()) == EXIT_SUCCESS;

        if (!compiled_successfully)
        {
            return std::format("Compilation of '{}' failed.", file_name.native());
        }
    }

    return std::nullopt;
}

static auto link_object_files(const Configuration& configuration,
                              const std::filesystem::path& path_to_root) -> std::optional<std::string>
{
    const auto actual_output_path = configuration.output_path.has_value()
                                        ? std::format("{}/{}", *configuration.output_path, *configuration.output_name)
                                        : *configuration.output_name;

    const auto object_files_path = path_to_root / params::BUILD_DIRECTORY_NAME / *configuration.name;
    const auto link_command =
        std::format("{} {}/*.o -o {}", *configuration.compiler, object_files_path.native(), actual_output_path);

    if (configuration.output_path.has_value())
    {
        std::filesystem::create_directory(*configuration.output_path);
    }

    std::println("Linking.");

    const auto linking_successful = std::system(link_command.c_str()) == EXIT_SUCCESS;

    if (!linking_successful)
    {
        return "Linking failed.";
    }

    std::println("Finished linking. Executable located at '{}'.", actual_output_path);

    return std::nullopt;
}

auto create_executable(const std::string_view configuration_name,
                       const std::filesystem::path& path_to_root,
                       const std::vector<Configuration>& configurations) -> int
{
    const auto actual_configuration = get_actual_configuration(configuration_name, configurations);

    if (!actual_configuration.has_value())
    {
        std::println("{}", actual_configuration.error());

        return EXIT_FAILURE;
    }

    const auto source_files = get_source_files(*actual_configuration, path_to_root);
    const auto [files_to_delete, files_to_compile] =
        build_caching::handle_build_caching(*actual_configuration->name, path_to_root, source_files);

    // TODO: delete object files of files that don't exist anymore (`files_to_delete`).

    const auto compilation_error = create_object_files(*actual_configuration, path_to_root, files_to_compile);

    if (compilation_error.has_value())
    {
        std::println("{}", *compilation_error);

        return EXIT_FAILURE;
    }

    const auto linking_error = link_object_files(*actual_configuration, path_to_root);

    if (linking_error.has_value())
    {
        std::println("{}", *linking_error);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

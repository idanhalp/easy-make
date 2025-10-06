#include "source/executable_creation/executable_creation.hpp"

#include <cstdlib> // `std::system`
#include <format>
#include <iterator>
#include <print>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/build_caching/build_caching.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/utils.hpp"

auto check_names_validity(const std::vector<Configuration>& configurations) -> std::optional<std::string>
{
    std::unordered_map<std::string, std::ptrdiff_t> configuration_to_index;

    for (const auto [index, configuration] : std::views::enumerate(configurations) | std::views::as_const)
    {
        const auto actual_index = index + 1;
        if (!configuration.name.has_value())
        {
            return std::format("Error: The {}{} configuration does not have a name.", actual_index,
                               utils::get_ordinal_indicator(actual_index));
        }

        const auto& name = *configuration.name;

        if (configuration_to_index.contains(name))
        {
            return std::format("Error: Both the {}{} and {}{} configurations have '{}' as name.",
                               configuration_to_index.at(name),
                               utils::get_ordinal_indicator(configuration_to_index.at(name)), actual_index,
                               utils::get_ordinal_indicator(actual_index), name);
        }

        configuration_to_index[name] = index + 1;
    }

    return std::nullopt;
}

auto check_parents_validity(const std::unordered_map<std::string, Configuration>& name_to_configuration)
    -> std::optional<std::string>
{
    for (const auto& configuration : std::views::values(name_to_configuration))
    {
        if (configuration.parent == configuration.name)
        {
            return std::format("Error: Configuration '{}' has itself as a parent.", *configuration.name);
        }

        if (configuration.parent.has_value() && !name_to_configuration.contains(*configuration.parent))
        {
            return std::format("Error: Configuration '{}' has a non-existent configuration as its parent ('{}').",
                               *configuration.name, *configuration.parent);
        }
    }

    return std::nullopt;
}

static auto merge_configuration_with_parent(const Configuration& original, const Configuration& parent) -> Configuration
{
    auto result = original;

    if (!original.compiler.has_value())
    {
        result.compiler = parent.compiler;
    }

    if (!original.standard.has_value())
    {
        result.standard = parent.standard;
    }

    if (!original.warnings.has_value())
    {
        result.warnings = parent.warnings;
    }

    if (!original.optimization.has_value())
    {
        result.optimization = parent.optimization;
    }

    if (!original.defines.has_value())
    {
        result.defines = parent.defines;
    }

    if (!original.include_directories.has_value())
    {
        result.include_directories = parent.include_directories;
    }

    if (!original.source_files.has_value())
    {
        result.source_files = parent.source_files;
    }

    if (!original.source_directories.has_value())
    {
        result.source_directories = parent.source_directories;
    }

    if (!original.excluded_files.has_value())
    {
        result.excluded_files = parent.excluded_files;
    }

    if (!original.excluded_directories.has_value())
    {
        result.excluded_directories = parent.excluded_directories;
    }

    if (!original.output_name.has_value())
    {
        result.output_name = parent.output_name;
    }

    if (!original.output_path.has_value())
    {
        result.output_path = parent.output_path;
    }

    result.parent = parent.parent;

    return result;
}

auto get_actual_configuration(std::string configuration_name, const std::vector<Configuration>& configurations)
    -> std::expected<Configuration, std::string>
{
    const auto name_error        = check_names_validity(configurations);
    const auto name_error_exists = name_error.has_value();

    if (name_error_exists)
    {
        return std::unexpected(*name_error);
    }

    const auto get_name_configuration_pair = [](const auto& config) { return std::make_pair(*config.name, config); };
    const auto name_to_configuration =
        configurations | std::views::transform(get_name_configuration_pair) | std::ranges::to<std::unordered_map>();

    const auto parent_error        = check_parents_validity(name_to_configuration);
    const auto parent_error_exists = parent_error.has_value();

    if (parent_error_exists)
    {
        return std::unexpected(*parent_error);
    }

    // TODO: Make sure parents do not create a cycle.

    const auto configuration_exists = name_to_configuration.contains(configuration_name);

    if (!configuration_exists)
    {
        return std::unexpected(std::format("'{}' does not contain a configuration named '{}'.",
                                           params::CONFIGURATIONS_FILE_NAME.native(), configuration_name));
    }

    auto current_configuration = name_to_configuration.at(configuration_name);

    while (current_configuration.parent.has_value())
    {
        const auto& parent_configuration = name_to_configuration.at(*current_configuration.parent);
        current_configuration            = merge_configuration_with_parent(current_configuration, parent_configuration);
    }

    const auto error_with_merged_configuration = current_configuration.check_for_errors();

    if (error_with_merged_configuration.has_value())
    {
        return std::unexpected(*error_with_merged_configuration);
    }

    return current_configuration;
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
        std::format_to(std::back_inserter(result), "-std=c++{} ", *configuration.standard);
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
        if (*configuration.compiler == "cl") // MSVC.
        {
            std::format_to(std::back_inserter(result), "/O{} ", *configuration.optimization);
        }
        else
        {
            std::format_to(std::back_inserter(result), "-O{} ", *configuration.optimization);
        }
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
    const auto actual_configuration = get_actual_configuration(std::string(configuration_name), configurations);

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

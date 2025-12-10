#include "source/commands/build/build.hpp"

#include <cstdlib> // `std::system`
#include <format>
#include <iterator> // std::back_inserter
#include <print>
#include <ranges>
#include <string>
#include <system_error> // std::error_code
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/commands/build/build_caching/build_caching.hpp"
#include "source/commands/build/compilation/compilation.hpp"
#include "source/commands/build/linking.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/graph.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

/// @brief  Creates a new configuration by merging fields from a parent hierarchy.
///         Missing fields in `original` are populated from `parent`. If `parent`
///         also lacks a field, the lookup continues recursively through `parent`'s
///         parent chain until a value is found or the chain ends.
/// @param  original  The base configuration (highest priority).
/// @param  parent    The parent configuration used to fill missing fields.
/// @return A new configuration with all fields resolved from the inheritance hierarchy.
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

static auto check_for_missing_values(const Configuration& configuration) -> std::optional<std::string>
{
    const auto missing_compiler            = !configuration.compiler.has_value();
    const auto missing_output_name         = !configuration.output_name.has_value();
    const auto all_critical_fields_present = !missing_compiler && !missing_output_name;

    if (all_critical_fields_present)
    {
        return std::nullopt;
    }

    const auto missing_field = missing_compiler ? "compiler" : missing_output_name ? "output.name" : "";

    return std::format("Error: Could not resolve {} for configuration '{}'.", missing_field, *configuration.name);
}

auto get_actual_configuration(const std::string& target_configuration_name,
                              const std::vector<Configuration>& configurations)
    -> std::expected<Configuration, std::string>
{
    const auto get_name_configuration_pair = [](const auto& config) { return std::make_pair(*config.name, config); };
    const auto name_to_configuration =
        configurations | std::views::transform(get_name_configuration_pair) | std::ranges::to<std::unordered_map>();

    const auto configuration_exists = name_to_configuration.contains(target_configuration_name);

    if (!configuration_exists)
    {
        const auto configuration_names = std::views::keys(name_to_configuration) | std::ranges::to<std::vector>();
        auto error_message             = std::format("'{}' does not contain a configuration named '{}'.",
                                         params::CONFIGURATIONS_FILE_NAME.native(),
                                         target_configuration_name);

        if (const auto closest_name = utils::find_closest_word(target_configuration_name, configuration_names);
            closest_name.has_value())
        {
            std::format_to(std::back_inserter(error_message), " Did you mean '{}'?", *closest_name);
        }

        return std::unexpected(error_message);
    }

    auto current_configuration = name_to_configuration.at(target_configuration_name);

    while (current_configuration.parent.has_value())
    {
        const auto& parent_configuration = name_to_configuration.at(*current_configuration.parent);
        current_configuration            = merge_configuration_with_parent(current_configuration, parent_configuration);
    }

    if (const auto missing_values_error = check_for_missing_values(current_configuration);
        missing_values_error.has_value())
    {
        return std::unexpected(*missing_values_error);
    }

    return current_configuration;
}

auto get_code_files(const Configuration& configuration,
                    const std::filesystem::path& path_to_root) -> std::vector<std::filesystem::path>
{
    std::unordered_set<std::filesystem::path> code_files;

    if (configuration.source_files.has_value())
    {
        for (const auto& file : *configuration.source_files)
        {
            const auto full_path_to_file = path_to_root / file;

            if (std::filesystem::exists(full_path_to_file))
            {
                code_files.insert(file);
            }
        }
    }

    if (configuration.source_directories.has_value())
    {
        for (const auto& directory : *configuration.source_directories)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path_to_root / directory))
            {
                if (entry.is_regular_file() && utils::is_code_file(entry))
                {
                    auto relative_path = std::filesystem::relative(entry, path_to_root);
                    code_files.insert(std::move(relative_path));
                }
            }
        }
    }

    if (configuration.excluded_files.has_value())
    {
        for (const auto& file : *configuration.excluded_files)
        {
            code_files.erase(file);
        }
    }

    if (configuration.excluded_directories.has_value())
    {
        for (const auto& directory : *configuration.excluded_directories)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path_to_root / directory))
            {
                if (entry.is_regular_file() && utils::is_code_file(entry))
                {
                    auto relative_path = std::filesystem::relative(entry, path_to_root);
                    code_files.erase(std::move(relative_path));
                }
            }
        }
    }

    return code_files | std::ranges::to<std::vector>();
}

static auto remove_object_files_of_deleted_files(const std::string_view configuration_name,
                                                 const std::vector<std::filesystem::path>& deleted_files,
                                                 const std::filesystem::path& path_to_root) -> void
{
    const auto object_files_directory        = path_to_root / params::BUILD_DIRECTORY_NAME / configuration_name;
    const auto object_files_directory_exists = std::filesystem::is_directory(object_files_directory);

    if (!object_files_directory_exists)
    {
        return;
    }

    for (const auto& file_name : deleted_files)
    {
        const auto object_file_path = object_files_directory / utils::get_object_file_name(file_name);
        std::error_code error;
        std::filesystem::remove(object_file_path, error);

        if (error)
        {
            std::println("Error: Failed to remove old object file for '{}': {}", file_name.native(), error.message());
        }
    }
}

auto commands::build(const BuildCommandInfo& info,
                     const std::vector<Configuration>& configurations,
                     const std::filesystem::path& path_to_root) -> BuildCommandResult
{
    const auto actual_configuration = get_actual_configuration(info.configuration_name, configurations);

    if (!actual_configuration.has_value())
    {
        utils::print_error("{}", actual_configuration.error());

        return {
            .num_of_files_compiled       = 0,
            .num_of_compilation_failures = 0,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    const auto code_files = get_code_files(*actual_configuration, path_to_root);
    const auto build_info = build_caching::handle_build_caching(*actual_configuration, path_to_root, code_files);
    const auto error_exists_in_build = !build_info.has_value();

    if (error_exists_in_build)
    {
        utils::print_error("{}", build_info.error());

        return {
            .num_of_files_compiled       = 0,
            .num_of_compilation_failures = 0,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    // Delete object files for deleted source files to prevent the linker from using stale objects,
    // which can cause linker errors or violate the ODR.
    remove_object_files_of_deleted_files(info.configuration_name, build_info->files_to_delete, path_to_root);

    const auto num_of_compilation_failures = compile_files(*actual_configuration,
                                                           path_to_root,
                                                           build_info->files_to_compile,
                                                           info.is_quiet,
                                                           info.use_parallel_compilation);

    ASSERT(num_of_compilation_failures >= 0);
    const auto compilation_successful = (num_of_compilation_failures == 0);

    if (!compilation_successful)
    {
        return {
            .num_of_files_compiled       = static_cast<int>(build_info->files_to_compile.size()),
            .num_of_compilation_failures = num_of_compilation_failures,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    const auto linking_successful = link_object_files(
        *actual_configuration, path_to_root, actual_configuration->link_flags.value_or({}), info.is_quiet);

    if (!linking_successful)
    {
        return {
            .num_of_files_compiled       = static_cast<int>(build_info->files_to_compile.size()),
            .num_of_compilation_failures = 0,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    return {
        .num_of_files_compiled       = static_cast<int>(build_info->files_to_compile.size()),
        .num_of_compilation_failures = 0,
        .exit_status                 = EXIT_SUCCESS,
    };
}

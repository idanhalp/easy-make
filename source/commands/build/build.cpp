#include "source/commands/build/build.hpp"

#include <cstdlib> // `std::system`
#include <format>
#include <print>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/commands/build/build_caching/build_caching.hpp"
#include "source/commands/build/compilation.hpp"
#include "source/commands/build/linking.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/graph.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

/// @brief  Validates that all configurations have unique names.
/// @param  configurations  A collection of configuration objects to validate.
/// @return `std::nullopt` if all names are valid, or a `std::string` with the relevant error otherwise.
auto check_names_validity(const std::vector<Configuration>& configurations) -> std::optional<std::string>
{
    std::unordered_map<std::string, std::ptrdiff_t> configuration_to_index;

    for (const auto [index, configuration] : std::views::enumerate(configurations) | std::views::as_const)
    {
        const auto actual_index = index + 1;

        if (!configuration.name.has_value())
        {
            return std::format("Error: The {}{} configuration does not have a name.",
                               actual_index,
                               utils::get_ordinal_indicator(actual_index));
        }

        const auto& name = *configuration.name;

        if (configuration_to_index.contains(name))
        {
            return std::format("Error: Both the {}{} and {}{} configurations have '{}' as name.",
                               configuration_to_index.at(name),
                               utils::get_ordinal_indicator(configuration_to_index.at(name)),
                               actual_index,
                               utils::get_ordinal_indicator(actual_index),
                               name);
        }

        configuration_to_index[name] = actual_index;
    }

    return std::nullopt;
}

static auto check_for_parent_cycles(const std::unordered_map<std::string, Configuration>& name_to_configuration)
    -> std::optional<std::string>
{
    utils::DirectedGraph<std::string> parent_graph;

    for (const auto& [name, configuration] : name_to_configuration)
    {
        if (configuration.parent.has_value())
        {
            parent_graph.add_edge(name, *configuration.parent);
        }
    }

    return parent_graph.check_for_cycle();
}

auto check_parents_validity(const std::unordered_map<std::string, Configuration>& name_to_configuration)
    -> std::optional<std::string>
{
    for (const auto& configuration : std::views::values(name_to_configuration))
    {
        const auto configuration_has_parent = configuration.parent.has_value();

        if (!configuration_has_parent)
        {
            continue;
        }

        const auto configuration_is_its_own_parent = configuration.name == configuration.parent;

        if (configuration_is_its_own_parent)
        {
            return std::format("Error: Configuration '{}' has itself as a parent.", *configuration.name);
        }

        const auto parent_exists = name_to_configuration.contains(*configuration.parent);

        if (!parent_exists)
        {
            const auto configuration_names = std::views::keys(name_to_configuration) | std::ranges::to<std::vector>();
            const auto closest_parent      = utils::find_closest_word(*configuration.parent, configuration_names);

            return closest_parent.has_value()
                       ? std::format("Error: Configuration '{}' has a non-existent configuration as its parent ('{}'). "
                                     "Did you mean '{}'?",
                                     *configuration.name,
                                     *configuration.parent,
                                     *closest_parent)
                       : std::format("Error: Configuration '{}' has a non-existent configuration as its parent ('{}').",
                                     *configuration.name,
                                     *configuration.parent);
        }
    }

    const auto cycle_info   = check_for_parent_cycles(name_to_configuration);
    const auto cycle_exists = cycle_info.has_value();

    if (cycle_exists)
    {
        return std::format("Error: Circular parent dependency detected.\n\n"
                           "The following configurations form a cycle:\n"
                           "{}\n\n"
                           "Consider restructuring the code to break the circular dependency.",
                           *cycle_info);
    }
    else
    {
        return std::nullopt;
    }
}

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

auto get_actual_configuration(const std::string& configuration_name, const std::vector<Configuration>& configurations)
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

    const auto configuration_exists = name_to_configuration.contains(configuration_name);

    if (!configuration_exists)
    {
        const auto configuration_names = std::views::keys(name_to_configuration) | std::ranges::to<std::vector>();
        const auto closest_name        = utils::find_closest_word(configuration_name, configuration_names);
        const auto error_message =
            closest_name.has_value()
                ? std::format("'{}' does not contain a configuration named '{}'. Did you mean '{}'?",
                              params::CONFIGURATIONS_FILE_NAME.native(),
                              configuration_name,
                              *closest_name)
                : std::format("'{}' does not contain a configuration named '{}'.",
                              params::CONFIGURATIONS_FILE_NAME.native(),
                              configuration_name);

        return std::unexpected(error_message);
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

auto commands::build(const BuildCommandInfo& info,
                     const std::vector<Configuration>& configurations,
                     const std::filesystem::path& path_to_root) -> BuildCommandResult
{
    const auto actual_configuration = get_actual_configuration(info.configuration_name, configurations);

    if (!actual_configuration.has_value())
    {
        utils::print_error("{}", actual_configuration.error());

        return {
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
            .num_of_compilation_failures = 0,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    // TODO: delete object files of files that don't exist anymore (`build_info->files_to_delete`).

    const auto num_of_compilation_failures =
        compile_files(*actual_configuration, path_to_root, build_info->files_to_compile, info.is_quiet);

    ASSERT(num_of_compilation_failures >= 0);
    const auto compilation_successful = (num_of_compilation_failures == 0);

    if (!compilation_successful)
    {
        return {
            .num_of_compilation_failures = num_of_compilation_failures,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    const auto linking_successful = link_object_files(*actual_configuration, path_to_root, info.is_quiet);

    if (!linking_successful)
    {
        return {
            .num_of_compilation_failures = 0,
            .exit_status                 = EXIT_FAILURE,
        };
    }

    return {
        .num_of_compilation_failures = 0,
        .exit_status                 = EXIT_SUCCESS,
    };
}

#include "source/commands/build/configuration_resolution.hpp"

#include <algorithm>
#include <format>
#include <ranges>
#include <unordered_map>

#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"

static auto map_names_to_configurations(const std::vector<Configuration>& configurations)
    -> std::unordered_map<std::string, Configuration>
{
    std::unordered_map<std::string, Configuration> name_to_configuration;

    for (const auto& configuration : configurations)
    {
        name_to_configuration[*configuration.name] = configuration;
    }

    return name_to_configuration;
}

static auto inherit_missing_fields_from_parent(const Configuration& original,
                                               const Configuration& parent) -> Configuration
{
    auto result = original;

    result.parent.reset();

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

    if (!original.compilation_flags.has_value())
    {
        result.compilation_flags = parent.compilation_flags;
    }

    if (!original.link_flags.has_value())
    {
        result.link_flags = parent.link_flags;
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

    return result;
}

static auto resolve_configuration(const Configuration& configuration_to_resolve,
                                  const std::unordered_map<std::string, Configuration>& name_to_configuration,
                                  std::unordered_map<std::string, Configuration>& resolved_configurations_map) -> void
{
    const auto& name                          = *configuration_to_resolve.name;
    const auto configuration_already_resolved = resolved_configurations_map.contains(name);

    if (configuration_already_resolved)
    {
        return;
    }
    else if (!configuration_to_resolve.parent.has_value())
    {
        resolved_configurations_map[name] = configuration_to_resolve;
    }
    else
    {
        const auto& original_parent_configuration = name_to_configuration.at(*configuration_to_resolve.parent);
        resolve_configuration(original_parent_configuration, name_to_configuration, resolved_configurations_map);
        const auto& resolved_parent_configuration = resolved_configurations_map.at(*configuration_to_resolve.parent);
        resolved_configurations_map[name] =
            inherit_missing_fields_from_parent(configuration_to_resolve, resolved_parent_configuration);
    }
}

static auto create_missing_configuration_error(const std::string_view target_name,
                                               const std::vector<Configuration>& configurations) -> std::string
{
    const auto get_name = [](const auto& config) { return *config.name; };
    const auto names    = configurations | std::views::transform(get_name) | std::ranges::to<std::vector>();
    auto error_message  = std::format("Error: '{}' does not contain a configuration named '{}'.",
                                     params::CONFIGURATIONS_FILE_NAME.native(),
                                     target_name);

    if (const auto closest_name = utils::find_closest_word(target_name, names); closest_name.has_value())
    {
        std::format_to(std::back_inserter(error_message), " Did you mean '{}'?", *closest_name);
    }

    return error_message;
}

static auto check_for_missing_fields(const Configuration& configuration) -> std::optional<std::string>
{
    const auto missing_compiler            = !configuration.compiler.has_value();
    const auto missing_output_name         = !configuration.output_name.has_value();
    const auto all_critical_fields_present = !missing_compiler && !missing_output_name;

    if (all_critical_fields_present)
    {
        return std::nullopt;
    }

    const auto missing_field = missing_compiler ? "compiler" : missing_output_name ? "output.name" : "";

    return std::format(
        "Error: configuration '{}' is incomplete - could not resolve '{}'.", *configuration.name, missing_field);
}

auto get_resolved_configuration(const std::vector<Configuration>& configurations,
                                const std::string_view target_name) -> std::expected<Configuration, std::string>
{
    const auto resolved_configurations = get_resolved_configurations(configurations, ConfigurationType::ALL);

    const auto is_name_correct      = [&](const auto& config) { return config.name == target_name; };
    const auto configuration        = std::ranges::find_if(resolved_configurations, is_name_correct);
    const auto configuration_exists = configuration != resolved_configurations.end();

    if (!configuration_exists)
    {
        return std::unexpected(create_missing_configuration_error(target_name, configurations));
    }

    const auto missing_field_error       = check_for_missing_fields(*configuration);
    const auto configuration_is_complete = !missing_field_error.has_value();

    if (!configuration_is_complete)
    {
        return std::unexpected(*missing_field_error);
    }

    return *configuration;
}

auto get_resolved_configurations(const std::vector<Configuration>& configurations,
                                 const ConfigurationType configuration_type) -> std::vector<Configuration>
{
    const auto name_to_configuration = map_names_to_configurations(configurations);
    std::unordered_map<std::string, Configuration> resolved_configurations_map;

    for (const auto& configuration : configurations)
    {
        resolve_configuration(
            configuration,
            name_to_configuration,
            resolved_configurations_map); // Adds the resolved configuration to `resolved_configurations_map`.
    }

    // Extract the configurations (values) from the map.
    // It is important to maintain the same order of `configurations`.
    std::vector<Configuration> resolved_configurations;
    resolved_configurations.reserve(configurations.size());

    for (const auto& configuration : configurations)
    {
        auto resolved_configuration = resolved_configurations_map.at(*configuration.name);
        const auto is_complete      = !check_for_missing_fields(resolved_configuration).has_value();
        const auto is_relevant      = configuration_type == ConfigurationType::COMPLETE     ? is_complete
                                      : configuration_type == ConfigurationType::INCOMPLETE ? !is_complete
                                                                                            : true;

        if (is_relevant)
        {
            resolved_configurations.push_back(std::move(resolved_configuration));
        }
    }

    return resolved_configurations;
}

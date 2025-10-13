#include "source/configuration_parsing/configuration_parsing.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <ranges>

#include "third_party/nlohmann/json.hpp"

#include "source/configuration_parsing/json_keys.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"

static auto
check_for_errors_in_configuration_json(const nlohmann::json& configuration_json) -> std::optional<std::string>
{
    using namespace json_keys;

    for (const auto& [key, _] : configuration_json.items())
    {
        if (is_valid_outer_key(key))
        {
            continue;
        }

        const auto closest_valid_key = utils::find_closest_word(key, get_valid_outer_keys());

        return closest_valid_key.has_value()
                   ? std::format("Error: Invalid JSON in '{}' - '{}' is not a valid outer key. Did you mean '{}'?",
                                 params::CONFIGURATIONS_FILE_NAME.native(), key, *closest_valid_key)
                   : std::format("Error: Invalid JSON in '{}' - '{}' is not a valid outer key.",
                                 params::CONFIGURATIONS_FILE_NAME.native(), key);
    }

    if (configuration_json.contains(key_to_string(JsonKey::Source)))
    {
        const auto sources = configuration_json[key_to_string(JsonKey::Source)];

        if (!sources.is_object())
        {
            return std::format("Error: Invalid JSON in '{}' - the value of '{}' must be an object.",
                               params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Source));
        }

        for (const auto& [key, _] : sources.items())
        {
            if (is_valid_source_key(key))
            {
                continue;
            }

            const auto closest_valid_key = utils::find_closest_word(key, get_valid_source_keys());

            return closest_valid_key.has_value()
                       ? std::format("Error: Invalid JSON in '{}' - '{}.{}' is not a valid key. Did you mean '{}.{}'?",
                                     params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Source), key,
                                     key_to_string(JsonKey::Source), *closest_valid_key)
                       : std::format("Error: Invalid JSON in '{}' - '{}.{}' is not a valid key.",
                                     params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Source), key);
        }
    }

    if (configuration_json.contains(key_to_string(JsonKey::Excludes)))
    {
        const auto excludes = configuration_json[key_to_string(JsonKey::Excludes)];

        if (!excludes.is_object())
        {
            return std::format("Error: Invalid JSON in '{}' - the value of '{}' must be an object.",
                               params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Excludes));
        }

        for (const auto& [key, _] : excludes.items())
        {
            if (is_valid_excludes_key(key))
            {
                continue;
            }

            const auto closest_valid_key = utils::find_closest_word(key, get_valid_excludes_keys());

            return closest_valid_key.has_value()
                       ? std::format("Error: Invalid JSON in '{}' - '{}.{}' is not a valid key. Did you mean '{}.{}'?",
                                     params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Excludes), key,
                                     key_to_string(JsonKey::Excludes), *closest_valid_key)
                       : std::format("Error: Invalid JSON in '{}' - '{}.{}' is not a valid key.",
                                     params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Excludes), key);
        }
    }

    if (configuration_json.contains(key_to_string(JsonKey::Output)))
    {
        const auto output = configuration_json[key_to_string(JsonKey::Output)];

        if (!output.is_object())
        {
            return std::format("Error: Invalid JSON in '{}' - the value of '{}' must be an object.",
                               params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Output));
        }

        for (const auto& [key, _] : output.items())
        {
            if (is_valid_output_key(key))
            {
                continue;
            }

            const auto closest_valid_key = utils::find_closest_word(key, get_valid_output_keys());

            return closest_valid_key.has_value()
                       ? std::format("Error: Invalid JSON in '{}' - '{}.{}' is not a valid key. Did you mean '{}.{}'?",
                                     params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Output), key,
                                     key_to_string(JsonKey::Output), *closest_valid_key)
                       : std::format("Error: Invalid JSON in '{}' - '{}.{}' is not a valid key.",
                                     params::CONFIGURATIONS_FILE_NAME.native(), key_to_string(JsonKey::Output), key);
        }
    }

    return std::nullopt;
}

static auto check_for_errors_in_json(const nlohmann::json& json) -> std::optional<std::string>
{
    if (!json.is_array())
    {
        return std::format("Error: Invalid JSON in '{}' - Must be an array.",
                           params::CONFIGURATIONS_FILE_NAME.native());
    }

    const auto is_array_of_objects = std::ranges::all_of(json, &nlohmann::json::is_object);

    if (!is_array_of_objects)
    {
        return std::format("Error: Invalid JSON in '{}' - All elements of the array must be objects.",
                           params::CONFIGURATIONS_FILE_NAME.native());
    }

    for (const auto& configuration_object : json)
    {
        if (const auto error = check_for_errors_in_configuration_json(configuration_object); error.has_value())
        {
            return *error;
        }
    }

    return std::nullopt;
}

static auto parse_configuration(const nlohmann::json& json) -> Configuration
{
    using namespace json_keys;

    Configuration configuration;

    if (json.contains(key_to_string(JsonKey::Name)))
    {
        configuration.name = json[key_to_string(JsonKey::Name)];
    }

    if (json.contains(key_to_string(JsonKey::Parent)))
    {
        configuration.parent = json[key_to_string(JsonKey::Parent)];
    }

    if (json.contains(key_to_string(JsonKey::Compiler)))
    {
        configuration.compiler = json[key_to_string(JsonKey::Compiler)];
    }

    if (json.contains(key_to_string(JsonKey::Standard)))
    {
        configuration.standard = json[key_to_string(JsonKey::Standard)];
    }

    if (json.contains(key_to_string(JsonKey::Optimization)))
    {
        configuration.optimization = json[key_to_string(JsonKey::Optimization)];
    }

    if (json.contains(key_to_string(JsonKey::Warnings)))
    {
        configuration.warnings = json[key_to_string(JsonKey::Warnings)];
    }

    if (json.contains(key_to_string(JsonKey::Defines)))
    {
        configuration.defines = json[key_to_string(JsonKey::Defines)];
    }

    if (json.contains(key_to_string(JsonKey::IncludeDirectories)))
    {
        configuration.include_directories = json[key_to_string(JsonKey::IncludeDirectories)];
    }

    if (json.contains(key_to_string(JsonKey::Source)))
    {
        const auto& source = json[key_to_string(JsonKey::Source)];

        if (source.contains(key_to_string(JsonKey::SourceFiles)))
        {
            configuration.source_files = source[key_to_string(JsonKey::SourceFiles)];
        }

        if (source.contains(key_to_string(JsonKey::SourceDirectories)))
        {
            configuration.source_directories = source[key_to_string(JsonKey::SourceDirectories)];
        }
    }

    if (json.contains(key_to_string(JsonKey::Excludes)))
    {
        const auto& excludes = json[key_to_string(JsonKey::Excludes)];

        if (excludes.contains(key_to_string(JsonKey::ExcludedFiles)))
        {
            configuration.excluded_files = excludes[key_to_string(JsonKey::ExcludedFiles)];
        }

        if (excludes.contains(key_to_string(JsonKey::ExcludedDirectories)))
        {
            configuration.excluded_directories = excludes[key_to_string(JsonKey::ExcludedDirectories)];
        }
    }

    if (json.contains(key_to_string(JsonKey::Output)))
    {
        const auto& output = json[key_to_string(JsonKey::Output)];

        if (output.contains(key_to_string(JsonKey::OutputName)))
        {
            configuration.output_name = output[key_to_string(JsonKey::OutputName)];
        }

        if (output.contains(key_to_string(JsonKey::OutputPath)))
        {
            configuration.output_path = output[key_to_string(JsonKey::OutputPath)];
        }
    }

    return configuration;
}

auto parse_configurations(const std::filesystem::path& path_to_configurations_file)
    -> std::expected<std::vector<Configuration>, std::string>
{
    std::ifstream configurations_file(path_to_configurations_file / params::CONFIGURATIONS_FILE_NAME);
    nlohmann::json json;

    try
    {
        configurations_file >> json;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        return std::unexpected(
            std::format("Error: Invalid JSON in '{}' - {}", params::CONFIGURATIONS_FILE_NAME.native(), e.what()));
    }

    const auto error_in_json       = check_for_errors_in_json(json);
    const auto found_error_in_json = error_in_json.has_value();

    if (found_error_in_json)
    {
        return std::unexpected(*error_in_json);
    }

    return json | std::views::transform(parse_configuration) | std::ranges::to<std::vector>();
}

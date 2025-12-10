#include "source/configuration_parsing/configuration_parsing.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <ranges>
#include <string>

#include "third_party/nlohmann/json.hpp"

#include "source/configuration_parsing/json_keys.hpp"
#include "source/configuration_parsing/structure_validation.hpp"
#include "source/configuration_parsing/value_validation.hpp"
#include "source/parameters/parameters.hpp"

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

    if (json.contains(key_to_string(JsonKey::CompilationFlags)))
    {
        configuration.compilation_flags = json[key_to_string(JsonKey::CompilationFlags)];
    }

    if (json.contains(key_to_string(JsonKey::LinkFlags)))
    {
        configuration.link_flags = json[key_to_string(JsonKey::LinkFlags)];
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

static auto read_json(const std::filesystem::path& path_to_root) -> std::expected<nlohmann::json, std::string>
{
    auto configurations_file = std::ifstream(path_to_root / params::CONFIGURATIONS_FILE_NAME);
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

    return json;
}

auto parse_configurations(const std::filesystem::path& path_to_root)
    -> std::expected<std::vector<Configuration>, std::string>
{
    const auto json                           = read_json(path_to_root);
    const auto encountered_error_reading_json = !json.has_value();

    if (encountered_error_reading_json)
    {
        return std::unexpected(json.error());
    }

    const auto json_structure_error = validate_json_structure(*json);
    const auto structure_is_valid   = !json_structure_error.has_value();

    if (!structure_is_valid)
    {
        return std::unexpected(*json_structure_error);
    }

    const auto configurations = *json | std::views::transform(parse_configuration) | std::ranges::to<std::vector>();
    const auto configuration_value_error              = validate_configuration_values(configurations, path_to_root);
    const auto found_configuration_with_invalid_value = configuration_value_error.has_value();

    if (found_configuration_with_invalid_value)
    {
        return std::unexpected(*configuration_value_error);
    }

    return configurations;
}

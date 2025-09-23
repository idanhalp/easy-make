#include "source/configuration_parsing/configuration_parsing.hpp"

#include <fstream>
#include <ranges>

#include "source/utils/utils.hpp"
#include "third_party/json.hpp"

static auto parse_configuration(const nlohmann::json& json) -> Configuration
{
    Configuration configuration;

    if (json.contains("name"))
    {
        configuration.name = json["name"];
    }

    if (json.contains("compiler"))
    {
        configuration.compiler = json["compiler"];
    }

    if (json.contains("standard"))
    {
        configuration.standard = json["standard"];
    }

    if (json.contains("optimization"))
    {
        configuration.optimization = json["optimization"];
    }

    if (json.contains("warnings"))
    {
        configuration.warnings = json["warnings"];
    }

    if (json.contains("defines"))
    {
        configuration.defines = json["defines"];
    }

    if (json.contains("sources"))
    {
        const auto& sources = json["sources"];

        if (sources.contains("files"))
        {
            configuration.source_files = sources["files"];
        }

        if (sources.contains("directories"))
        {
            configuration.source_directories = sources["directories"];
        }
    }

    if (json.contains("exclude"))
    {
        const auto& exclude = json["exclude"];

        if (exclude.contains("files"))
        {
            configuration.excluded_files = exclude["files"];
        }

        if (exclude.contains("directories"))
        {
            configuration.excluded_directories = exclude["directories"];
        }
    }

    if (json.contains("output"))
    {
        const auto& output = json["output"];

        if (output.contains("name"))
        {
            configuration.output_name = output["name"];
        }
        if (output.contains("path"))
        {
            configuration.output_path = output["path"];
        }
    }

    return configuration;
}

auto parse_configurations(const std::filesystem::path& path_to_configurations_file) -> std::vector<Configuration>
{
    std::ifstream configurations_file(path_to_configurations_file / utils::CONFIGURATIONS_FILE_NAME);
    nlohmann::json json;

    configurations_file >> json;

    return json | std::views::transform(parse_configuration) | std::ranges::to<std::vector>();
}

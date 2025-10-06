#include "source/configuration_parsing/configuration_parsing.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <ranges>

#include "third_party/json.hpp"

#include "source/parameters/parameters.hpp"

auto Configuration::check_for_errors() const -> std::optional<std::string>
{
    // Check for obligatory fields.

    if (!compiler.has_value())
    {
        return std::format("Error: Could not resolve 'compiler' for '{}'.", *name);
    }

    if (!output_name.has_value())
    {
        return std::format("Error: Could not resolve 'output.name' for '{}'.", *name);
    }

    // Check for invalid values.

    // --- Name ---
    const auto name_is_valid = !name->empty();

    if (!name_is_valid)
    {
        return std::format("Error: empty configuration name.");
    }

    // --- Compiler ---
    static const auto valid_compilers = {"g++", "clang++", "cl"};
    const auto compiler_is_valid      = std::ranges::contains(valid_compilers, *compiler);

    if (!compiler_is_valid)
    {
        return std::format("Error: Configuration '{}' - unknown compiler '{}'.", *name, *compiler);
    }

    // --- Standard ---
    static const auto valid_standards = {"98", "03", "11", "14", "17", "20", "23", "26"};
    const auto standard_is_valid      = !standard.has_value() || std::ranges::contains(valid_standards, *standard);

    if (!standard_is_valid)
    {
        return std::format("Error: Configuration '{}' - unknown standard '{}'.", *name, *standard);
    }

    // --- Warnings ---
    if (warnings.has_value())
    {
        static const auto is_valid_warning = [&](const std::string_view warning)
        {
            return *compiler == "cl"
                       ? (warning.starts_with("/W") || warning.starts_with("/w"))
                       : (warning.starts_with("-W") || warning == "-pedantic" || warning == "-pedantic-errors");
        };

        const auto invalid_warning        = std::ranges::find_if_not(*warnings, is_valid_warning);
        const auto invalid_warning_exists = invalid_warning != warnings->end();

        if (invalid_warning_exists)
        {
            return std::format("Error: Configuration '{}' - warning '{}' is invalid when compiling with '{}'.", *name,
                               *invalid_warning, *compiler);
        }
    }

    // --- Optimization ---
    static const auto gcc_and_clang_optimizations = {"0", "1", "2", "3", "s", "fast"};
    static const auto msvc_optimizations          = {"d", "1", "2", "3", "s", "x"};

    const auto optimization_is_valid =
        !optimization.has_value() ||
        (*compiler != "cl" && std::ranges::contains(gcc_and_clang_optimizations, *optimization)) ||
        (*compiler == "cl" && std::ranges::contains(msvc_optimizations, *optimization));

    if (!optimization_is_valid)
    {
        const auto optimization_mismatched_with_compiler =
            (*compiler == "cl" && std::ranges::contains(gcc_and_clang_optimizations, *optimization)) ||
            (*compiler != "cl" && std::ranges::contains(msvc_optimizations, *optimization));

        if (optimization_mismatched_with_compiler)
        {
            const auto correct_compiler = *compiler == "cl" ? "'g++' or 'clang++'" : "'cl'";

            return std::format("Error: Configuration '{}' - optimization '{}' does not match specified compiler '{}'. "
                               "Did you mean to compile with {} instead?",
                               *name, *optimization, *compiler, correct_compiler);
        }
        else
        {
            return std::format("Error: Configuration '{}' - unknown optimization '{}'.", *name, *optimization);
        }
    }

    return std::nullopt;
}

static auto parse_configuration(const nlohmann::json& json) -> Configuration
{
    Configuration configuration;

    if (json.contains("name"))
    {
        configuration.name = json["name"];
    }

    if (json.contains("parent"))
    {
        configuration.parent = json["parent"];
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

    if (json.contains("includeDirectories"))
    {
        configuration.include_directories = json["includeDirectories"];
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
    std::ifstream configurations_file(path_to_configurations_file / params::CONFIGURATIONS_FILE_NAME);
    nlohmann::json json;

    configurations_file >> json;

    return json | std::views::transform(parse_configuration) | std::ranges::to<std::vector>();
}

#include "source/executable_creation/executable_creation.hpp"

#include <algorithm>
#include <format>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

#include "source/utils/utils.hpp"

auto get_actual_configuration(std::string_view configuration_name, const std::vector<Configuration>& configurations)
    -> std::expected<Configuration, std::string>
{
    const auto original_configuration = std::ranges::find_if(
        configurations, [&](const Configuration& configuration) { return configuration.name == configuration_name; });
    const auto configuration_exists = original_configuration != configurations.end();

    if (!configuration_exists)
    {
        return std::unexpected(std::format("'{}' does not contain a configuration named {}.",
                                           utils::CONFIGURATIONS_FILE_NAME, configuration_name));
    }

    if (original_configuration->name == "default")
    {
        return *original_configuration;
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
        else
        {
            return std::unexpected(std::format("Could not resolve 'output.path' for '{}'.", configuration_name));
        }
    }

    return actual_configuration;
}

auto get_files_to_compile(const Configuration& configuration,
                          const std::filesystem::path& path_to_root) -> std::vector<std::string>
{
    std::unordered_set<std::string> files_to_compile;

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

auto create_executable(const std::string_view configuration_name,
                       const std::filesystem::path& path_to_root,
                       const std::vector<Configuration>& configurations) -> std::optional<std::string>
{
    const auto actual_configuration = get_actual_configuration(configuration_name, configurations);

    if (!actual_configuration.has_value())
    {
        return actual_configuration.error();
    }

    const auto files_to_compile = get_files_to_compile(*actual_configuration, path_to_root);

    return std::nullopt;
}

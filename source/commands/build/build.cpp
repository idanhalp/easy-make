#include "source/commands/build/build.hpp"

#include <print>
#include <ranges>
#include <string>
#include <system_error> // std::error_code
#include <unordered_set>
#include <vector>

#include "source/commands/build/build_caching/build_caching.hpp"
#include "source/commands/build/compilation/compilation.hpp"
#include "source/commands/build/configuration_resolution.hpp"
#include "source/commands/build/linking.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

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

static auto build_configuration(const BuildCommandInfo& info,
                                const Configuration& configuration,
                                const std::filesystem::path& path_to_root) -> BuildCommandResult
{
    const auto code_files            = get_code_files(configuration, path_to_root);
    const auto build_info            = build_caching::handle_build_caching(configuration, path_to_root, code_files);
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
    remove_object_files_of_deleted_files(*configuration.name, build_info->files_to_delete, path_to_root);

    const auto num_of_compilation_failures = compile_files(
        configuration, path_to_root, build_info->files_to_compile, info.is_quiet, info.use_parallel_compilation);

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

    const auto linking_successful =
        link_object_files(configuration, path_to_root, configuration.link_flags.value_or({}), info.is_quiet);

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

auto commands::build(const BuildCommandInfo& info,
                     const std::vector<Configuration>& configurations,
                     const std::filesystem::path& path_to_root) -> BuildCommandResult
{
    if (info.build_all_configurations)
    {
        BuildCommandResult total_result{};

        for (const auto& configuration : get_resolved_configurations(configurations, ConfigurationType::COMPLETE))
        {
            const auto build_result = build_configuration(info, configuration, path_to_root);
            total_result.num_of_files_compiled += build_result.num_of_files_compiled;
            total_result.num_of_compilation_failures += build_result.num_of_compilation_failures;
            total_result.exit_status |= build_result.exit_status;
        }

        return total_result;
    }

    const auto configuration                  = get_resolved_configuration(configurations, *info.configuration_name);
    const auto found_error_with_configuration = !configuration.has_value();

    if (found_error_with_configuration)
    {
        utils::print_error("{}", configuration.error());

        return {
            .num_of_files_compiled       = 0,
            .num_of_compilation_failures = 0,
            .exit_status                 = EXIT_FAILURE,
        };
    }
    else
    {
        return build_configuration(info, *configuration, path_to_root);
    }
}

#include "source/executable_creation/compilation.hpp"

#include <algorithm>
#include <cstdlib>
#include <format>
#include <iterator>
#include <print>
#include <ranges>
#include <string_view>

#include "source/parameters/parameters.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

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

    if (result.ends_with(" "))
    {
        result.pop_back();
    }

    return result;
}

static auto compile(const std::filesystem::path& file_name,
                    const std::filesystem::path& object_files_path,
                    const std::string_view compilation_flags,
                    const Configuration& configuration) -> bool
{
    ASSERT(configuration.compiler.has_value());

    const auto object_file_path    = object_files_path / utils::get_object_file_name(file_name);
    const auto compilation_command = std::format("{} {} -c {} -o {}", *configuration.compiler, compilation_flags,
                                                 file_name.native(), object_file_path.native());

    const auto file_compiled_successfully = std::system(compilation_command.c_str()) == EXIT_SUCCESS;

    return file_compiled_successfully;
}

static auto print_compilation_result(const std::vector<std::filesystem::path>& files_failed_to_compile) -> void
{
    ASSERT(std::ranges::is_sorted(files_failed_to_compile));

    const auto all_files_compiled_successfully = files_failed_to_compile.empty();

    if (all_files_compiled_successfully)
    {
        utils::print_success("Compilation complete.");
        return;
    }

    const auto max_index_width = utils::count_digits(files_failed_to_compile.size()); // For formatting.

    std::println();
    std::println("The following files failed to compile:");

    for (const auto [index, file] : std::views::enumerate(files_failed_to_compile) | std::views::as_const)
    {
        std::println("{0:>{2}}. {1}", index + 1, file.native(), max_index_width);
    }

    utils::print_error("Compilation failed.");
}

auto compile_files(const Configuration& configuration,
                   const std::filesystem::path& path_to_root,
                   const std::vector<std::filesystem::path>& files_to_compile) -> bool
{
    ASSERT(configuration.name.has_value());

    const auto compilation_flags = create_compilation_flags_string(configuration);
    const auto object_files_path = path_to_root / params::BUILD_DIRECTORY_NAME / *configuration.name;

    switch (files_to_compile.size())
    {
    case 0:
        std::println("No files to compile.");
        break;

    case 1:
        std::println("Compiling one file...");
        break;

    default:
        std::println("Compiling {} files...", files_to_compile.size());
        break;
    }

    const auto max_index_width = utils::count_digits(files_to_compile.size()); // For formatting.
    std::vector<std::filesystem::path> files_failed_to_compile;

    for (const auto& [index, file_name] : std::views::enumerate(files_to_compile) | std::views::as_const)
    {
        // Print current info, for example:
        // 12/20 [ 60%] path/to/file.cpp
        const auto completion_percentage = 100 * (index + 1) / files_to_compile.size();
        std::println("{0:>{2}}/{1} [{3:>3}%] {4}", index + 1, files_to_compile.size(), max_index_width,
                     completion_percentage, file_name.native());

        const auto file_compiled_successfully = compile(file_name, object_files_path, compilation_flags, configuration);

        if (!file_compiled_successfully)
        {
            files_failed_to_compile.push_back(file_name);
        }
    }

    print_compilation_result(files_failed_to_compile);

    return files_failed_to_compile.empty(); // All files compiled successfully.
}

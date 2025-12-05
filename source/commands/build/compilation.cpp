#include "source/commands/build/compilation.hpp"

#include <algorithm>
#include <cstdlib>
#include <format>
#include <future>
#include <iterator> // std::istreambuf_iterator
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error> // std::error_code
#include <thread>       // std::thread::hardware_concurrency

#include "source/parameters/parameters.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

namespace
{
    struct CompilationInfo
    {
        bool is_successful;
        std::string compiler_output;
    };
}

static auto print_number_of_files_to_compile(const int number_of_files,
                                             const std::string_view configuration_name) -> void
{
    ASSERT(number_of_files >= 0);

    switch (number_of_files)
    {
    case 0:
        std::println("No files to compile for configuration '{}'.", configuration_name);
        break;

    case 1:
        std::println("Compiling 1 file for configuration '{}'...", configuration_name);
        break;

    default:
        std::println("Compiling {} files for configuration '{}'...", number_of_files, configuration_name);
        break;
    }
}

static auto remove_outdated_object_files(const std::filesystem::path& object_files_directory,
                                         const std::vector<std::filesystem::path>& files_to_compile) -> void
{
    if (!std::filesystem::is_directory(object_files_directory))
    {
        return;
    }

    for (const auto& file_name : files_to_compile)
    {
        const auto object_file_path = object_files_directory / utils::get_object_file_name(file_name);
        std::error_code error;
        std::filesystem::remove(object_file_path, error);

        if (error)
        {
            std::println("Error: Failed to remove stale object file for '{}': {}", file_name.native(), error.message());
        }
    }
}

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

static auto compile_file(const std::filesystem::path& file_name,
                         const std::filesystem::path& object_files_directory,
                         const std::string_view compilation_flags,
                         const Configuration& configuration) -> CompilationInfo
{
    ASSERT(utils::is_source_file(file_name));
    ASSERT(configuration.compiler.has_value());

    const auto object_file_path    = object_files_directory / utils::get_object_file_name(file_name);
    const auto temporary_file_path = std::filesystem::temp_directory_path() / utils::get_object_file_name(file_name);

    // Compile the file with the given flag
    // and redirect stdout and stderr to the temporary file.
    const auto compilation_command = std::format("{} {} -fdiagnostics-color=always -c {} -o {} > {} 2>&1",
                                                 *configuration.compiler,
                                                 compilation_flags,
                                                 file_name.native(),
                                                 object_file_path.native(),
                                                 temporary_file_path.native());

    const auto file_compiled_successfully = std::system(compilation_command.c_str()) == EXIT_SUCCESS;
    const auto compiler_output            = [&]
    {
        auto temporary_file = std::ifstream(temporary_file_path);

        return std::string(std::istreambuf_iterator<char>(temporary_file), std::istreambuf_iterator<char>());
    }();

    std::filesystem::remove(temporary_file_path);

    return {
        .is_successful   = file_compiled_successfully,
        .compiler_output = compiler_output,
    };
}

static auto compile_batch(const std::span<const std::filesystem::path> batch,
                          const std::filesystem::path& object_files_directory,
                          const std::string_view compilation_flags,
                          const Configuration& configuration) -> std::vector<std::future<CompilationInfo>>
{
    std::vector<std::future<CompilationInfo>> futures;
    futures.reserve(batch.size());

    for (const auto& file : batch)
    {
        futures.push_back(std::async(
            std::launch::async, compile_file, file, object_files_directory, compilation_flags, configuration));
    }

    return futures;
}

static auto print_compilation_result(const std::vector<std::filesystem::path>& failed_compilation) -> void
{
    ASSERT(std::ranges::is_sorted(failed_compilation));
    ASSERT(std::ranges::adjacent_find(failed_compilation) == failed_compilation.end()) // Files are unique.

    const auto all_files_compiled_successfully = failed_compilation.empty();

    if (all_files_compiled_successfully)
    {
        utils::print_success("Compilation complete.");
        return;
    }

    const auto max_index_width = utils::count_digits(failed_compilation.size()); // For formatting.

    std::println();
    std::println("The following files failed to compile:");

    for (const auto [index, file] : std::views::enumerate(failed_compilation) | std::views::as_const)
    {
        std::println("{0:>{2}}. {1}", index + 1, file.native(), max_index_width);
    }

    utils::print_error("Compilation failed.");
}

auto compile_files(const Configuration& configuration,
                   const std::filesystem::path& path_to_root,
                   const std::vector<std::filesystem::path>& files_to_compile,
                   const bool is_quiet,
                   const bool use_parallel_compilation) -> int
{
    ASSERT(configuration.name.has_value());
    ASSERT(std::ranges::is_sorted(files_to_compile));

    if (!is_quiet)
    {
        print_number_of_files_to_compile(files_to_compile.size(), *configuration.name);
    }

    // If a previous build was interrupted or a file failed to compile, some object files
    // may be left in an inconsistent state. Before starting a new build, we remove all
    // object files for this configuration to ensure we don't incorrectly treat any of them
    // as up-to-date. This guarantees that the next build recompiles everything cleanly.
    const auto object_files_directory = path_to_root / params::BUILD_DIRECTORY_NAME / *configuration.name;
    remove_outdated_object_files(object_files_directory, files_to_compile);

    const auto batch_size        = use_parallel_compilation ? std::max(1U, std::thread::hardware_concurrency()) : 1;
    const auto compilation_flags = create_compilation_flags_string(configuration);
    const auto max_index_width   = utils::count_digits(files_to_compile.size()); // For formatting.
    std::vector<std::filesystem::path> failed_compilation;

    for (auto batch_start = 0UZ; batch_start < files_to_compile.size(); batch_start += batch_size)
    {
        const auto batch_end = std::min(batch_start + batch_size, files_to_compile.size());
        const auto batch     = std::span(files_to_compile.begin() + batch_start, files_to_compile.begin() + batch_end);
        auto futures         = compile_batch(batch, object_files_directory, compilation_flags, configuration);

        for (auto index = 0UZ; index < batch.size(); ++index)
        {
            if (!is_quiet)
            {
                // Print current info, for example:
                // 12/20 [ 60%] path/to/file.cpp
                const auto actual_index          = batch_start + index + 1;
                const auto completion_percentage = 100 * actual_index / files_to_compile.size();
                std::println("{0:>{2}}/{1} [{3:>3}%] {4}",
                             actual_index,
                             files_to_compile.size(),
                             max_index_width,
                             completion_percentage,
                             batch[index].native());
            }

            const auto result = futures[index].get();

            if (!result.is_successful)
            {
                failed_compilation.push_back(batch[index]);
            }

            std::print("{}", result.compiler_output);
        }
    }

    if (!is_quiet)
    {
        print_compilation_result(failed_compilation);
    }

    return failed_compilation.size();
}

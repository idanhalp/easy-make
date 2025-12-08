#include "source/commands/build/compilation/compilation.hpp"

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

#include "source/commands/build/compilation/thread_pool.hpp"
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

    if (configuration.compilation_flags.has_value())
    {
        for (const auto& compilation_flag : *configuration.compilation_flags)
        {
            std::format_to(std::back_inserter(result), "{} ", compilation_flag);
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

static auto print_file_compilation_status(const std::filesystem::path& file_name,
                                          const int index,
                                          const int total_num_of_files,
                                          const int width,
                                          const bool before_completion) -> void
{
    if (before_completion) // Omit percentage
    {
        // Example (before completion):
        // 12/20 [    ] src/module/foo.cpp
        std::print("{0:>{2}}/{1} [    ] {3}", index, total_num_of_files, width, file_name.native());
        std::fflush(stdout);
    }
    else // Include percentage.
    {
        // Example (after completion):
        // 12/20 [ 60%] src/module/foo.cpp
        const auto completion_percentage = 100 * index / total_num_of_files;
        std::print("\r"); // Undo previous print.
        std::println(
            "{0:>{2}}/{1} [{3:>3}%] {4}", index, total_num_of_files, width, completion_percentage, file_name.native());
    }
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

    const auto compilation_flags = create_compilation_flags_string(configuration);
    const auto max_index_width   = utils::count_digits(files_to_compile.size()); // For formatting.
    const auto num_of_threads    = use_parallel_compilation ? std::max(1U, std::thread::hardware_concurrency() / 2) : 1;
    ThreadPool thread_pool(num_of_threads);
    std::vector<std::future<CompilationInfo>> futures;

    for (const auto& path : files_to_compile)
    {
        futures.push_back(thread_pool.add_task(
            [&] { return compile_file(path, object_files_directory, compilation_flags, configuration); }));
    }

    std::vector<std::filesystem::path> failed_compilation;

    for (auto [index, future] : std::views::enumerate(futures))
    {
        const auto& file_name = files_to_compile[index];

        if (!is_quiet)
        {
            // Print the file's compilation status *before* starting the blocking `get()`.
            // This marks the file as "in progress" without a completion percentage.
            print_file_compilation_status(file_name, index + 1, files_to_compile.size(), max_index_width, true);
        }

        const auto result = future.get(); // The call to `get` is blocking.

        if (!result.is_successful)
        {
            failed_compilation.push_back(file_name);
        }

        if (!is_quiet)
        {
            // Print the file's status *after* it finishes compiling.
            // This time we include the completion percentage to mark it as completed.
            print_file_compilation_status(file_name, index + 1, files_to_compile.size(), max_index_width, false);
        }

        if (!result.compiler_output.empty())
        {
            std::print("{}", result.compiler_output);
        }
    }

    if (!is_quiet)
    {
        print_compilation_result(failed_compilation);
    }

    return failed_compilation.size();
}

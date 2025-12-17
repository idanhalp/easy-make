#ifndef SOURCE_COMMANDS_BUILD_BUILD_HPP
#define SOURCE_COMMANDS_BUILD_BUILD_HPP

#include <filesystem>
#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

auto get_code_files(const Configuration& configuration,
                    const std::filesystem::path& path_to_root) -> std::vector<std::filesystem::path>;

struct BuildCommandResult
{
    int num_of_files_compiled;       // Both successes and failures.
    int num_of_compilation_failures; // Number of files that failed to compile.
    int exit_status;
};

namespace commands
{
    auto build(const BuildCommandInfo& info,
               const std::vector<Configuration>& configurations,
               const std::filesystem::path& path_to_root) -> BuildCommandResult;
}

#endif // SOURCE_COMMANDS_BUILD_BUILD_HPP

#ifndef SOURCE_COMMANDS_CLEAN_CLEAN_HPP
#define SOURCE_COMMANDS_CLEAN_CLEAN_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

namespace commands
{
    auto clean(const CleanCommandInfo& info,
               const std::vector<Configuration>& configurations,
               const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_CLEAN_HPP

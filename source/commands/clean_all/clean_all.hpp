#ifndef SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP
#define SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP

#include <filesystem>
#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

namespace commands
{
    auto clean_all(const CleanAllCommandInfo& info,
                   const std::vector<Configuration>& configurations,
                   const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP

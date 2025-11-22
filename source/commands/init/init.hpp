#ifndef SOURCE_COMMANDS_INIT_INIT_HPP
#define SOURCE_COMMANDS_INIT_INIT_HPP

#include <filesystem>
#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

namespace commands
{
    auto init(const InitCommandInfo& info, const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_INIT_INIT_HPP

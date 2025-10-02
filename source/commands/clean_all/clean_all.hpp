#ifndef SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP
#define SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP

#include <filesystem>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"

namespace commands
{
    auto clean_all(const std::vector<Configuration>& configurations, const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP

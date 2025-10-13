#ifndef SOURCE_COMMANDS_CLEAN_CLEAN_HPP
#define SOURCE_COMMANDS_CLEAN_CLEAN_HPP

#include <filesystem>
#include <string>
#include <vector>

#include "source/configuration_parsing/configuration.hpp"

namespace commands
{
    auto clean(const std::string& configuration_name,
               const std::vector<Configuration>& configurations,
               const std::filesystem::path& path_to_root,
               bool verbose = true) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_CLEAN_HPP

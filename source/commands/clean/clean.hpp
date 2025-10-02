#ifndef SOURCE_COMMANDS_CLEAN_CLEAN_HPP
#define SOURCE_COMMANDS_CLEAN_CLEAN_HPP

#include <filesystem>
#include <string_view>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"

namespace commands
{
    auto clean(std::string_view configuration_name,
               const std::vector<Configuration>& configurations,
               const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_CLEAN_HPP

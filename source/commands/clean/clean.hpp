#ifndef SOURCE_COMMANDS_CLEAN_HPP
#define SOURCE_COMMANDS_CLEAN_HPP

#include <filesystem>
#include <string_view>

namespace commands
{
    auto clean(std::string_view configuration_name, const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_HPP

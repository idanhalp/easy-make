#ifndef SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP
#define SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP

#include <filesystem>
#include <string_view>

namespace commands
{
    auto clean_all(const std::filesystem::path& path_to_root) -> int;
}

#endif // SOURCE_COMMANDS_CLEAN_CLEAN_ALL_HPP

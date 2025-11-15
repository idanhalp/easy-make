#ifndef SOURCE_COMMANDS_LIST_FILES_LIST_FILES_HPP
#define SOURCE_COMMANDS_LIST_FILES_LIST_FILES_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

namespace commands
{
    auto list_files(const ListFilesCommandInfo& info,
                    const std::vector<Configuration>& configurations,
                    const std::filesystem::path& path_to_root,
                    std::ostream& output = std::cout) -> int;
}

#endif // SOURCE_COMMANDS_LIST_FILES_LIST_FILES_HPP

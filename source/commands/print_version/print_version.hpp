#ifndef SOURCE_COMMANDS_PRINT_VERSION_PRINT_VERSION_HPP
#define SOURCE_COMMANDS_PRINT_VERSION_PRINT_VERSION_HPP

#include "source/argument_parsing/command_info.hpp"

namespace commands
{
    auto print_version(const PrintVersionCommandInfo& info) -> int;
}

#endif // SOURCE_COMMANDS_PRINT_VERSION_PRINT_VERSION_HPP

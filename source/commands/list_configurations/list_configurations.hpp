#ifndef SOURCE_COMMANDS_LIST_CONFIGURATIONS_LIST_CONFIGURATIONS_HPP
#define SOURCE_COMMANDS_LIST_CONFIGURATIONS_LIST_CONFIGURATIONS_HPP

#include <vector>

#include "source/argument_parsing/command_info.hpp"
#include "source/configuration_parsing/configuration.hpp"

namespace commands
{
    auto list_configurations(const ListConfigurationsCommandInfo& info,
                             const std::vector<Configuration>& configurations) -> int;
}

#endif // SOURCE_COMMANDS_LIST_CONFIGURATIONS_LIST_CONFIGURATIONS_HPP

#ifndef SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP
#define SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP

#include <string>
#include <variant>

struct BuildCommandInfo
{
    std::string configuration_name;
};

struct CleanCommandInfo
{
    std::string configuration_name;
    bool is_quiet;
};

struct CleanAllCommandInfo
{
    bool is_quiet;
};

struct ListCommandInfo
{
    bool complete_configurations_only;
    bool count;
    bool incomplete_configurations_only;
    bool porcelain_output;
    bool sorted_output;
};

struct PrintVersionCommandInfo
{
};

using CommandInfo =
    std::variant<BuildCommandInfo, CleanCommandInfo, CleanAllCommandInfo, ListCommandInfo, PrintVersionCommandInfo>;

#endif // SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP

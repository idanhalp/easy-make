#ifndef SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP
#define SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP

#include <string>
#include <variant>

struct CleanCommandInfo
{
    std::string configuration_name;
    bool is_quiet;
};

struct CleanAllCommandInfo
{
};

struct CompileCommandInfo
{
    std::string configuration_name;
};

struct PrintVersionCommandInfo
{
};

using CommandInfo = std::variant<CleanCommandInfo, CleanAllCommandInfo, CompileCommandInfo, PrintVersionCommandInfo>;

#endif // SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP

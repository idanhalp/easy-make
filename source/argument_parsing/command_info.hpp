#ifndef SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP
#define SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP

#include <string>
#include <variant>

struct BuildCommandInfo
{
    std::string configuration_name;
    bool is_quiet;
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

struct InitCommandInfo
{
    bool is_quiet;
    bool overwrite_existing_configuration_file;
};

struct ListConfigurationsCommandInfo
{
    bool complete_configurations_only;
    bool count;
    bool incomplete_configurations_only;
    bool porcelain_output;
    bool sorted_output;
};

struct ListFilesCommandInfo
{
    std::string configuration_name;
    bool count;
    bool header_only;
    bool porcelain_output;
    bool source_only;
};

struct PrintVersionCommandInfo
{
};

using CommandInfo = std::variant<BuildCommandInfo,
                                 CleanCommandInfo,
                                 CleanAllCommandInfo,
                                 InitCommandInfo,
                                 ListConfigurationsCommandInfo,
                                 ListFilesCommandInfo,
                                 PrintVersionCommandInfo>;

#endif // SOURCE_ARGUMENT_PARSING_COMMAND_INFO_HPP

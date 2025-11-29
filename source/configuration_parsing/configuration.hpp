#ifndef SOURCE_CONFIGURATION_PARSING_CONFIGURATION_HPP
#define SOURCE_CONFIGURATION_PARSING_CONFIGURATION_HPP

#include <optional>
#include <string>
#include <vector>

struct Configuration
{
    std::optional<std::string> name;
    std::optional<std::string> parent;
    std::optional<std::string> compiler;
    std::optional<std::string> standard;
    std::optional<std::vector<std::string>> warnings;
    std::optional<std::vector<std::string>> compilation_flags;
    std::optional<std::vector<std::string>> link_flags;
    std::optional<std::string> optimization;
    std::optional<std::vector<std::string>> defines;
    std::optional<std::vector<std::string>> include_directories;
    std::optional<std::vector<std::string>> source_files;
    std::optional<std::vector<std::string>> source_directories;
    std::optional<std::vector<std::string>> excluded_files;
    std::optional<std::vector<std::string>> excluded_directories;
    std::optional<std::string> output_name;
    std::optional<std::string> output_path;

    auto check_for_errors() const -> std::optional<std::string>;
};

#endif // SOURCE_CONFIGURATION_PARSING_CONFIGURATION_HPP

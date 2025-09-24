#ifndef SOURCE_CONFIGURATIONS_PARSING_CONFIGURATIONS_PARSING_HPP
#define SOURCE_CONFIGURATIONS_PARSING_CONFIGURATIONS_PARSING_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

struct Configuration
{
    std::optional<std::string> name;
    std::optional<std::string> compiler;
    std::optional<std::string> standard;
    std::optional<std::vector<std::string>> warnings;
    std::optional<std::string> optimization;
    std::optional<std::vector<std::string>> defines;
    std::optional<std::vector<std::string>> include_directories;
    std::optional<std::vector<std::string>> source_files;
    std::optional<std::vector<std::string>> source_directories;
    std::optional<std::vector<std::string>> excluded_files;
    std::optional<std::vector<std::string>> excluded_directories;
    std::optional<std::string> output_name;
    std::optional<std::string> output_path;
};

auto parse_configurations(const std::filesystem::path& path_to_configurations_file) -> std::vector<Configuration>;

#endif // SOURCE_CONFIGURATIONS_PARSING_CONFIGURATIONS_PARSING_HPP

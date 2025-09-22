#ifndef SOURCE_PARSE_CONFIGURATIONS_PARSE_CONFIGURATIONS_HPP
#define SOURCE_PARSE_CONFIGURATIONS_PARSE_CONFIGURATIONS_HPP

#include <optional>
#include <string>
#include <vector>

struct Configuration
{
    std::optional<std::string> name;
    std::optional<std::string> compiler;
    std::optional<std::string> standard;
    std::vector<std::string> warnings;
    std::optional<std::string> optimization_level;
    std::vector<std::string> defines;
    std::vector<std::string> source_files;
    std::vector<std::string> source_directories;
    std::vector<std::string> excluded_files;
    std::vector<std::string> excluded_directories;
    std::optional<std::string> output_name;
    std::optional<std::string> output_path;

    auto get_error_message() const -> std::optional<std::string>;
};

auto parse_configurations(std::string path_to_configurations_file) -> std::vector<Configuration>;

#endif // SOURCE_PARSE_CONFIGURATIONS_PARSE_CONFIGURATIONS_HPP

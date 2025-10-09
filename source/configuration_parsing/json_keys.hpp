#ifndef SOURCE_CONFIGURATION_PARSING_JSON_KEYS_HPP
#define SOURCE_CONFIGURATION_PARSING_JSON_KEYS_HPP

#include <string>
#include <vector>

namespace json_keys
{
    enum class JsonKey
    {
        Name,
        Parent,
        Warnings,
        Optimization,
        Compiler,
        Standard,
        Defines,
        IncludeDirectories,
        Source,
        SourceFiles,
        SourceDirectories,
        Excludes,
        ExcludedFiles,
        ExcludedDirectories,
        Output,
        OutputName,
        OutputPath,
    };

    auto key_to_string(JsonKey key) -> std::string;
    auto string_to_key(const std::string& s) -> JsonKey;

    auto is_valid_outer_key(const std::string& s) -> bool;
    auto get_valid_outer_keys() -> std::vector<std::string>;

    auto is_valid_source_key(const std::string& s) -> bool;
    auto get_valid_source_keys() -> std::vector<std::string>;

    auto is_valid_excludes_key(const std::string& s) -> bool;
    auto get_valid_excludes_keys() -> std::vector<std::string>;

    auto is_valid_output_key(const std::string& s) -> bool;
    auto get_valid_output_keys() -> std::vector<std::string>;
}

#endif // SOURCE_CONFIGURATION_PARSING_JSON_KEYS_HPP

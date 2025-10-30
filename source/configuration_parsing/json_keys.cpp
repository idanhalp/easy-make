#include "source/configuration_parsing/json_keys.hpp"

#include <flat_map>
#include <flat_set>
#include <ranges>

using namespace json_keys;

static const std::flat_map<JsonKey, std::string> key_to_string_map = {
    {JsonKey::Name,                "name"              },
    {JsonKey::Parent,              "parent"            },
    {JsonKey::Warnings,            "warnings"          },
    {JsonKey::Optimization,        "optimization"      },
    {JsonKey::Compiler,            "compiler"          },
    {JsonKey::Standard,            "standard"          },
    {JsonKey::Defines,             "defines"           },
    {JsonKey::IncludeDirectories,  "includeDirectories"},
    {JsonKey::Source,              "sources"           },
    {JsonKey::SourceFiles,         "files"             },
    {JsonKey::SourceDirectories,   "directories"       },
    {JsonKey::Excludes,            "exclude"           },
    {JsonKey::ExcludedFiles,       "files"             },
    {JsonKey::ExcludedDirectories, "directories"       },
    {JsonKey::Output,              "output"            },
    {JsonKey::OutputName,          "name"              },
    {JsonKey::OutputPath,          "path"              },
};

static const auto string_to_key_map = []
{
    std::flat_map<std::string, JsonKey> result;

    for (const auto& [key, string] : key_to_string_map)
    {
        result.emplace(string, key);
    }

    return result;
}();

static const std::flat_set<std::string> valid_outer_json_keys = {
    key_to_string(JsonKey::Name),     key_to_string(JsonKey::Parent),
    key_to_string(JsonKey::Warnings), key_to_string(JsonKey::Optimization),
    key_to_string(JsonKey::Compiler), key_to_string(JsonKey::Standard),
    key_to_string(JsonKey::Defines),  key_to_string(JsonKey::IncludeDirectories),
    key_to_string(JsonKey::Source),   key_to_string(JsonKey::Excludes),
    key_to_string(JsonKey::Output)};

static const std::flat_set<std::string> valid_source_json_keys = {key_to_string(JsonKey::SourceFiles),
                                                                  key_to_string(JsonKey::SourceDirectories)};

static const std::flat_set<std::string> valid_excludes_json_keys = {key_to_string(JsonKey::ExcludedFiles),
                                                                    key_to_string(JsonKey::ExcludedDirectories)};

static const std::flat_set<std::string> valid_output_json_keys = {key_to_string(JsonKey::OutputName),
                                                                  key_to_string(JsonKey::OutputPath)};

auto json_keys::key_to_string(const JsonKey key) -> std::string { return key_to_string_map.at(key); }

auto json_keys::string_to_key(const std::string& s) -> JsonKey { return string_to_key_map.at(s); }

auto json_keys::is_valid_outer_key(const std::string& s) -> bool { return valid_outer_json_keys.contains(s); }

auto json_keys::get_valid_outer_keys() -> std::vector<std::string>
{
    return valid_outer_json_keys | std::ranges::to<std::vector>();
}

auto json_keys::is_valid_source_key(const std::string& s) -> bool { return valid_source_json_keys.contains(s); }

auto json_keys::get_valid_source_keys() -> std::vector<std::string>
{
    return valid_source_json_keys | std::ranges::to<std::vector>();
}

auto json_keys::is_valid_excludes_key(const std::string& s) -> bool { return valid_excludes_json_keys.contains(s); }

auto json_keys::get_valid_excludes_keys() -> std::vector<std::string>
{
    return valid_excludes_json_keys | std::ranges::to<std::vector>();
}

auto json_keys::is_valid_output_key(const std::string& s) -> bool { return valid_output_json_keys.contains(s); }

auto json_keys::get_valid_output_keys() -> std::vector<std::string>
{
    return valid_output_json_keys | std::ranges::to<std::vector>();
}

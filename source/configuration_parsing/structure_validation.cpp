#include "source/configuration_parsing/structure_validation.hpp"

#include <algorithm>
#include <format>
#include <iterator> // std::back_inserter
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

#include "source/configuration_parsing/json_keys.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/utils.hpp"

using namespace json_keys;
using namespace nlohmann;

static auto type_to_string(const json::value_t type) -> std::string_view
{
    switch (type)
    {
    case json::value_t::null:
        return "null";
    case json::value_t::object:
        return "object";
    case json::value_t::array:
        return "array";
    case json::value_t::string:
        return "string";
    case json::value_t::boolean:
        return "boolean";
    case json::value_t::binary:
        return "binary";
    case json::value_t::discarded:
        return "discarded";
    case json::value_t::number_integer:
    case json::value_t::number_unsigned:
    case json::value_t::number_float:
        return "number";
    default:
        return "invalid";
    }
}

static auto create_invalid_key_massage(const std::string_view key_name,
                                       const int configuration_index,
                                       const std::vector<std::string>& candidates) -> std::string
{
    auto error_message = std::format("Error: Invalid JSON - the {}{} configuration contains an unknown key '{}'.",
                                     configuration_index,
                                     utils::get_ordinal_indicator(configuration_index),
                                     key_name);

    // If `key_name` is a concatenation of outer and inner keys (e.g "output.path"),
    // we need to extract them separately as `utils::find_closest_word` cannot handle concatenated keys.
    const auto dot_position   = key_name.find('.');
    const auto has_nested_key = dot_position != std::string_view::npos;
    const auto outer_key      = has_nested_key ? key_name.substr(0, dot_position + 1) : std::string_view{};
    const auto inner_key      = has_nested_key ? key_name.substr(dot_position + 1) : key_name;

    if (const auto closest_key = utils::find_closest_word(inner_key, candidates); closest_key.has_value())
    {
        std::format_to(std::back_inserter(error_message), " Did you mean '{}{}'?", outer_key, *closest_key);
    }

    return error_message;
}

static auto create_invalid_array_element_message(const std::string_view key,
                                                 const std::string_view expected_type,
                                                 const std::string_view actual_type,
                                                 const int configuration_index,
                                                 const int element_index) -> std::string
{
    return std::format("Error: Invalid JSON - the value of '{}' at index {} in the {}{} configuration must be '{}', "
                       "but got '{}'.",
                       key,
                       element_index,
                       configuration_index,
                       utils::get_ordinal_indicator(configuration_index),
                       expected_type,
                       actual_type);
}

static auto create_invalid_type_message(const std::string_view key,
                                        const std::string_view expected_type,
                                        const std::string_view actual_type,
                                        const int configuration_index) -> std::string
{
    return std::format("Error: Invalid JSON - the value of '{}' in the {}{} configuration must be '{}', "
                       "but got '{}'.",
                       key,
                       configuration_index,
                       utils::get_ordinal_indicator(configuration_index),
                       expected_type,
                       actual_type);
}

static auto get_expected_outer_value_type(const std::string& key_name) -> std::optional<json::value_t>
{
    if (!is_valid_outer_key(key_name))
    {
        return std::nullopt;
    }

    const auto key = string_to_key(key_name);

    switch (key)
    {
    case JsonKey::Name:
    case JsonKey::Parent:
    case JsonKey::Compiler:
    case JsonKey::Standard:
    case JsonKey::Optimization:
        return json::value_t::string;

    case JsonKey::Warnings:
    case JsonKey::CompilationFlags:
    case JsonKey::LinkFlags:
    case JsonKey::Defines:
    case JsonKey::IncludeDirectories:
        return json::value_t::array;

    case JsonKey::Source:
    case JsonKey::Excludes:
    case JsonKey::Output:
        return json::value_t::object;

    default:
        return std::nullopt;
    }
}

static auto get_expected_inner_value_type(const std::string& key_name,
                                          const std::string& parent_key_name) -> std::optional<json::value_t>
{
    ASSERT(is_valid_outer_key(parent_key_name));

    if (parent_key_name == key_to_string(JsonKey::Source))
    {
        if (key_name == key_to_string(JsonKey::SourceFiles) || key_name == key_to_string(JsonKey::SourceDirectories))
        {
            return json::value_t::array;
        }
    }
    else if (parent_key_name == key_to_string(JsonKey::Excludes))
    {
        if (key_name == key_to_string(JsonKey::ExcludedFiles) ||
            key_name == key_to_string(JsonKey::ExcludedDirectories))
        {
            return json::value_t::array;
        }
    }
    else if (parent_key_name == key_to_string(JsonKey::Output))
    {
        if (key_name == key_to_string(JsonKey::OutputName) || key_name == key_to_string(JsonKey::OutputPath))
        {
            return json::value_t::string;
        }
    }

    return std::nullopt;
}

static auto validate_configuration_structure(const json& json,
                                             const int configuration_index) -> std::optional<std::string>
{
    for (const auto& [outer_key, value] : json.items())
    {
        const auto expected_value_type = get_expected_outer_value_type(outer_key);
        const auto key_is_invalid      = !expected_value_type.has_value();

        if (key_is_invalid)
        {
            return create_invalid_key_massage(outer_key, configuration_index, get_valid_outer_keys());
        }

        const auto actual_value_type = value.type();
        const auto type_is_correct   = *expected_value_type == actual_value_type;

        if (!type_is_correct)
        {
            return create_invalid_type_message(outer_key,
                                               type_to_string(*expected_value_type),
                                               type_to_string(actual_value_type),
                                               configuration_index);
        }
        else if (actual_value_type == json::value_t::array)
        {
            // Make sure that all the array's elements are strings.
            const auto invalid_element_position = std::ranges::find_if_not(value, &json::is_string);
            const auto invalid_element_exists   = invalid_element_position != value.end();

            if (invalid_element_exists)
            {
                const auto element_index = std::ranges::distance(value.begin(), invalid_element_position);

                return create_invalid_array_element_message(
                    outer_key, "string", value.at(element_index).type_name(), configuration_index, element_index);
            }
        }
        else if (actual_value_type == json::value_t::object)
        {
            for (const auto& [inner_key, inner_value] : value.items())
            {
                const auto expected_inner_value_type = get_expected_inner_value_type(inner_key, outer_key);
                const auto inner_key_is_valid        = expected_inner_value_type.has_value();

                if (!inner_key_is_valid)
                {
                    return create_invalid_key_massage(std::format("{}.{}", outer_key, inner_key),
                                                      configuration_index,
                                                      get_related_inner_key_names(outer_key));
                }

                const auto actual_inner_value_type = inner_value.type();
                const auto inner_type_is_correct   = *expected_inner_value_type == actual_inner_value_type;

                if (!inner_type_is_correct)
                {
                    return create_invalid_type_message(std::format("{}.{}", outer_key, inner_key),
                                                       type_to_string(*expected_inner_value_type),
                                                       type_to_string(actual_inner_value_type),
                                                       configuration_index);
                }
            }
        }
    }

    return std::nullopt; // Everything is fine.
}

auto validate_json_structure(const json& json) -> std::optional<std::string>
{
    const auto json_is_array_of_objects = json.is_array() && std::ranges::all_of(json, &json::is_object);

    if (!json_is_array_of_objects)
    {
        return "Error: Invalid JSON - must be an array of objects.";
    }

    for (const auto [index, configuration_json] : std::views::enumerate(json))
    {
        const auto configuration_structure_error = validate_configuration_structure(configuration_json, index + 1);
        const auto structure_is_valid            = !configuration_structure_error.has_value();

        if (!structure_is_valid)
        {
            return *configuration_structure_error;
        }
    }

    return std::nullopt;
}

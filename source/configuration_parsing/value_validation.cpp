#include "source/configuration_parsing/value_validation.hpp"

#include <algorithm>
#include <format>
#include <iterator> // std::back_inserter
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/graph.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/utils.hpp" // utils::get_ordinal_indicator

using namespace std::literals;

/// @brief  Validates that each configuration has a name and that all names are unique.
/// @param  configurations  The list of configurations to validate.
/// @return `std::nullopt` if all names are valid; otherwise, an error message
///         describing the first missing or duplicate name encountered.
static auto validate_names(const std::vector<Configuration>& configurations) -> std::optional<std::string>
{
    std::unordered_map<std::string_view, std::ptrdiff_t> name_to_index;

    for (const auto [index, configuration] : std::views::enumerate(configurations) | std::views::as_const)
    {
        const auto actual_index           = index + 1;
        const auto configuration_has_name = configuration.name.has_value();

        if (!configuration_has_name)
        {
            return std::format("Error: The {}{} configuration does not have a name.",
                               actual_index,
                               utils::get_ordinal_indicator(actual_index));
        }

        const auto& name            = *configuration.name;
        const auto name_seen_before = !name_to_index.insert({name, actual_index}).second;

        if (name_seen_before)
        {
            const auto previous_index = name_to_index.at(name);

            return std::format("Error: Both the {}{} and {}{} configurations have '{}' as name.",
                               previous_index,
                               utils::get_ordinal_indicator(previous_index),
                               actual_index,
                               utils::get_ordinal_indicator(actual_index),
                               name);
        }
    }

    return std::nullopt;
}

static auto validate_parents(const std::vector<Configuration>& configurations) -> std::optional<std::string>
{
    // Extract names in case we find a non-existent parent
    // and want to find the closest name.
    const auto configuration_names = [&]
    {
        std::vector<std::string_view> names;
        names.reserve(configurations.size());

        for (const auto& configuration : configurations)
        {
            ASSERT(configuration.name.has_value());
            names.push_back(*configuration.name);
        }

        std::ranges::sort(names); // So we can use binary search in the future.

        return names;
    }();

    utils::DirectedGraph<std::string_view> parent_graph{};

    for (const auto& configuration : configurations)
    {
        const auto configuration_has_parent = configuration.parent.has_value();

        if (!configuration_has_parent)
        {
            continue;
        }

        const auto configuration_is_its_own_parent = configuration.name == configuration.parent;

        if (configuration_is_its_own_parent)
        {
            return std::format("Error: Configuration '{}' has itself as a parent.", *configuration.name);
        }

        ASSERT(std::ranges::is_sorted(configuration_names));
        const auto parent_exists = std::ranges::binary_search(configuration_names, *configuration.parent);

        if (parent_exists)
        {
            parent_graph.add_edge(*configuration.name, *configuration.parent);
            continue;
        }

        auto non_existent_parent_error =
            std::format("Error: Configuration '{}' has a non-existent parent configuration '{}'.",
                        *configuration.name,
                        *configuration.parent);

        if (const auto closest_name = utils::find_closest_word(*configuration.parent, configuration_names);
            closest_name.has_value())
        {
            std::format_to(std::back_inserter(non_existent_parent_error), " Did you mean '{}'?", *closest_name);
        }

        return non_existent_parent_error;
    }

    const auto cycle_info   = parent_graph.check_for_cycle();
    const auto cycle_exists = cycle_info.has_value();

    if (cycle_exists)
    {
        return std::format("Error: Circular parent dependency detected.\n\n"
                           "The following configurations form a cycle:\n"
                           "{}\n\n"
                           "Consider restructuring the code to break the circular dependency.",
                           *cycle_info);
    }
    else
    {
        return std::nullopt;
    }
}

static auto validate_compiler(const Configuration& configuration) -> std::optional<std::string>
{
    constexpr auto GCC                = "g++"sv;
    constexpr auto CLANG              = "clang++"sv;
    constexpr auto MSVC               = "cl"sv;
    static const auto valid_compilers = params::ENABLE_MSVC ? std::vector{GCC, CLANG, MSVC} : std::vector{GCC, CLANG};

    if (!configuration.compiler.has_value() || std::ranges::contains(valid_compilers, *configuration.compiler))
    {
        return std::nullopt;
    }

    auto invalid_compiler_error = std::format(
        "Error: Configuration '{}' has an unknown compiler '{}'.", *configuration.name, *configuration.compiler);

    if (const auto closest_compiler = utils::find_closest_word(*configuration.compiler, valid_compilers);
        closest_compiler.has_value())
    {
        std::format_to(std::back_inserter(invalid_compiler_error), " Did you mean '{}'?", *closest_compiler);
    }

    return invalid_compiler_error;
}

static auto validate_standard(const Configuration& configuration) -> std::optional<std::string>
{
    static const auto valid_standards = std::vector{
        "98",
        "03",
        "11",
        "14",
        "17",
        "20",
        "23",
        "26",
    };

    if (!configuration.standard.has_value() || std::ranges::contains(valid_standards, *configuration.standard))
    {
        return std::nullopt;
    }

    return std::format(
        "Error: Configuration '{}' has an unknown standard '{}'.", *configuration.name, *configuration.standard);
}

static auto validate_warnings(const Configuration& configuration) -> std::optional<std::string>
{
    if (!configuration.warnings.has_value())
    {
        return std::nullopt;
    }

    static_assert(!params::ENABLE_MSVC, "This implementation does not handle MSVC warnings.");
    const auto is_valid_warning       = [](const std::string_view s) { return s.starts_with("-W") && s != "-W"; };
    const auto invalid_warning        = std::ranges::find_if_not(*configuration.warnings, is_valid_warning);
    const auto all_warnings_are_valid = invalid_warning == configuration.warnings->end();

    if (all_warnings_are_valid)
    {
        return std::nullopt;
    }

    const auto invalid_warning_index = std::ranges::distance(configuration.warnings->begin(), invalid_warning);

    return std::format("Error: Configuration '{}' has an invalid warning '{}' at index {}.",
                       *configuration.name,
                       *invalid_warning,
                       invalid_warning_index);
}

static auto validate_optimization(const Configuration& configuration) -> std::optional<std::string>
{
    static_assert(!params::ENABLE_MSVC, "This implementation does not handle MSVC optimizations.");
    const auto valid_optimization_levels = std::vector{
        "0"sv,
        "1"sv,
        "2"sv,
        "3"sv,
        "s"sv,
        "fast"sv,
    };

    if (!configuration.optimization.has_value() ||
        std::ranges::contains(valid_optimization_levels, *configuration.optimization))
    {
        return std::nullopt;
    }

    return std::format("Error: Configuration '{}' has an unknown optimization '{}'.",
                       *configuration.name,
                       *configuration.optimization);
}

static auto validate_sources_and_excludes(const Configuration& configuration,
                                          const std::filesystem::path& path_to_root) -> std::optional<std::string>
{
    if (configuration.source_files.has_value())
    {
        for (const auto& file : *configuration.source_files)
        {
            constexpr auto short_error_format = "Error: Configuration '{}' lists '{}' in 'sources.files', which {}.";
            constexpr auto long_error_format  = "Error: Configuration '{}' lists '{}' in 'sources.files', which {}.\n"
                                                "\n"
                                                "The valid file extensions are:\n"
                                                "* .cpp\n"
                                                "* .cc\n"
                                                "* .cxx\n"
                                                "\n"
                                                "See "
                                                "<https://github.com/idanhalp/easy-make/blob/master/documentation/"
                                                "easy-make-configurations-reference.md> for more information."sv;

            static_assert(long_error_format.starts_with(short_error_format));

            if (utils::is_header_file(file))
            {
                return std::format(long_error_format, *configuration.name, file, "is a header file");
            }
            if (!utils::is_source_file(file))
            {
                return std::format(long_error_format, *configuration.name, file, "has an unsupported file extension");
            }

            const auto full_path   = path_to_root / file;
            const auto file_exists = std::filesystem::is_regular_file(full_path);

            if (!file_exists)
            {
                return std::format(short_error_format, *configuration.name, file, "could not be found");
            }
        }
    }

    if (configuration.source_directories.has_value())
    {
        for (const auto& directory : *configuration.source_directories)
        {
            if (!std::filesystem::is_directory(path_to_root / directory))
            {
                return std::format("Error: Configuration '{}' has a non-existent source directory '{}'.",
                                   *configuration.name,
                                   directory);
            }
        }
    }

    if (configuration.excluded_files.has_value())
    {
        for (const auto& file : *configuration.excluded_files)
        {
            if (!std::filesystem::is_regular_file(path_to_root / file))
            {
                return std::format(
                    "Error: Configuration '{}' has a non-existent excluded file '{}'.", *configuration.name, file);
            }
        }
    }

    if (configuration.excluded_directories.has_value())
    {
        for (const auto& directory : *configuration.excluded_directories)
        {
            if (!std::filesystem::is_directory(path_to_root / directory))
            {
                return std::format("Error: Configuration '{}' has a non-existent excluded directory '{}'.",
                                   *configuration.name,
                                   directory);
            }
        }
    }

    return std::nullopt;
}

static auto validate_arguments(const std::vector<Configuration>& configurations,
                               const std::filesystem::path& path_to_root) -> std::optional<std::string>
{
    for (const auto& configuration : configurations)
    {
        if (const auto compiler_error = validate_compiler(configuration); compiler_error.has_value())
        {
            return *compiler_error;
        }
        if (const auto standard_error = validate_standard(configuration); standard_error.has_value())
        {
            return *standard_error;
        }
        if (const auto warnings_error = validate_warnings(configuration); warnings_error.has_value())
        {
            return *warnings_error;
        }
        if (const auto optimization_error = validate_optimization(configuration); optimization_error.has_value())
        {
            return *optimization_error;
        }
        if (const auto sources_error = validate_sources_and_excludes(configuration, path_to_root);
            sources_error.has_value())
        {
            return *sources_error;
        }
    }

    return std::nullopt;
}

auto validate_configuration_values(const std::vector<Configuration>& configurations,
                                   const std::filesystem::path& path_to_root) -> std::optional<std::string>
{
    const auto name_error             = validate_names(configurations);
    const auto found_error_with_names = name_error.has_value();

    if (found_error_with_names)
    {
        return *name_error;
    }

    const auto parent_error             = validate_parents(configurations);
    const auto found_error_with_parents = parent_error.has_value();

    if (found_error_with_parents)
    {
        return *parent_error;
    }

    const auto argument_error             = validate_arguments(configurations, path_to_root);
    const auto found_error_with_arguments = argument_error.has_value();

    if (found_error_with_arguments)
    {
        return *argument_error;
    }

    return std::nullopt;
}

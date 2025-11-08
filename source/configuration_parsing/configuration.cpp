#include "source/configuration_parsing/configuration.hpp"

#include <algorithm>
#include <format>

#include "source/parameters/parameters.hpp"
#include "source/utils/find_closest_word.hpp"
#include "source/utils/macros/assert.hpp"

auto Configuration::check_for_errors() const -> std::optional<std::string>
{
    ASSERT(name.has_value());

    // Check for obligatory fields.

    if (!compiler.has_value())
    {
        return std::format("Error: Could not resolve 'compiler' for '{}'.", *name);
    }

    if (!output_name.has_value())
    {
        return std::format("Error: Could not resolve 'output.name' for '{}'.", *name);
    }

    // Check for invalid values.

    // --- Name ---
    const auto name_is_valid = !name->empty();

    if (!name_is_valid)
    {
        return std::format("Error: empty configuration name.");
    }

    // --- Compiler ---
    static const auto valid_compilers = (params::ENABLE_MSVC ? std::vector<std::string>{"g++", "clang++", "cl"}
                                                             : std::vector<std::string>{"g++", "clang++"});

    const auto compiler_is_valid = std::ranges::contains(valid_compilers, *compiler);

    if (!compiler_is_valid)
    {
        const auto closest_compiler = utils::find_closest_word(*compiler, valid_compilers);

        return closest_compiler.has_value()
                   ? std::format("Error: Configuration '{}' - unknown compiler '{}'. Did you mean '{}'?",
                                 *name,
                                 *compiler,
                                 *closest_compiler)
                   : std::format("Error: Configuration '{}' - unknown compiler '{}'.", *name, *compiler);
    }

    // --- Standard ---
    static const auto valid_standards = {"98", "03", "11", "14", "17", "20", "23", "26"};
    const auto standard_is_valid      = !standard.has_value() || std::ranges::contains(valid_standards, *standard);

    if (!standard_is_valid)
    {
        return std::format("Error: Configuration '{}' - unknown standard '{}'.", *name, *standard);
    }

    // --- Warnings ---
    if (warnings.has_value())
    {
        static const auto is_valid_warning = [&](const std::string_view warning)
        {
            return *compiler == "cl"
                       ? (warning.starts_with("/W") || warning.starts_with("/w"))
                       : (warning.starts_with("-W") || warning == "-pedantic" || warning == "-pedantic-errors");
        };

        const auto invalid_warning        = std::ranges::find_if_not(*warnings, is_valid_warning);
        const auto invalid_warning_exists = invalid_warning != warnings->end();

        if (invalid_warning_exists)
        {
            return std::format("Error: Configuration '{}' - warning '{}' is invalid when compiling with '{}'.",
                               *name,
                               *invalid_warning,
                               *compiler);
        }
    }

    // --- Optimization ---
    static const auto gcc_and_clang_optimizations = {"0", "1", "2", "3", "s", "fast"};
    static const auto msvc_optimizations          = {"d", "1", "2", "3", "s", "x"};

    const auto optimization_is_valid =
        !optimization.has_value() ||
        (*compiler != "cl" && std::ranges::contains(gcc_and_clang_optimizations, *optimization)) ||
        (*compiler == "cl" && std::ranges::contains(msvc_optimizations, *optimization));

    if (!optimization_is_valid)
    {
        const auto optimization_mismatched_with_compiler =
            (*compiler == "cl" && std::ranges::contains(gcc_and_clang_optimizations, *optimization)) ||
            (*compiler != "cl" && std::ranges::contains(msvc_optimizations, *optimization));

        if (params::ENABLE_MSVC && optimization_mismatched_with_compiler)
        {
            const auto correct_compiler = *compiler == "cl" ? "'g++' or 'clang++'" : "'cl'";

            return std::format("Error: Configuration '{}' - optimization '{}' does not match specified compiler '{}'. "
                               "Did you mean to compile with {} instead?",
                               *name,
                               *optimization,
                               *compiler,
                               correct_compiler);
        }
        else
        {
            return std::format("Error: Configuration '{}' - unknown optimization '{}'.", *name, *optimization);
        }
    }

    return std::nullopt;
}

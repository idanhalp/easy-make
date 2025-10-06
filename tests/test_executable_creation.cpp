#include "tests/tests.hpp"

#include <algorithm>
#include <cassert>
#include <print>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/executable_creation/executable_creation.hpp"
#include "tests/utils/utils.hpp"

static auto test_check_names_validity_1() -> void
{
    std::vector<Configuration> configurations(3);
    configurations[0].name = "name-1";
    configurations[2].name = "name-3";

    const auto names_error = check_names_validity(configurations);
    assert(names_error.has_value() && *names_error == "Error: The 2nd configuration does not have a name.");
}

static auto test_check_names_validity_2() -> void
{
    std::vector<Configuration> configurations(3);
    configurations[0].name = "name-1";
    configurations[1].name = "name-1";
    configurations[2].name = "name-3";

    const auto names_error = check_names_validity(configurations);
    assert(names_error.has_value() &&
           *names_error == "Error: Both the 1st and 2nd configurations have 'name-1' as name.");
}

static auto test_check_names_validity_3() -> void
{
    std::vector<Configuration> configurations(3);
    configurations[0].name = "name-1";
    configurations[1].name = "name-2";
    configurations[2].name = "name-3";

    const auto names_error = check_names_validity(configurations);
    assert(!names_error.has_value());
}

static auto test_check_parents_validity_1() -> void
{
    std::vector<Configuration> configurations(3);
    configurations[0].name = "name-1";
    configurations[1].name = "name-2";
    configurations[2].name = "name-3";

    configurations[1].parent = "name-2";
    configurations[2].parent = "name-1";

    std::unordered_map<std::string, Configuration> name_to_configuration;

    for (const auto& configuration : configurations)
    {
        name_to_configuration[*configuration.name] = configuration;
    }

    const auto parent_error = check_parents_validity(name_to_configuration);
    assert(parent_error.has_value() && *parent_error == "Error: Configuration 'name-2' has itself as a parent.");
}

static auto test_check_parents_validity_2() -> void
{
    std::vector<Configuration> configurations(3);
    configurations[0].name = "name-1";
    configurations[1].name = "name-2";
    configurations[2].name = "name-3";

    configurations[0].parent = "non-existent";
    configurations[1].parent = "name-1";
    configurations[2].parent = "name-1";

    std::unordered_map<std::string, Configuration> name_to_configuration;

    for (const auto& configuration : configurations)
    {
        name_to_configuration[*configuration.name] = configuration;
    }

    const auto parent_error = check_parents_validity(name_to_configuration);
    assert(parent_error.has_value() &&
           *parent_error ==
               "Error: Configuration 'name-1' has a non-existent configuration as its parent ('non-existent').");
}

static auto test_check_parents_validity_3() -> void
{
    std::vector<Configuration> configurations(3);
    configurations[0].name = "name-1";
    configurations[1].name = "name-2";
    configurations[2].name = "name-3";

    configurations[1].parent = "name-1";
    configurations[2].parent = "name-2";

    std::unordered_map<std::string, Configuration> name_to_configuration;

    for (const auto& configuration : configurations)
    {
        name_to_configuration[*configuration.name] = configuration;
    }

    const auto parent_error = check_parents_validity(name_to_configuration);
    assert(!parent_error.has_value());
}

static auto test_get_actual_configuration_without_compiler() -> void
{
    Configuration configuration;
    configuration.name        = "test-configuration";
    configuration.output_name = "output";
    configuration.output_path = "build";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Error: Could not resolve 'compiler' for 'test-configuration'.");
}

static auto test_get_actual_configuration_without_output_name() -> void
{
    Configuration configuration;
    configuration.name        = "test-configuration";
    configuration.compiler    = "g++";
    configuration.output_path = "build";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Error: Could not resolve 'output.name' for 'test-configuration'.");
}

static auto test_actual_configuration_with_overridden_fields_1() -> void
{
    Configuration default_configuration;
    default_configuration.name         = "default";
    default_configuration.compiler     = "clang++";
    default_configuration.output_name  = "output";
    default_configuration.output_path  = "build";
    default_configuration.optimization = "2";

    Configuration test_configuration;
    test_configuration.name         = "test";
    test_configuration.parent       = "default";
    test_configuration.optimization = "3";
    test_configuration.output_name  = "test-output";

    const auto actual_configuration = get_actual_configuration("test", {test_configuration, default_configuration});
    assert(actual_configuration.has_value());
    assert(actual_configuration->name == "test");
    assert(actual_configuration->compiler == "clang++");
    assert(actual_configuration->output_name == "test-output");
    assert(actual_configuration->output_path == "build");
    assert(actual_configuration->optimization == "3");
}

static auto test_actual_configuration_with_overridden_fields_2() -> void
{
    Configuration configuration_0;
    configuration_0.name        = "config-0";
    configuration_0.compiler    = "g++";
    configuration_0.defines     = {"DEF0"};
    configuration_0.output_name = "output.exe";

    Configuration configuration_1;
    configuration_1.name    = "config-1";
    configuration_1.parent  = "config-0";
    configuration_1.defines = {"DEF0", "DEF1"};

    Configuration configuration_2;
    configuration_2.name     = "config-2";
    configuration_2.parent   = "config-1";
    configuration_2.compiler = "clang++";

    const auto actual_configuration =
        get_actual_configuration("config-2", {configuration_0, configuration_1, configuration_2});
    assert(actual_configuration.has_value());
    assert(actual_configuration->name == configuration_2.name);
    assert(actual_configuration->compiler == configuration_2.compiler);
    assert(actual_configuration->defines == configuration_1.defines);
    assert(actual_configuration->output_name == configuration_0.output_name);
}

static auto test_actual_configuration_with_invalid_name() -> void
{
    Configuration configuration;
    configuration.compiler    = "g++";
    configuration.name        = ""; // Invalid.
    configuration.output_name = "test.exe";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Error: empty configuration name.");
}

static auto test_actual_configuration_with_invalid_compiler() -> void
{
    Configuration configuration;
    configuration.compiler    = "rustc"; // Invalid!!
    configuration.name        = "test";
    configuration.output_name = "test.exe";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Error: Configuration 'test' - unknown compiler 'rustc'.");
}

static auto test_actual_configuration_with_invalid_standard() -> void
{
    Configuration configuration;
    configuration.compiler    = "g++";
    configuration.name        = "test";
    configuration.output_name = "test.exe";
    configuration.standard    = "27"; // Invalid

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Error: Configuration 'test' - unknown standard '27'.");
}

static auto test_actual_configuration_with_invalid_warnings_1() -> void
{
    Configuration configuration;
    configuration.compiler    = "cl";
    configuration.name        = "test";
    configuration.output_name = "test.exe";
    configuration.warnings    = {"/W3", "/Wall", "/wd4996", "invalid-warning", "/WX"}; // Invalid

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() &&
           result.error() ==
               "Error: Configuration 'test' - warning 'invalid-warning' is invalid when compiling with 'cl'.");
}

static auto test_actual_configuration_with_invalid_warnings_2() -> void
{
    Configuration configuration;
    configuration.compiler    = "g++";
    configuration.name        = "test";
    configuration.output_name = "test.exe";
    configuration.warnings    = {"-Wall", "-Wextra", "-pedantic", "invalid-warning", "-pedantic-errors"}; // Invalid

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() &&
           result.error() ==
               "Error: Configuration 'test' - warning 'invalid-warning' is invalid when compiling with 'g++'.");
}

static auto test_actual_configuration_with_invalid_optimization() -> void
{
    Configuration configuration;
    configuration.compiler     = "g++";
    configuration.name         = "test";
    configuration.output_name  = "test.exe";
    configuration.optimization = "4";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Error: Configuration 'test' - unknown optimization '4'.");
}

static auto test_actual_configuration_with_mismatched_optimization_1() -> void
{
    Configuration configuration;
    configuration.compiler     = "g++";
    configuration.name         = "test";
    configuration.output_name  = "test.exe";
    configuration.optimization = "d";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() &&
           result.error() == "Error: Configuration 'test' - optimization 'd' does not match specified compiler 'g++'. "
                             "Did you mean to compile with 'cl' instead?");
}

static auto test_actual_configuration_with_mismatched_optimization_2() -> void
{
    Configuration configuration;
    configuration.compiler     = "cl";
    configuration.name         = "test";
    configuration.output_name  = "test.exe";
    configuration.optimization = "0";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() &&
           result.error() == "Error: Configuration 'test' - optimization '0' does not match specified compiler 'cl'. "
                             "Did you mean to compile with 'g++' or 'clang++' instead?");
}

static auto test_get_source_files() -> void
{
    const auto project_5_path       = tests::utils::get_path_to_resources_project(5);
    const auto configurations       = parse_configurations(project_5_path);
    const auto actual_configuration = get_actual_configuration("default", configurations);

    assert(actual_configuration.has_value());

    const auto files_to_compile = get_source_files(*actual_configuration, project_5_path);
    assert(files_to_compile.size() == 3);
    assert(std::ranges::contains(files_to_compile, "main.cpp"));
    assert(std::ranges::contains(files_to_compile, "source/f_1.cpp"));
    assert(std::ranges::contains(files_to_compile, "source/dir_2/f_4.cxx"));
}

static auto test_create_compilation_flags_string_1() -> void
{
    Configuration configuration;
    configuration.name                = "test";
    configuration.compiler            = "clang++";
    configuration.standard            = "20";
    configuration.warnings            = {"-Wall", "-Werror"};
    configuration.optimization        = "2";
    configuration.defines             = {"DEBUG", "VERSION=12"};
    configuration.include_directories = {".", "source", "test"};

    assert(create_compilation_flags_string(configuration) ==
           "-std=c++20 -Wall -Werror -O2 -DDEBUG -DVERSION=12 -I. -Isource -Itest");
}

static auto test_create_compilation_flags_string_2() -> void
{
    Configuration configuration;
    configuration.name                = "test";
    configuration.compiler            = "cl";
    configuration.standard            = "98";
    configuration.optimization        = "d";
    configuration.defines             = {"DEBUG", "VERSION=12"};
    configuration.include_directories = {".", "source", "test"};

    assert(create_compilation_flags_string(configuration) == "-std=c++98 /Od -DDEBUG -DVERSION=12 -I. -Isource -Itest");
}

auto tests::test_executable_creation() -> void
{
    std::println("Running `create_executable_tests` tests.");

    test_check_names_validity_1();
    test_check_names_validity_2();
    test_check_names_validity_3();
    test_check_parents_validity_1();
    test_check_parents_validity_2();
    test_check_parents_validity_3();
    test_get_actual_configuration_without_compiler();
    test_get_actual_configuration_without_output_name();
    test_actual_configuration_with_overridden_fields_1();
    test_actual_configuration_with_overridden_fields_2();
    test_actual_configuration_with_invalid_name();
    test_actual_configuration_with_invalid_compiler();
    test_actual_configuration_with_invalid_standard();
    test_actual_configuration_with_invalid_warnings_1();
    test_actual_configuration_with_invalid_warnings_2();
    test_actual_configuration_with_invalid_optimization();
    test_actual_configuration_with_mismatched_optimization_1();
    test_actual_configuration_with_mismatched_optimization_2();
    test_get_source_files();
    test_create_compilation_flags_string_1();
    test_create_compilation_flags_string_2();

    std::println("Done.");
}

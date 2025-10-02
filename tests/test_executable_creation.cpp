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
    assert(!result.has_value() && result.error() == "Could not resolve 'compiler' for 'test-configuration'.");
}

static auto test_get_actual_configuration_without_output_name() -> void
{
    Configuration configuration;
    configuration.name        = "test-configuration";
    configuration.compiler    = "g++";
    configuration.output_path = "build";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Could not resolve 'output.name' for 'test-configuration'.");
}

static auto test_actual_configuration_with_overridden_fields_1() -> void
{
    Configuration default_configuration;
    default_configuration.name         = "default";
    default_configuration.compiler     = "clang++";
    default_configuration.output_name  = "output";
    default_configuration.output_path  = "build";
    default_configuration.optimization = "-O2";

    Configuration test_configuration;
    test_configuration.name         = "test";
    test_configuration.parent       = "default";
    test_configuration.optimization = "-O3";
    test_configuration.output_name  = "test-output";

    const auto actual_configuration = get_actual_configuration("test", {test_configuration, default_configuration});
    assert(actual_configuration.has_value());
    assert(actual_configuration->name == "test");
    assert(actual_configuration->compiler == "clang++");
    assert(actual_configuration->output_name == "test-output");
    assert(actual_configuration->output_path == "build");
    assert(actual_configuration->optimization == "-O3");
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

static auto test_actual_configuration_with_invalid_default() -> void
{
    Configuration configuration;
    configuration.name     = "default";
    configuration.compiler = "clang++";

    const auto result = get_actual_configuration(*configuration.name, {configuration});
    assert(!result.has_value() && result.error() == "Could not resolve 'output.name' for 'default'.");
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

static auto test_create_compilation_flags_string() -> void
{
    Configuration configuration;
    configuration.name                = "test";
    configuration.standard            = "c++20";
    configuration.warnings            = {"-Wall", "-Werror"};
    configuration.optimization        = "-O2";
    configuration.defines             = {"DEBUG", "VERSION=12"};
    configuration.include_directories = {".", "source", "test"};

    assert(create_compilation_flags_string(configuration) ==
           "-std=c++20 -Wall -Werror -O2 -DDEBUG -DVERSION=12 -I. -Isource -Itest");
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
    test_actual_configuration_with_invalid_default();
    test_get_source_files();
    test_create_compilation_flags_string();

    std::println("Done.");
}

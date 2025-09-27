#include "tests/tests.hpp"

#include <algorithm>
#include <cassert>
#include <print>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/executable_creation/executable_creation.hpp"
#include "tests/utils/utils.hpp"

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

static auto test_actual_configuration_with_overridden_fields() -> void
{
    Configuration default_configuration;
    default_configuration.name         = "default";
    default_configuration.compiler     = "clang++";
    default_configuration.output_name  = "output";
    default_configuration.output_path  = "build";
    default_configuration.optimization = "-O2";

    Configuration test_configuration;
    test_configuration.name         = "test";
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

    test_get_actual_configuration_without_compiler();
    test_get_actual_configuration_without_output_name();
    test_actual_configuration_with_overridden_fields();
    test_actual_configuration_with_invalid_default();
    test_get_source_files();
    test_create_compilation_flags_string();

    std::println("Done.");
}

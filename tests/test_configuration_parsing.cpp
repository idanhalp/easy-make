#include "tests/tests.hpp"

#include <cassert>
#include <print>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/utils/utils.hpp"

static auto test_regular_configurations() -> void
{
    const auto project_2_path = tests::utils::get_path_to_resources_project(2);
    const auto configurations = parse_configurations(project_2_path);

    assert(configurations.size() == 2);

    const auto& default_configuration = configurations[0];
    assert(default_configuration.name == "default");
    assert(default_configuration.compiler == "g++");
    assert((default_configuration.warnings == std::vector<std::string>{"-Wall", "-Wextra"}));
    assert(default_configuration.optimization == "-O2");
    assert((default_configuration.source_files == std::vector<std::string>{"src/main.cpp", "src/utils.cpp"}));
    assert((default_configuration.source_directories == std::vector<std::string>{"src"}));
    assert((default_configuration.excluded_files == std::vector<std::string>{"src/legacy.cpp"}));
    assert((default_configuration.excluded_directories == std::vector<std::string>{"third_party"}));
    assert(default_configuration.output_name == "my_app");
    assert(default_configuration.output_path == "build");

    const auto& debug_configuration = configurations[1];
    assert(debug_configuration.name == "debug");
    assert(debug_configuration.optimization == "-O0");
    assert((debug_configuration.defines == std::vector<std::string>{"DDEBUG"}));
    assert((debug_configuration.include_directories == std::vector<std::string>{".", "source"}));
    assert(debug_configuration.output_name == "my_app_debug");
    assert(debug_configuration.output_path == "build");
}

static auto test_configuration_with_missing_fields() -> void
{
    const auto project_4_path = tests::utils::get_path_to_resources_project(4);
    const auto configurations = parse_configurations(project_4_path);

    assert(configurations.size() == 1);

    const auto& default_configuration = configurations[0];
    assert(default_configuration.name == "default");
    assert(!default_configuration.compiler.has_value());
    assert(!default_configuration.warnings.has_value());
    assert(!default_configuration.optimization.has_value());
    assert(!default_configuration.source_files.has_value());
    assert(!default_configuration.source_directories.has_value());
    assert(!default_configuration.excluded_files.has_value());
    assert(!default_configuration.excluded_directories.has_value());
    assert(!default_configuration.output_name.has_value());
    assert(!default_configuration.output_path.has_value());
}

auto tests::test_configuration_parsing() -> void
{
    std::println("Running `parse_configurations` tests.");

    test_regular_configurations();
    test_configuration_with_missing_fields();

    std::println("Done.");
}

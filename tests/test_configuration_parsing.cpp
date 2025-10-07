#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/utils/utils.hpp"

TEST_SUITE("configuration_parsing")
{
    TEST_CASE("Regular configurations are parsed correctly")
    {
        const auto project_2_path = tests::utils::get_path_to_resources_project(2);
        const auto configurations = parse_configurations(project_2_path);

        CHECK_EQ(configurations.size(), 2);

        const auto& default_configuration = configurations[0];
        CHECK_EQ(default_configuration.name, "default");
        CHECK_EQ(default_configuration.compiler, "g++");
        CHECK_EQ(default_configuration.warnings.value(), std::vector<std::string>{"-Wall", "-Wextra"});
        CHECK_EQ(default_configuration.optimization.value(), "-O2");
        CHECK_EQ(default_configuration.source_files.value(), std::vector<std::string>{"src/main.cpp", "src/utils.cpp"});
        CHECK_EQ(default_configuration.source_directories.value(), std::vector<std::string>{"src"});
        CHECK_EQ(default_configuration.excluded_files.value(), std::vector<std::string>{"src/legacy.cpp"});
        CHECK_EQ(default_configuration.excluded_directories.value(), std::vector<std::string>{"third_party"});
        CHECK_EQ(default_configuration.output_name.value(), "my_app");
        CHECK_EQ(default_configuration.output_path.value(), "build");

        const auto& debug_configuration = configurations[1];
        CHECK_EQ(debug_configuration.name, "debug");
        CHECK_EQ(debug_configuration.optimization.value(), "-O0");
        CHECK_EQ(debug_configuration.defines.value(), std::vector<std::string>{"DDEBUG"});
        CHECK_EQ(debug_configuration.include_directories.value(), std::vector<std::string>{".", "source"});
        CHECK_EQ(debug_configuration.output_name.value(), "my_app_debug");
        CHECK_EQ(debug_configuration.output_path.value(), "build");
    }

    TEST_CASE("Configuration with missing fields is handled correctly")
    {
        const auto project_4_path = tests::utils::get_path_to_resources_project(4);
        const auto configurations = parse_configurations(project_4_path);

        CHECK_EQ(configurations.size(), 1);

        const auto& default_configuration = configurations[0];
        CHECK_EQ(default_configuration.name, "default");
        CHECK_FALSE(default_configuration.compiler.has_value());
        CHECK_FALSE(default_configuration.warnings.has_value());
        CHECK_FALSE(default_configuration.optimization.has_value());
        CHECK_FALSE(default_configuration.source_files.has_value());
        CHECK_FALSE(default_configuration.source_directories.has_value());
        CHECK_FALSE(default_configuration.excluded_files.has_value());
        CHECK_FALSE(default_configuration.excluded_directories.has_value());
        CHECK_FALSE(default_configuration.output_name.has_value());
        CHECK_FALSE(default_configuration.output_path.has_value());
    }
}

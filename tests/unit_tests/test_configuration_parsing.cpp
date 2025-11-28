#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/configuration_parsing/configuration.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/parameters.hpp"
#include "tests/unit_tests/utils/utils.hpp"

TEST_SUITE("configuration_parsing" * doctest::test_suite(test_type::unit))
{
    TEST_CASE("Regular configurations are parsed correctly")
    {
        const auto project_2_path = tests::utils::get_path_to_resources_project(2);
        const auto configurations = parse_configurations(project_2_path);

        REQUIRE(configurations.has_value());
        CHECK_EQ(configurations->size(), 2);

        const auto& default_configuration = configurations->at(0);
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

        const auto& debug_configuration = configurations->at(1);
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

        REQUIRE(configurations.has_value());

        CHECK_EQ(configurations->size(), 1);

        const auto& default_configuration = configurations->at(0);
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

    TEST_CASE("Configuration file with invalid JSON is handled correctly")
    {
        const auto project_9_path = tests::utils::get_path_to_resources_project(9);
        const auto configurations = parse_configurations(project_9_path);

        REQUIRE(!configurations.has_value());
        CHECK(configurations.error().starts_with("Error: Invalid JSON in 'easy-make-configurations.json' - "));
    }

    TEST_CASE("Configuration file has an invalid outer field")
    {
        {
            const auto project_10_path = tests::utils::get_path_to_resources_project(10);
            const auto configurations  = parse_configurations(project_10_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(
                configurations.error(),
                "Error: Invalid JSON in 'easy-make-configurations.json' - 'someInvalidKey' is not a valid outer key.");
        }

        {
            const auto project_11_path = tests::utils::get_path_to_resources_project(11);
            const auto configurations  = parse_configurations(project_11_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - 'optimizatio' "
                     "is not a valid outer key. Did you mean 'optimization'?");
        }
    }

    TEST_CASE("Configuration file has invalid key inside 'sources' object")
    {
        {
            const auto project_12_path = tests::utils::get_path_to_resources_project(12);
            const auto configurations  = parse_configurations(project_12_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - "
                     "'sources.someInvalidKey' is not a valid key.");
        }

        {
            const auto project_13_path = tests::utils::get_path_to_resources_project(13);
            const auto configurations  = parse_configurations(project_13_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - 'sources.directori' is not a valid key. "
                     "Did you mean 'sources.directories'?");
        }
    }

    TEST_CASE("Configuration file has invalid key inside 'exclude' object")
    {
        {
            const auto project_14_path = tests::utils::get_path_to_resources_project(14);
            const auto configurations  = parse_configurations(project_14_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - "
                     "'exclude.someInvalidKey' is not a valid key.");
        }

        {
            const auto project_15_path = tests::utils::get_path_to_resources_project(15);
            const auto configurations  = parse_configurations(project_15_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - 'exclude.file' is not a valid key. "
                     "Did you mean 'exclude.files'?");
        }
    }

    TEST_CASE("Configuration file has invalid key inside 'output' object")
    {
        {
            const auto project_16_path = tests::utils::get_path_to_resources_project(16);
            const auto configurations  = parse_configurations(project_16_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - "
                     "'output.someInvalidKey' is not a valid key.");
        }

        {
            const auto project_17_path = tests::utils::get_path_to_resources_project(17);
            const auto configurations  = parse_configurations(project_17_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON in 'easy-make-configurations.json' - 'output.pate' is not a valid key. "
                     "Did you mean 'output.path'?");
        }
    }

    TEST_CASE("Source files contain a file that does not exist.")
    {
        const auto project_21_path = tests::utils::get_path_to_resources_project(21);
        const auto configurations  = parse_configurations(project_21_path);

        REQUIRE(!configurations.has_value());
        CHECK_EQ(configurations.error(), "Error: Configuration 'default' - source file 'f_3.cpp' does not exist.");
    }

    TEST_CASE("Source directories contain a directory that does not exist.")
    {
        const auto project_22_path = tests::utils::get_path_to_resources_project(22);
        const auto configurations  = parse_configurations(project_22_path);

        REQUIRE(!configurations.has_value());
        CHECK_EQ(configurations.error(), "Error: Configuration 'default' - source directory 'dir_2' does not exist.");
    }

    TEST_CASE("Excluded files contain a file that does not exist.")
    {
        const auto project_23_path = tests::utils::get_path_to_resources_project(23);
        const auto configurations  = parse_configurations(project_23_path);

        REQUIRE(!configurations.has_value());
        CHECK_EQ(configurations.error(), "Error: Configuration 'default' - excluded file 'f_3.cpp' does not exist.");
    }

    TEST_CASE("Excluded directories contain a directory that does not exist.")
    {
        const auto project_24_path = tests::utils::get_path_to_resources_project(24);
        const auto configurations  = parse_configurations(project_24_path);

        REQUIRE(!configurations.has_value());
        CHECK_EQ(configurations.error(), "Error: Configuration 'default' - excluded directory 'dir_2' does not exist.");
    }
}

#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/configuration_parsing/configuration.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/configuration_parsing/value_validation.hpp"
#include "source/parameters/parameters.hpp"
#include "tests/parameters.hpp"
#include "tests/unit_tests/utils/utils.hpp"

TEST_SUITE("configuration_parsing" * doctest::test_suite(test_type::unit))
{
    TEST_CASE("Regular configurations are parsed correctly")
    {
        const auto project_2_path = tests::utils::get_path_to_resources_project(2);
        const auto configurations = parse_configurations(project_2_path);

        REQUIRE(configurations.has_value());
        CHECK_EQ(configurations->size(), 3);

        const auto& default_configuration = configurations->at(0);
        CHECK_EQ(default_configuration.name, "default");
        CHECK_EQ(default_configuration.compiler, "g++");
        CHECK_EQ(default_configuration.warnings.value(), std::vector<std::string>{"-Wall", "-Wextra"});
        CHECK_EQ(default_configuration.optimization.value(), "2");
        CHECK_EQ(default_configuration.source_files.value(), std::vector<std::string>{"src/main.cpp", "src/utils.cpp"});
        CHECK_EQ(default_configuration.source_directories.value(), std::vector<std::string>{"src"});
        CHECK_EQ(default_configuration.excluded_files.value(), std::vector<std::string>{"src/legacy.cpp"});
        CHECK_EQ(default_configuration.excluded_directories.value(), std::vector<std::string>{"third_party"});
        CHECK_EQ(default_configuration.output_name.value(), "my_app");
        CHECK_EQ(default_configuration.output_path.value(), "build");

        const auto& debug_configuration = configurations->at(1);
        CHECK_EQ(debug_configuration.name, "debug");
        CHECK_EQ(debug_configuration.optimization.value(), "0");
        CHECK_EQ(debug_configuration.defines.value(), std::vector<std::string>{"DDEBUG"});
        CHECK_EQ(debug_configuration.include_directories.value(), std::vector<std::string>{".", "source"});
        CHECK_EQ(debug_configuration.output_name.value(), "my_app_debug");
        CHECK_EQ(debug_configuration.output_path.value(), "build");

        const auto& performance_configuration = configurations->at(2);
        CHECK_EQ(performance_configuration.compilation_flags, std::vector<std::string>{"-pg"});
        CHECK_EQ(performance_configuration.link_flags, std::vector<std::string>{"-pg"});
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

    TEST_CASE("Invalid outer field")
    {
        SUBCASE("similar valid key does not exists")
        {
            const auto project_10_path = tests::utils::get_path_to_resources_project(10);
            const auto configurations  = parse_configurations(project_10_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'someInvalidKey'.");
        }

        SUBCASE("similar valid key exists")
        {
            const auto project_11_path = tests::utils::get_path_to_resources_project(11);
            const auto configurations  = parse_configurations(project_11_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'optimizatio'. "
                     "Did you mean 'optimization'?");
        }
    }

    TEST_CASE("Invalid key inside 'sources' object")
    {
        SUBCASE("similar valid key does not exists")
        {
            const auto project_12_path = tests::utils::get_path_to_resources_project(12);
            const auto configurations  = parse_configurations(project_12_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'sources.someInvalidKey'.");
        }

        SUBCASE("similar valid key exists")
        {
            const auto project_13_path = tests::utils::get_path_to_resources_project(13);
            const auto configurations  = parse_configurations(project_13_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'sources.directori'. "
                     "Did you mean 'sources.directories'?");
        }
    }

    TEST_CASE("Invalid key inside 'exclude' object")
    {
        SUBCASE("similar valid key does not exists")
        {
            const auto project_14_path = tests::utils::get_path_to_resources_project(14);
            const auto configurations  = parse_configurations(project_14_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'exclude.someInvalidKey'.");
        }

        SUBCASE("similar valid key exists")
        {
            const auto project_15_path = tests::utils::get_path_to_resources_project(15);
            const auto configurations  = parse_configurations(project_15_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'exclude.file'. "
                     "Did you mean 'exclude.files'?");
        }
    }

    TEST_CASE("Invalid key inside 'output' object")
    {
        SUBCASE("similar valid key does not exists")
        {
            const auto project_16_path = tests::utils::get_path_to_resources_project(16);
            const auto configurations  = parse_configurations(project_16_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'output.someInvalidKey'.");
        }

        SUBCASE("similar valid key exists")
        {
            const auto project_17_path = tests::utils::get_path_to_resources_project(17);
            const auto configurations  = parse_configurations(project_17_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the 1st configuration contains an unknown key 'output.pate'. "
                     "Did you mean 'output.path'?");
        }
    }

    TEST_CASE("JSON is not an array")
    {
        const auto project_26_path = tests::utils::get_path_to_resources_project(26);
        const auto configurations  = parse_configurations(project_26_path);

        REQUIRE(!configurations.has_value());
        CHECK_EQ(configurations.error(), "Error: Invalid JSON - must be an array of objects.");
    }

    TEST_CASE("Field has an invalid type")
    {
        SUBCASE("standard")
        {
            const auto project_27_path = tests::utils::get_path_to_resources_project(27);
            const auto configurations  = parse_configurations(project_27_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the value of 'standard' in the 2nd configuration must be 'string', "
                     "but got 'number'.");
        }

        SUBCASE("warnings - Incorrect type")
        {
            const auto project_28_path = tests::utils::get_path_to_resources_project(28);
            const auto configurations  = parse_configurations(project_28_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the value of 'warnings' in the 1st configuration must be 'array', "
                     "but got 'string'.");
        }

        SUBCASE("warnings - Incorrect element type")
        {
            const auto project_29_path = tests::utils::get_path_to_resources_project(29);
            const auto configurations  = parse_configurations(project_29_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(
                configurations.error(),
                "Error: Invalid JSON - the value of 'warnings' at index 2 in the 1st configuration must be 'string', "
                "but got 'number'.");
        }

        SUBCASE("output")
        {
            const auto project_30_path = tests::utils::get_path_to_resources_project(30);
            const auto configurations  = parse_configurations(project_30_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Invalid JSON - the value of 'output' in the 3rd configuration must be 'object', "
                     "but got 'string'.");
        }
    }

    TEST_CASE("Validate configuration names")
    {
        SUBCASE("valid names")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-1";
            configurations[1].name = "name-2";
            configurations[2].name = "name-3";

            const auto names_error = validate_configuration_values(configurations, "");
            CHECK_FALSE(names_error.has_value());
        }

        SUBCASE("configuration without a name")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-1";
            configurations[2].name = "name-3";

            const auto names_error = validate_configuration_values(configurations, "");
            REQUIRE(names_error.has_value());
            CHECK_EQ(*names_error, "Error: The 2nd configuration does not have a name.");
        }

        SUBCASE("duplicate name")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-1";
            configurations[1].name = "name-1";
            configurations[2].name = "name-3";

            const auto names_error = validate_configuration_values(configurations, "");
            REQUIRE(names_error.has_value());
            CHECK_EQ(*names_error, "Error: Both the 1st and 2nd configurations have 'name-1' as name.");
        }
    }

    TEST_CASE("Check parents validity")
    {
        SUBCASE("configuration with itself as parent")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[1].parent = "name-0";
            configurations[2].parent = "name-2";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'name-2' has itself as a parent.");
        }

        SUBCASE("nonexistent parent")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[0].parent = "non-existent";
            configurations[1].parent = "name-0";
            configurations[2].parent = "name-0";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'name-0' has a non-existent parent configuration 'non-existent'.");
        }

        SUBCASE("parent name is almost valid")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[1].parent = "nome-0";
            configurations[2].parent = "name-0";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error,
                     "Error: Configuration 'name-1' has a non-existent parent configuration 'nome-0'. "
                     "Did you mean 'name-0'?");
        }

        SUBCASE("circular parent dependency")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[0].parent = "name-1";
            configurations[1].parent = "name-2";
            configurations[2].parent = "name-0";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            REQUIRE(error->starts_with("Error:"));
            REQUIRE(error->contains("name-0 -> name-1 -> name-2"));
        }

        SUBCASE("valid parents")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[1].parent = "name-0";
            configurations[2].parent = "name-1";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE_FALSE(error.has_value());
        }
    }

    TEST_CASE("Check argument validity")
    {
        SUBCASE("valid compiler - g++")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].compiler = "g++";

            CHECK_FALSE(validate_configuration_values(configurations, "").has_value());
        }

        SUBCASE("valid compiler - clang++")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].compiler = "clang++";

            CHECK_FALSE(validate_configuration_values(configurations, "").has_value());
        }

        SUBCASE("valid compiler - cl")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].compiler = "cl";

            const auto error = validate_configuration_values(configurations, "");

            if (params::ENABLE_MSVC)
            {
                CHECK_FALSE(error.has_value());
            }
            else
            {
                REQUIRE(error.has_value());
                CHECK(error->starts_with("Error: Configuration 'config' has an unknown compiler 'cl'."));
            }
        }

        SUBCASE("invalid compiler")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].compiler = "rustc";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'config' has an unknown compiler 'rustc'.");
        }

        SUBCASE("almost valid compiler")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].compiler = "clang+";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'config' has an unknown compiler 'clang+'. Did you mean 'clang++'?");
        }

        SUBCASE("valid standard")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].standard = "17";

            CHECK_FALSE(validate_configuration_values(configurations, "").has_value());
        }

        SUBCASE("invalid standard")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].standard = "27";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'config' has an unknown standard '27'.");
        }

        SUBCASE("valid warnings")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].warnings = {"-Wall", "-Wextra"};

            CHECK_FALSE(validate_configuration_values(configurations, "").has_value());
        }

        SUBCASE("invalid warning")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name     = "config";
            configurations[0].warnings = {"-Wall", "-pedantic", "-Wextra"};

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'config' has an invalid warning '-pedantic' at index 1.");
        }

        SUBCASE("valid optimization")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name         = "config";
            configurations[0].optimization = "2";

            CHECK_FALSE(validate_configuration_values(configurations, "").has_value());
        }

        SUBCASE("invalid optimization")
        {
            std::vector<Configuration> configurations(1);
            configurations[0].name         = "config";
            configurations[0].optimization = "7";

            const auto error = validate_configuration_values(configurations, "");
            REQUIRE(error.has_value());
            CHECK_EQ(*error, "Error: Configuration 'config' has an unknown optimization '7'.");
        }

        SUBCASE("invalid source files")
        {
            const auto project_21_path = tests::utils::get_path_to_resources_project(21);
            const auto configurations  = parse_configurations(project_21_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Configuration 'default' has a non-existent source file 'f_3.cpp'.");
        }

        SUBCASE("invalid source directories")
        {
            const auto project_22_path = tests::utils::get_path_to_resources_project(22);
            const auto configurations  = parse_configurations(project_22_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Configuration 'default' has a non-existent source directory 'dir_2'.");
        }

        SUBCASE("invalid excluded files")
        {
            const auto project_23_path = tests::utils::get_path_to_resources_project(23);
            const auto configurations  = parse_configurations(project_23_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Configuration 'default' has a non-existent excluded file 'f_3.cpp'.");
        }

        SUBCASE("invalid excluded directories")
        {
            const auto project_24_path = tests::utils::get_path_to_resources_project(24);
            const auto configurations  = parse_configurations(project_24_path);

            REQUIRE(!configurations.has_value());
            CHECK_EQ(configurations.error(),
                     "Error: Configuration 'default' has a non-existent excluded directory 'dir_2'.");
        }
    }
}

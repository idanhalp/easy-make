#include <algorithm>
#include <unordered_map>
#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/build/build.hpp"
#include "source/commands/build/compilation/compilation.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/parameters/parameters.hpp"
#include "tests/parameters.hpp"
#include "tests/unit_tests/utils/utils.hpp"

TEST_SUITE("commands::build" * doctest::test_suite(test_type::unit))
{
    TEST_CASE("Check names validity")
    {
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-1";
            configurations[2].name = "name-3";

            const auto names_error = check_names_validity(configurations);
            REQUIRE(names_error.has_value());
            CHECK_EQ(*names_error, "Error: The 2nd configuration does not have a name.");
        }

        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-1";
            configurations[1].name = "name-1";
            configurations[2].name = "name-3";

            const auto names_error = check_names_validity(configurations);
            REQUIRE(names_error.has_value());
            CHECK_EQ(*names_error, "Error: Both the 1st and 2nd configurations have 'name-1' as name.");
        }

        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-1";
            configurations[1].name = "name-2";
            configurations[2].name = "name-3";

            const auto names_error = check_names_validity(configurations);
            CHECK_FALSE(names_error.has_value());
        }
    }

    TEST_CASE("Check parents validity")
    {
        SUBCASE("Configuration with itself as parent")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[1].parent = "name-0";
            configurations[2].parent = "name-2";

            std::unordered_map<std::string, Configuration> name_to_configuration;
            for (const auto& configuration : configurations)
                name_to_configuration[*configuration.name] = configuration;

            const auto parent_error = check_parents_validity(name_to_configuration);
            REQUIRE(parent_error.has_value());
            CHECK_EQ(*parent_error, "Error: Configuration 'name-2' has itself as a parent.");
        }

        SUBCASE("Configuration with nonexistent parent")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[0].parent = "non-existent";
            configurations[1].parent = "name-0";
            configurations[2].parent = "name-0";

            std::unordered_map<std::string, Configuration> name_to_configuration;
            for (const auto& configuration : configurations)
                name_to_configuration[*configuration.name] = configuration;

            const auto parent_error = check_parents_validity(name_to_configuration);
            REQUIRE(parent_error.has_value());
            CHECK_EQ(*parent_error,
                     "Error: Configuration 'name-0' has a non-existent configuration as its parent ('non-existent').");
        }

        SUBCASE("Circular parent dependency")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[0].parent = "name-1";
            configurations[1].parent = "name-2";
            configurations[2].parent = "name-0";

            std::unordered_map<std::string, Configuration> name_to_configuration;
            for (const auto& configuration : configurations)
                name_to_configuration[*configuration.name] = configuration;

            const auto parent_error = check_parents_validity(name_to_configuration);
            REQUIRE(parent_error.has_value());
            REQUIRE(parent_error->starts_with("Error:"));
            REQUIRE(parent_error->contains("name-0 -> name-1 -> name-2"));
        }

        SUBCASE("Valid parents")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "name-0";
            configurations[1].name = "name-1";
            configurations[2].name = "name-2";

            configurations[1].parent = "name-0";
            configurations[2].parent = "name-1";

            std::unordered_map<std::string, Configuration> name_to_configuration;
            for (const auto& configuration : configurations)
                name_to_configuration[*configuration.name] = configuration;

            const auto parent_error = check_parents_validity(name_to_configuration);
            REQUIRE_FALSE(parent_error.has_value());
        }
    }

    TEST_CASE("Get actual configuration")
    {
        {
            Configuration configuration;
            configuration.output_name = "output";
            configuration.output_path = "build";

            Configuration configuration_1 = configuration;
            Configuration configuration_2 = configuration;
            Configuration configuration_3 = configuration;

            configuration_1.name = "aaaa";
            configuration_2.name = "bbbb";
            configuration_3.name = "cccc";

            const auto result = get_actual_configuration("dddd", {configuration_1, configuration_2, configuration_3});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "'easy-make-configurations.json' does not contain a configuration named 'dddd'.");
        }

        {
            Configuration configuration;
            configuration.output_name = "output";
            configuration.output_path = "build";

            Configuration configuration_1 = configuration;
            Configuration configuration_2 = configuration;
            Configuration configuration_3 = configuration;

            configuration_1.name = "aaaa";
            configuration_2.name = "bbbb";
            configuration_3.name = "cccc";

            const auto result = get_actual_configuration("ccdc", {configuration_1, configuration_2, configuration_3});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(
                result.error(),
                "'easy-make-configurations.json' does not contain a configuration named 'ccdc'. Did you mean 'cccc'?");
        }

        {
            Configuration configuration;
            configuration.name        = "test-configuration";
            configuration.output_name = "output";
            configuration.output_path = "build";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: Could not resolve 'compiler' for 'test-configuration'.");
        }

        {
            Configuration configuration;
            configuration.name        = "test-configuration";
            configuration.compiler    = "g++";
            configuration.output_path = "build";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: Could not resolve 'output.name' for 'test-configuration'.");
        }
    }

    TEST_CASE("Actual configuration with overridden fields")
    {
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

            const auto actual_configuration =
                get_actual_configuration("test", {test_configuration, default_configuration});
            REQUIRE(actual_configuration.has_value());
            CHECK_EQ(actual_configuration->name, "test");
            CHECK_EQ(actual_configuration->compiler, "clang++");
            CHECK_EQ(actual_configuration->output_name, "test-output");
            CHECK_EQ(actual_configuration->output_path, "build");
            CHECK_EQ(actual_configuration->optimization, "3");
        }

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
            REQUIRE(actual_configuration.has_value());
            CHECK_EQ(actual_configuration->name, configuration_2.name);
            CHECK_EQ(actual_configuration->compiler, configuration_2.compiler);
            CHECK_EQ(actual_configuration->defines, configuration_1.defines);
            CHECK_EQ(actual_configuration->output_name, configuration_0.output_name);
        }
    }

    TEST_CASE("Actual configuration invalid scenarios")
    {
        {
            Configuration configuration;
            configuration.compiler    = "g++";
            configuration.name        = "";
            configuration.output_name = "test.exe";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: empty configuration name.");
        }

        {
            Configuration configuration;
            configuration.compiler    = "rustc";
            configuration.name        = "test";
            configuration.output_name = "test.exe";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: Configuration 'test' - unknown compiler 'rustc'.");
        }

        {
            Configuration configuration;
            configuration.compiler    = "g++";
            configuration.name        = "test";
            configuration.output_name = "test.exe";
            configuration.standard    = "27";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: Configuration 'test' - unknown standard '27'.");
        }
    }

    TEST_CASE("Actual configuration invalid warnings and optimizations")
    {
        if (params::ENABLE_MSVC)
        {
            Configuration configuration;
            configuration.compiler    = "cl";
            configuration.name        = "test";
            configuration.output_name = "test.exe";
            configuration.warnings    = {"/W3", "/Wall", "/wd4996", "invalid-warning", "/WX"};

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(),
                     "Error: Configuration 'test' - warning 'invalid-warning' is invalid when compiling with 'cl'.");
        }

        {
            Configuration configuration;
            configuration.compiler    = "g++";
            configuration.name        = "test";
            configuration.output_name = "test.exe";
            configuration.warnings    = {"-Wall", "-Wextra", "-pedantic", "invalid-warning", "-pedantic-errors"};

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(),
                     "Error: Configuration 'test' - warning 'invalid-warning' is invalid when compiling with 'g++'.");
        }

        {
            Configuration configuration;
            configuration.compiler     = "g++";
            configuration.name         = "test";
            configuration.output_name  = "test.exe";
            configuration.optimization = "4";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: Configuration 'test' - unknown optimization '4'.");
        }

        if (params::ENABLE_MSVC)
        {
            Configuration configuration;
            configuration.compiler     = "g++";
            configuration.name         = "test";
            configuration.output_name  = "test.exe";
            configuration.optimization = "d";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(),
                     "Error: Configuration 'test' - optimization 'd' does not match specified compiler "
                     "'g++'. Did you mean to compile with 'cl' instead?");
        }

        if (params::ENABLE_MSVC)
        {
            Configuration configuration;
            configuration.compiler     = "cl";
            configuration.name         = "test";
            configuration.output_name  = "test.exe";
            configuration.optimization = "0";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(),
                     "Error: Configuration 'test' - optimization '0' does not match specified compiler "
                     "'cl'. Did you mean to compile with 'g++' or 'clang++' instead?");
        }
    }

    TEST_CASE("Get source files")
    {
        const auto project_5_path = tests::utils::get_path_to_resources_project(5);
        const auto configurations = parse_configurations(project_5_path);

        REQUIRE(configurations.has_value());

        const auto actual_configuration = get_actual_configuration("default", *configurations);

        REQUIRE(actual_configuration.has_value());

        const auto files_to_compile = get_code_files(*actual_configuration, project_5_path);
        CHECK_EQ(files_to_compile.size(), 3);
        CHECK(std::ranges::contains(files_to_compile, "main.cpp"));
        CHECK(std::ranges::contains(files_to_compile, "source/f_1.cpp"));
        CHECK(std::ranges::contains(files_to_compile, "source/dir_2/f_4.cxx"));
    }

    TEST_CASE("Create compilation flags string")
    {
        SUBCASE("Simple functionality check (clang++/g++)")
        {
            Configuration configuration;
            configuration.name                = "test";
            configuration.compiler            = "clang++";
            configuration.standard            = "20";
            configuration.warnings            = {"-Wall", "-Werror"};
            configuration.compilation_flags   = {"-pg"};
            configuration.optimization        = "2";
            configuration.defines             = {"DEBUG", "VERSION=12"};
            configuration.include_directories = {".", "source", "test"};

            CHECK_EQ(create_compilation_flags_string(configuration),
                     "-std=c++20 -Wall -Werror -pg -O2 -DDEBUG -DVERSION=12 -I. -Isource -Itest");
        }

        SUBCASE("Simple functionality check (cl)")
        {
            if (params::ENABLE_MSVC)
            {
                Configuration configuration;
                configuration.name                = "test";
                configuration.compiler            = "cl";
                configuration.standard            = "98";
                configuration.optimization        = "d";
                configuration.defines             = {"DEBUG", "VERSION=12"};
                configuration.include_directories = {".", "source", "test"};

                CHECK_EQ(create_compilation_flags_string(configuration),
                         "-std=c++98 /Od -DDEBUG -DVERSION=12 -I. -Isource -Itest");
            }
        }

        SUBCASE("No flags")
        {
            Configuration configuration;
            configuration.name     = "test";
            configuration.compiler = "cl";

            CHECK_EQ(create_compilation_flags_string(configuration), "");
        }
    }
}

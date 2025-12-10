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
            CHECK_EQ(result.error(), "Error: Could not resolve compiler for configuration 'test-configuration'.");
        }

        {
            Configuration configuration;
            configuration.name        = "test-configuration";
            configuration.compiler    = "g++";
            configuration.output_path = "build";

            const auto result = get_actual_configuration(*configuration.name, {configuration});
            REQUIRE_FALSE(result.has_value());
            CHECK_EQ(result.error(), "Error: Could not resolve output.name for configuration 'test-configuration'.");
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

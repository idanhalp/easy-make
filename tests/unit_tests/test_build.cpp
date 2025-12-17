#include <algorithm>
#include <unordered_map>
#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/build/build.hpp"
#include "source/commands/build/compilation/compilation.hpp"
#include "source/commands/build/configuration_resolution.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/parameters/parameters.hpp"
#include "tests/parameters.hpp"
#include "tests/unit_tests/utils/utils.hpp"

TEST_SUITE("commands::build" * doctest::test_suite(test_type::unit))
{
    TEST_CASE("get_resolved_configurations")
    {
        SUBCASE("no parents")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name        = "config-0";
            configurations[0].compiler    = "g++";
            configurations[0].output_name = "output-0.exe";

            configurations[1].name        = "config-1";
            configurations[1].compiler    = "clang++";
            configurations[1].output_name = "output-1.exe";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(resolved.size(), 2);
            CHECK_EQ(*resolved[0].compiler, "g++");
            CHECK_EQ(*resolved[1].compiler, "clang++");
        }

        SUBCASE("single parent inheritance")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name        = "base";
            configurations[0].compiler    = "g++";
            configurations[0].output_name = "base.exe";

            configurations[1].name        = "child";
            configurations[1].parent      = "base";
            configurations[1].output_name = "child.exe";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(*resolved[1].compiler, "g++");          // inherited
            CHECK_EQ(*resolved[1].output_name, "child.exe"); // overridden
        }

        SUBCASE("child overrides parent field")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name     = "base";
            configurations[0].compiler = "g++";

            configurations[1].name     = "child";
            configurations[1].parent   = "base";
            configurations[1].compiler = "clang++";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(*resolved[1].compiler, "clang++");
        }

        SUBCASE("multi-level inheritance")
        {
            std::vector<Configuration> configurations(3);

            configurations[0].name     = "base";
            configurations[0].compiler = "g++";
            configurations[0].standard = "c++20";

            configurations[1].name         = "mid";
            configurations[1].parent       = "base";
            configurations[1].optimization = "O2";

            configurations[2].name   = "leaf";
            configurations[2].parent = "mid";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(*resolved[2].compiler, "g++");
            CHECK_EQ(*resolved[2].standard, "c++20");
            CHECK_EQ(*resolved[2].optimization, "O2");
        }

        SUBCASE("parent defined after child")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name   = "child";
            configurations[0].parent = "parent";

            configurations[1].name     = "parent";
            configurations[1].compiler = "clang++";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(*resolved[0].compiler, "clang++");
        }

        SUBCASE("vector fields inherited")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name     = "base";
            configurations[0].warnings = std::vector<std::string>{"-Wall", "-Wextra"};

            configurations[1].name   = "child";
            configurations[1].parent = "base";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            REQUIRE(resolved[1].warnings.has_value());
            CHECK_EQ(resolved[1].warnings->size(), 2);
            CHECK_EQ((*resolved[1].warnings)[0], "-Wall");
        }

        SUBCASE("vector fields are replaced, not merged")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name    = "base";
            configurations[0].defines = std::vector<std::string>{"A", "B"};

            configurations[1].name    = "child";
            configurations[1].parent  = "base";
            configurations[1].defines = std::vector<std::string>{"C"};

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(resolved[1].defines->size(), 1);
            CHECK_EQ((*resolved[1].defines)[0], "C");
        }

        SUBCASE("unrelated configurations remain independent")
        {
            std::vector<Configuration> configurations(2);

            configurations[0].name     = "a";
            configurations[0].compiler = "g++";

            configurations[1].name     = "b";
            configurations[1].compiler = "clang++";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(*resolved[0].compiler, "g++");
            CHECK_EQ(*resolved[1].compiler, "clang++");
        }

        SUBCASE("output order matches input order")
        {
            std::vector<Configuration> configurations(3);

            configurations[0].name = "first";
            configurations[1].name = "second";
            configurations[2].name = "third";

            const auto resolved = get_resolved_configurations(configurations, ConfigurationType::ALL);

            CHECK_EQ(*resolved[0].name, "first");
            CHECK_EQ(*resolved[1].name, "second");
            CHECK_EQ(*resolved[2].name, "third");
        }

        SUBCASE("return only complete/incomplete configurations")
        {
            std::vector<Configuration> configurations(3);
            configurations[0].name = "config-0";
            configurations[1].name = "config-1";
            configurations[2].name = "config-2";

            configurations[0].compiler    = "g++";
            configurations[0].output_name = "output.exe";

            configurations[2].parent = configurations[0].name;

            const auto complete_configs = get_resolved_configurations(configurations, ConfigurationType::COMPLETE);
            CHECK_EQ(complete_configs.size(), 2);
            CHECK_EQ(complete_configs[0].name, "config-0");
            CHECK_EQ(complete_configs[1].name, "config-2");

            const auto incomplete_configs = get_resolved_configurations(configurations, ConfigurationType::INCOMPLETE);
            CHECK_EQ(incomplete_configs.size(), 1);
            CHECK_EQ(incomplete_configs[0].name, "config-1");
        }
    }

    TEST_CASE("Get source files")
    {
        const auto project_5_path = tests::utils::get_path_to_resources_project(5);
        const auto configurations = parse_configurations(project_5_path);

        REQUIRE(configurations.has_value());

        const auto actual_configuration = get_resolved_configuration(*configurations, "default");
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

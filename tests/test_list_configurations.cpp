#include <string>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/list_configurations/list_configurations.hpp"
#include "tests/utils/temporary_file_creator.hpp"

using namespace std::literals;

TEST_SUITE("'list-configs' command")
{
    TEST_CASE_FIXTURE(TemporaryFileCreator, "Valid case without arguments")
    {
        ListConfigurationsCommandInfo info{};

        std::vector<Configuration> configurations(4);

        configurations[0].name        = "config-0";
        configurations[0].compiler    = "g++";
        configurations[0].output_name = "build";

        configurations[1].name        = "config-1";
        configurations[1].compiler    = "g++";
        configurations[1].output_name = "build";

        configurations[2].name        = "config-2";
        configurations[2].compiler    = "g++";
        configurations[2].output_name = "build";

        configurations[3].name = "config-3";
        // configurations[3].compiler = "g++";
        configurations[3].output_name = "build";

        const auto expected = "There are 4 configurations in the 'easy-make-configurations.json' file:\n"
                              "1 config-0\n"
                              "2 config-1\n"
                              "3 config-2\n"
                              "4 config-3\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "Valid case without arguments - single configuration")
    {
        ListConfigurationsCommandInfo info{};

        std::vector<Configuration> configurations(1);

        configurations[0].name        = "config-0";
        configurations[0].compiler    = "g++";
        configurations[0].output_name = "build";

        const auto expected = "There is 1 configuration in the 'easy-make-configurations.json' file:\n"
                              "1 config-0\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "Valid case without arguments - no configurations")
    {
        ListConfigurationsCommandInfo info{};

        const auto expected = "There are no configurations in the 'easy-make-configurations.json' file.\n"s;

        REQUIRE_EQ(commands::list_configurations(info, {}, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--count flag #1")
    {
        ListConfigurationsCommandInfo info{};
        info.count = true;

        std::vector<Configuration> configurations(4);

        configurations[0].name        = "config-0";
        configurations[0].compiler    = "g++";
        configurations[0].output_name = "build";

        configurations[1].name        = "config-1";
        configurations[1].compiler    = "g++";
        configurations[1].output_name = "build";

        configurations[2].name        = "config-2";
        configurations[2].compiler    = "g++";
        configurations[2].output_name = "build";

        configurations[3].name = "config-3";
        // configurations[3].compiler = "g++";
        configurations[3].output_name = "build";

        const auto expected = "4\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--count flag #2 (no configurations)")
    {
        ListConfigurationsCommandInfo info{};
        info.count = true;

        const auto expected = "0\n"s;

        REQUIRE_EQ(commands::list_configurations(info, {}, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--porcelain flag")
    {
        ListConfigurationsCommandInfo info{};
        info.porcelain_output = true;

        std::vector<Configuration> configurations(3);
        configurations[0].name = "b";
        configurations[1].name = "a";
        configurations[2].name = "c";

        const auto expected = "b\n"
                              "a\n"
                              "c\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--sorted flag")
    {
        ListConfigurationsCommandInfo info{};
        info.sorted_output = true;

        std::vector<Configuration> configurations(3);
        configurations[0].name = "zeta";
        configurations[1].name = "alpha";
        configurations[2].name = "beta";

        const auto expected = "There are 3 configurations in the 'easy-make-configurations.json' file:\n"
                              "1 alpha\n"
                              "2 beta\n"
                              "3 zeta\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--sorted and --porcelain flags together")
    {
        ListConfigurationsCommandInfo info{};
        info.sorted_output    = true;
        info.porcelain_output = true;

        std::vector<Configuration> configurations(3);
        configurations[0].name = "z";
        configurations[1].name = "a";
        configurations[2].name = "m";

        const auto expected = "a\n"
                              "m\n"
                              "z\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--complete-only flag")
    {
        ListConfigurationsCommandInfo info{};
        info.complete_configurations_only = true;

        std::vector<Configuration> configurations(4);

        configurations[0].name        = "config-0";
        configurations[0].compiler    = "g++";
        configurations[0].output_name = "build";

        // No `output_name`.
        configurations[1].name     = "config-1";
        configurations[1].compiler = "g++";

        configurations[2].name        = "config-2";
        configurations[2].compiler    = "g++";
        configurations[2].output_name = "build";

        // No `compiler`.
        configurations[3].name        = "config-3";
        configurations[3].output_name = "build";

        const auto expected = "There are 2 complete configurations in the 'easy-make-configurations.json' file:\n"
                              "1 config-0\n"
                              "2 config-2\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "--incomplete-only flag")
    {
        ListConfigurationsCommandInfo info{};
        info.incomplete_configurations_only = true;

        std::vector<Configuration> configurations(4);

        configurations[0].name        = "config-0";
        configurations[0].compiler    = "g++";
        configurations[0].output_name = "build";

        // No `output_name`.
        configurations[1].name     = "config-1";
        configurations[1].compiler = "g++";

        configurations[2].name        = "config-2";
        configurations[2].compiler    = "g++";
        configurations[2].output_name = "build";

        // No `compiler`.
        configurations[3].name        = "config-3";
        configurations[3].output_name = "build";

        const auto expected = "There are 2 incomplete configurations in the 'easy-make-configurations.json' file:\n"
                              "1 config-1\n"
                              "2 config-3\n"s;

        REQUIRE_EQ(commands::list_configurations(info, configurations, get_ofstream()), EXIT_SUCCESS);
        REQUIRE_EQ(get_content(), expected);
    }
}

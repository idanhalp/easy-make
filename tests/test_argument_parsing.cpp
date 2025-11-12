#include <variant>
#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/argument_parsing/argument_parsing.hpp"
#include "source/argument_parsing/command_info.hpp"

TEST_SUITE("argument_parsing")
{
    TEST_CASE("'build' command")
    {
        SUBCASE("Valid case")
        {
            const std::vector arguments = {"./easy-make", "build", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<BuildCommandInfo>(*command_info));

            const auto& build_command_info = std::get<BuildCommandInfo>(*command_info);
            CHECK_EQ(build_command_info.configuration_name, "config-name");
        }

        SUBCASE("Missing configuration name")
        {
            const std::vector arguments = {"./easy-make", "build"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Must specify a configuration name when using 'build' command.");
        }

        SUBCASE("Multiple configuration names")
        {
            const std::vector arguments = {"./easy-make", "build", "config-name-1", "config-name-2"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Command 'build' requires one configuration name, "
                     "instead got both 'config-name-1' and 'config-name-2'.");
        }

        SUBCASE("Invalid flag")
        {
            const std::vector arguments = {"./easy-make", "build", "--fast"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown flag '--fast' provided to command 'build'.");
        }
    }

    TEST_CASE("'clean' command")
    {
        SUBCASE("Valid case without flags")
        {
            const std::vector arguments = {"./easy-make", "clean", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<CleanCommandInfo>(*command_info));

            const auto& clean_command_info = std::get<CleanCommandInfo>(*command_info);
            CHECK_EQ(clean_command_info.configuration_name, "config-name");
            CHECK_FALSE(clean_command_info.is_quiet);
        }

        SUBCASE("Valid case with flags")
        {
            const std::vector arguments = {"./easy-make", "clean", "config-name", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<CleanCommandInfo>(*command_info));

            const auto& clean_command_info = std::get<CleanCommandInfo>(*command_info);
            CHECK_EQ(clean_command_info.configuration_name, "config-name");
            CHECK(clean_command_info.is_quiet);
        }

        SUBCASE("Missing configuration name")
        {
            const std::vector arguments = {"./easy-make", "clean"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Must specify a configuration name when using 'clean' command.");
        }

        SUBCASE("Multiple configuration names")
        {
            const std::vector arguments = {"./easy-make", "clean", "config-name-1", "config-name-2"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Command 'clean' requires one configuration name, "
                     "instead got both 'config-name-1' and 'config-name-2'.");
        }

        SUBCASE("Invalid flag")
        {
            const std::vector arguments = {"./easy-make", "clean", "--quiet", "--fast"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown flag '--fast' provided to command 'clean'.");
        }

        SUBCASE("Invalid flag similar to a valid one")
        {
            const std::vector arguments = {"./easy-make", "clean", "--quie"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Unknown flag '--quie' provided to command 'clean'. "
                     "Did you mean '--quiet'?");
        }

        SUBCASE("Duplicate flag")
        {
            const std::vector arguments = {"./easy-make", "clean", "config-name", "--quiet", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Flag '--quiet' was provided to command 'clean' more than once.");
        }
    }

    TEST_CASE("'clean-all' command")
    {
        SUBCASE("Valid case without flags")
        {
            const std::vector arguments = {"./easy-make", "clean-all"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<CleanAllCommandInfo>(*command_info));

            const auto& clean_all_command_info = std::get<CleanAllCommandInfo>(*command_info);
            CHECK_FALSE(clean_all_command_info.is_quiet);
        }

        SUBCASE("Valid case with flags")
        {
            const std::vector arguments = {"./easy-make", "clean-all", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<CleanAllCommandInfo>(*command_info));

            const auto& clean_all_command_info = std::get<CleanAllCommandInfo>(*command_info);
            CHECK(clean_all_command_info.is_quiet);
        }

        SUBCASE("Invalid flag")
        {
            const std::vector arguments = {"./easy-make", "clean-all", "--quiet", "--fast"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown flag '--fast' provided to command 'clean-all'.");
        }

        SUBCASE("Invalid flag similar to a valid one")
        {
            const std::vector arguments = {"./easy-make", "clean-all", "--quie"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Unknown flag '--quie' provided to command 'clean-all'. "
                     "Did you mean '--quiet'?");
        }

        SUBCASE("Duplicate flag")
        {
            const std::vector arguments = {"./easy-make", "clean-all", "--quiet", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Flag '--quiet' was provided to command 'clean-all' more than once.");
        }

        SUBCASE("Non flag argument")
        {
            const std::vector arguments = {"./easy-make", "clean-all", "config-name", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown argument 'config-name' provided to command 'clean-all'.");
        }
    }

    TEST_CASE("'list-configs' command")
    {
        SUBCASE("Valid case without flags")
        {
            const std::vector arguments = {"./easy-make", "list-configs"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListConfigurationsCommandInfo>(*command_info));

            const auto& list_configs_command_info = std::get<ListConfigurationsCommandInfo>(*command_info);
            CHECK_FALSE(list_configs_command_info.complete_configurations_only);
            CHECK_FALSE(list_configs_command_info.count);
            CHECK_FALSE(list_configs_command_info.incomplete_configurations_only);
            CHECK_FALSE(list_configs_command_info.porcelain_output);
            CHECK_FALSE(list_configs_command_info.sorted_output);
        }

        SUBCASE("Valid case with flags #1")
        {
            const std::vector arguments = {"./easy-make", "list-configs", "--sorted", "--porcelain"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListConfigurationsCommandInfo>(*command_info));

            const auto& list_configs_command_info = std::get<ListConfigurationsCommandInfo>(*command_info);
            CHECK_FALSE(list_configs_command_info.complete_configurations_only);
            CHECK_FALSE(list_configs_command_info.count);
            CHECK_FALSE(list_configs_command_info.incomplete_configurations_only);
            CHECK(list_configs_command_info.porcelain_output);
            CHECK(list_configs_command_info.sorted_output);
        }

        SUBCASE("Valid case with flags #2")
        {
            const std::vector arguments = {"./easy-make", "list-configs", "--complete-only", "--porcelain"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListConfigurationsCommandInfo>(*command_info));

            const auto& list_configs_command_info = std::get<ListConfigurationsCommandInfo>(*command_info);
            CHECK(list_configs_command_info.complete_configurations_only);
            CHECK_FALSE(list_configs_command_info.count);
            CHECK_FALSE(list_configs_command_info.incomplete_configurations_only);
            CHECK(list_configs_command_info.porcelain_output);
            CHECK_FALSE(list_configs_command_info.sorted_output);
        }

        SUBCASE("Valid case with flags #3")
        {
            const std::vector arguments = {"./easy-make", "list-configs", "--incomplete-only", "--sorted"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListConfigurationsCommandInfo>(*command_info));

            const auto& list_configs_command_info = std::get<ListConfigurationsCommandInfo>(*command_info);
            CHECK_FALSE(list_configs_command_info.complete_configurations_only);
            CHECK_FALSE(list_configs_command_info.count);
            CHECK(list_configs_command_info.incomplete_configurations_only);
            CHECK_FALSE(list_configs_command_info.porcelain_output);
            CHECK(list_configs_command_info.sorted_output);
        }

        SUBCASE("Valid case with flags #4")
        {
            const std::vector arguments = {"./easy-make", "list-configs", "--complete-only", "--count"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListConfigurationsCommandInfo>(*command_info));

            const auto& list_configs_command_info = std::get<ListConfigurationsCommandInfo>(*command_info);
            CHECK(list_configs_command_info.complete_configurations_only);
            CHECK(list_configs_command_info.count);
            CHECK_FALSE(list_configs_command_info.incomplete_configurations_only);
            CHECK_FALSE(list_configs_command_info.porcelain_output);
            CHECK_FALSE(list_configs_command_info.sorted_output);
        }

        SUBCASE("Conflicting flags #1")
        {
            const std::vector arguments = {
                "./easy-make", "list-configs", "--incomplete-only", "--complete-only", "--sorted"};
            const auto command_info = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            REQUIRE_EQ(
                command_info.error(),
                "Error: Both '--complete-only' and '--incomplete-only' flags were supplied to command 'list-configs'.");
        }

        SUBCASE("Conflicting flags #2")
        {
            const std::vector arguments = {"./easy-make", "list-configs", "--count", "--sorted"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            REQUIRE_EQ(
                command_info.error(),
                "Error: Cannot provide '--sorted' flag to command 'list-configs' when '--count' flag is provided.");
        }
    }

    TEST_CASE("'version' command")
    {
        SUBCASE("Valid case")
        {
            const std::vector arguments = {"./easy-make", "version"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<PrintVersionCommandInfo>(*command_info));
        }

        SUBCASE("Invalid argument")
        {
            const std::vector arguments = {"./easy-make", "version", "foo"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            REQUIRE_EQ(command_info.error(), "Error: Command 'version' doesn't accept any arguments or flags.");
        }
    }

    TEST_CASE("Invalid commands")
    {
        SUBCASE("No command")
        {
            const std::vector arguments = {"./easy-make"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Must specify a command.");
        }

        SUBCASE("Nonexistent command")
        {
            const std::vector arguments = {"./easy-make", "nonexistent"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown command 'nonexistent'.");
        }

        SUBCASE("Nonexistent command similar to a valid one")
        {
            const std::vector arguments = {"./easy-make", "buil"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown command 'buil'. Did you mean 'build'?");
        }
    }
}

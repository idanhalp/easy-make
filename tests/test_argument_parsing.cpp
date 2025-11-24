#include <variant>
#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/argument_parsing/argument_parsing.hpp"
#include "source/argument_parsing/command_info.hpp"

TEST_SUITE("argument_parsing")
{
    TEST_CASE("'build' command")
    {
        SUBCASE("Valid case without flags")
        {
            const std::vector arguments = {"./easy-make", "build", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<BuildCommandInfo>(*command_info));

            const auto& build_command_info = std::get<BuildCommandInfo>(*command_info);
            CHECK_EQ(build_command_info.configuration_name, "config-name");
            CHECK_FALSE(build_command_info.is_quiet);
        }

        SUBCASE("Valid case with flag")
        {
            const std::vector arguments = {"./easy-make", "build", "config-name", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<BuildCommandInfo>(*command_info));

            const auto& build_command_info = std::get<BuildCommandInfo>(*command_info);
            CHECK_EQ(build_command_info.configuration_name, "config-name");
            CHECK(build_command_info.is_quiet);
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

        SUBCASE("Same configuration name provided more than once")
        {
            const std::vector arguments = {"./easy-make", "build", "config-name", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Command 'build' requires one configuration name, "
                     "instead got 'config-name' more than once.");
        }

        SUBCASE("Duplicate flag")
        {
            const std::vector arguments = {"./easy-make", "build", "config-name", "--quiet", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Flag '--quiet' was provided to command 'build' more than once.");
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

        SUBCASE("Same configuration name provided more than once")
        {
            const std::vector arguments = {"./easy-make", "clean", "config-name", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Command 'clean' requires one configuration name, "
                     "instead got 'config-name' more than once.");
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

    TEST_CASE("'init' command")
    {
        SUBCASE("Valid case without flags")
        {
            const std::vector arguments = {"./easy-make", "init"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<InitCommandInfo>(*command_info));

            const auto& init_command_info = std::get<InitCommandInfo>(*command_info);
            CHECK_FALSE(init_command_info.is_quiet);
            CHECK_FALSE(init_command_info.overwrite_existing_configuration_file);
        }

        SUBCASE("Valid case with flags #1")
        {
            const std::vector arguments = {"./easy-make", "init", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<InitCommandInfo>(*command_info));

            const auto& init_command_info = std::get<InitCommandInfo>(*command_info);
            CHECK(init_command_info.is_quiet);
            CHECK_FALSE(init_command_info.overwrite_existing_configuration_file);
        }

        SUBCASE("Valid case with flags #2")
        {
            const std::vector arguments = {"./easy-make", "init", "--overwrite"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<InitCommandInfo>(*command_info));

            const auto& init_command_info = std::get<InitCommandInfo>(*command_info);
            CHECK_FALSE(init_command_info.is_quiet);
            CHECK(init_command_info.overwrite_existing_configuration_file);
        }

        SUBCASE("Valid case with both flags")
        {
            const std::vector arguments = {"./easy-make", "init", "--quiet", "--overwrite"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<InitCommandInfo>(*command_info));

            const auto& init_command_info = std::get<InitCommandInfo>(*command_info);
            CHECK(init_command_info.is_quiet);
            CHECK(init_command_info.overwrite_existing_configuration_file);
        }

        SUBCASE("Invalid argument")
        {
            const std::vector arguments = {"./easy-make", "init", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown argument 'config-name' provided to command 'init'.");
        }

        SUBCASE("Invalid flag")
        {
            const std::vector arguments = {"./easy-make", "init", "--fast"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown flag '--fast' provided to command 'init'.");
        }

        SUBCASE("Invalid flag similar to a valid one")
        {
            const std::vector arguments = {"./easy-make", "init", "--overwreit"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Unknown flag '--overwreit' provided to command 'init'. Did you mean '--overwrite'?");
        }

        SUBCASE("Duplicate flag")
        {
            const std::vector arguments = {"./easy-make", "init", "--quiet", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Flag '--quiet' was provided to command 'init' more than once.");
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

    TEST_CASE("'list-files' command")
    {
        SUBCASE("Valid case without flags")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListFilesCommandInfo>(*command_info));

            const auto& list_files_command_info = std::get<ListFilesCommandInfo>(*command_info);
            CHECK_EQ(list_files_command_info.configuration_name, "config-name");
            CHECK_FALSE(list_files_command_info.count);
            CHECK_FALSE(list_files_command_info.header_only);
            CHECK_FALSE(list_files_command_info.porcelain_output);
            CHECK_FALSE(list_files_command_info.source_only);
        }

        SUBCASE("Valid case with flags #1")
        {
            const std::vector arguments = {
                "./easy-make", "list-files", "config-name-1", "--source-only", "--porcelain"};
            const auto command_info = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListFilesCommandInfo>(*command_info));

            const auto& list_files_command_info = std::get<ListFilesCommandInfo>(*command_info);
            CHECK_EQ(list_files_command_info.configuration_name, "config-name-1");
            CHECK_FALSE(list_files_command_info.count);
            CHECK_FALSE(list_files_command_info.header_only);
            CHECK(list_files_command_info.porcelain_output);
            CHECK(list_files_command_info.source_only);
        }

        SUBCASE("Valid case with flags #2")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name-2", "--header-only"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListFilesCommandInfo>(*command_info));

            const auto& list_files_command_info = std::get<ListFilesCommandInfo>(*command_info);
            CHECK_EQ(list_files_command_info.configuration_name, "config-name-2");
            CHECK_FALSE(list_files_command_info.count);
            CHECK(list_files_command_info.header_only);
            CHECK_FALSE(list_files_command_info.porcelain_output);
            CHECK_FALSE(list_files_command_info.source_only);
        }

        SUBCASE("Valid case with flags #3")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name-3", "--count"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE(command_info.has_value());
            REQUIRE(std::holds_alternative<ListFilesCommandInfo>(*command_info));

            const auto& list_files_command_info = std::get<ListFilesCommandInfo>(*command_info);
            CHECK_EQ(list_files_command_info.configuration_name, "config-name-3");
            CHECK(list_files_command_info.count);
            CHECK_FALSE(list_files_command_info.header_only);
            CHECK_FALSE(list_files_command_info.porcelain_output);
            CHECK_FALSE(list_files_command_info.source_only);
        }

        SUBCASE("Missing configuration name")
        {
            const std::vector arguments = {"./easy-make", "list-files"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Must specify a configuration name when using 'list-files' command.");
        }

        SUBCASE("Missing configuration name with flags provided")
        {
            const std::vector arguments = {"./easy-make", "list-files", "--source-only", "--porcelain"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Must specify a configuration name when using 'list-files' command.");
        }

        SUBCASE("Multiple configuration names")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name-1", "config-name-2"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Command 'list-files' requires one configuration name, "
                     "instead got both 'config-name-1' and 'config-name-2'.");
        }

        SUBCASE("Same configuration name provided more than once")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name", "config-name"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Command 'list-files' requires one configuration name, "
                     "instead got 'config-name' more than once.");
        }

        SUBCASE("Conflicting flags #1")
        {
            const std::vector arguments = {
                "./easy-make", "list-files", "config-name", "--source-only", "--header-only"};
            const auto command_info = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            REQUIRE_EQ(command_info.error(),
                       "Error: Both '--header-only' and '--source-only' flags were supplied to command 'list-files'.");
        }

        SUBCASE("Conflicting flags #2")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name", "--count", "--porcelain"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            REQUIRE_EQ(
                command_info.error(),
                "Error: Cannot provide '--porcelain' flag to command 'list-files' when '--count' flag is provided.");
        }

        SUBCASE("Invalid flag")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name", "--count", "--quiet"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(), "Error: Unknown flag '--quiet' provided to command 'list-files'.");
        }

        SUBCASE("Duplicate flag")
        {
            const std::vector arguments = {"./easy-make", "list-files", "config-name", "--count", "--count"};
            const auto command_info     = parse_arguments(arguments);

            REQUIRE_FALSE(command_info.has_value());
            CHECK_EQ(command_info.error(),
                     "Error: Flag '--count' was provided to command 'list-files' more than once.");
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

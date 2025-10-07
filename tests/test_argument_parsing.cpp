#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/argument_parsing/argument_parsing.hpp"

TEST_SUITE("argument_parsing")
{
    TEST_CASE("valid arguments")
    {
        const std::vector arguments = {"./easy-make", "release", "--clean"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE(argument_info.has_value());
        CHECK_EQ(argument_info->configuration_name, "release");
        CHECK(argument_info->clean_configuration);
        CHECK_FALSE(argument_info->clean_all_configurations);
        CHECK_FALSE(argument_info->print_version);
    }

    TEST_CASE("valid configuration #1")
    {
        const std::vector arguments = {"./easy-make", "release"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE(argument_info.has_value());
        CHECK_EQ(argument_info->configuration_name, "release");
        CHECK_FALSE(argument_info->clean_configuration);
        CHECK_FALSE(argument_info->clean_all_configurations);
        CHECK_FALSE(argument_info->print_version);
    }

    TEST_CASE("valid configuration #2")
    {
        const std::vector arguments = {"./easy-make", "--version"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE(argument_info.has_value());
        CHECK_FALSE(argument_info->clean_configuration);
        CHECK_FALSE(argument_info->clean_all_configurations);
        CHECK(argument_info->print_version);
    }

    TEST_CASE("valid configuration #3")
    {
        const std::vector arguments = {"./easy-make", "--clean", "debug"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE(argument_info.has_value());
        CHECK_EQ(argument_info->configuration_name, "debug");
        CHECK(argument_info->clean_configuration);
        CHECK_FALSE(argument_info->clean_all_configurations);
        CHECK_FALSE(argument_info->print_version);
    }

    TEST_CASE("valid configuration #4")
    {
        const std::vector arguments = {"./easy-make", "--clean-all"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE(argument_info.has_value());
        CHECK_FALSE(argument_info->clean_configuration);
        CHECK(argument_info->clean_all_configurations);
        CHECK_FALSE(argument_info->print_version);
    }

    TEST_CASE("invalid flag")
    {
        const std::vector arguments = {"./easy-make", "debug", "--nonexistent"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(), "Error: Unknown argument '--nonexistent'.");
    }

    TEST_CASE("invalid flag with close match")
    {
        const std::vector arguments = {"./easy-make", "--versio"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(), "Error: Unknown argument '--versio'. Did you mean '--version'?");
    }

    TEST_CASE("duplicate flag #1")
    {
        const std::vector arguments = {"./easy-make", "debug", "--clean", "--clean"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(), "Error: Flag '--clean' was specified more than once.");
    }

    TEST_CASE("duplicate flag #2")
    {
        const std::vector arguments = {"./easy-make", "--version", "--version"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(), "Error: Flag '--version' was specified more than once.");
    }

    TEST_CASE("conflicting flags #1")
    {
        const std::vector arguments = {"./easy-make", "--clean", "--version"};
        const auto argument_info    = parse_arguments(arguments);

        CHECK_FALSE(argument_info.has_value());
    }

    TEST_CASE("conflicting flags #2")
    {
        const std::vector arguments = {"./easy-make", "--clean", "--clean-all"};
        const auto argument_info    = parse_arguments(arguments);

        CHECK_FALSE(argument_info.has_value());
    }

    TEST_CASE("conflicting flags #3")
    {
        const std::vector arguments = {"./easy-make", "--version", "--clean-all"};
        const auto argument_info    = parse_arguments(arguments);

        CHECK_FALSE(argument_info.has_value());
    }

    TEST_CASE("no configuration name with --clean-all")
    {
        const std::vector arguments = {"./easy-make", "name", "--clean-all"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(),
                 "Error: Cannot specify a configuration name ('name') when '--clean-all' is used.");
    }

    TEST_CASE("no configuration name with --version")
    {
        const std::vector arguments = {"./easy-make", "name", "--version"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(),
                 "Error: Cannot specify a configuration name ('name') when '--version' is used.");
    }

    TEST_CASE("no configuration name for build")
    {
        const std::vector arguments = {"./easy-make"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(), "Error: Must specify a configuration name when building.");
    }

    TEST_CASE("no configuration name for --clean")
    {
        const std::vector arguments = {"./easy-make", "--clean"};
        const auto argument_info    = parse_arguments(arguments);

        REQUIRE_FALSE(argument_info.has_value());
        CHECK_EQ(argument_info.error(), "Error: Must specify a configuration name when '--clean' is used.");
    }
}

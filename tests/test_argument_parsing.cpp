#include "tests/tests.hpp"

#include <cassert>
#include <print>
#include <vector>

#include "source/argument_parsing/argument_parsing.hpp"

static auto test_valid_arguments() -> void
{
    const std::vector arguments = {"./easy-make", "release", "--clean"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(argument_info->configuration_name == "release");
    assert(argument_info->clean_configuration);
    assert(!argument_info->clean_all_configurations);
    assert(!argument_info->print_version);
}

static auto test_defaults() -> void
{
    const std::vector arguments = {"./easy-make"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(argument_info->configuration_name == "default");
    assert(!argument_info->clean_configuration);
    assert(!argument_info->clean_all_configurations);
    assert(!argument_info->print_version);
}

static auto test_valid_configuration_1() -> void
{
    const std::vector arguments = {"./easy-make", "release"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(argument_info->configuration_name == "release");
    assert(!argument_info->clean_configuration);
    assert(!argument_info->clean_all_configurations);
    assert(!argument_info->print_version);
}

static auto test_valid_configuration_2() -> void
{
    const std::vector arguments = {"./easy-make", "--version"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(!argument_info->clean_configuration);
    assert(!argument_info->clean_all_configurations);
    assert(argument_info->print_version);
}

static auto test_valid_configuration_3() -> void
{
    const std::vector arguments = {"./easy-make", "--clean", "debug"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(argument_info->configuration_name == "debug");
    assert(argument_info->clean_configuration);
    assert(!argument_info->clean_all_configurations);
    assert(!argument_info->print_version);
}

static auto test_valid_configuration_4() -> void
{
    const std::vector arguments = {"./easy-make", "--clean-all"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(!argument_info->clean_configuration);
    assert(argument_info->clean_all_configurations);
    assert(!argument_info->print_version);
}

static auto test_invalid_flag() -> void
{
    const std::vector arguments = {"./easy-make", "debug", "--nonexistent"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value() && argument_info.error() == "Error: Unknown argument '--nonexistent'.");
}

static auto test_duplicate_flag_1() -> void
{
    const std::vector arguments = {"./easy-make", "debug", "--clean", "--clean"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value() &&
           argument_info.error() == "Error: Flag '--clean' was specified more than once.");
}

static auto test_duplicate_flag_2() -> void
{
    const std::vector arguments = {"./easy-make", "--version", "--version"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value() &&
           argument_info.error() == "Error: Flag '--version' was specified more than once.");
}

static auto test_conflicting_flags_1() -> void
{
    const std::vector arguments = {"./easy-make", "--clean", "--version"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value());
}

static auto test_conflicting_flags_2() -> void
{
    const std::vector arguments = {"./easy-make", "--clean", "--clean-all"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value());
}

static auto test_conflicting_flags_3() -> void
{
    const std::vector arguments = {"./easy-make", "--version", "--clean-all"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value());
}

static auto test_no_configuration_name_with_clean_all() -> void
{
    const std::vector arguments = {"./easy-make", "name", "--clean-all"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value() &&
           argument_info.error() == "Error: Cannot specify a configuration name ('name') when '--clean-all' is used.");
}

static auto test_no_configuration_name_with_print_version() -> void
{
    const std::vector arguments = {"./easy-make", "name", "--version"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value() &&
           argument_info.error() == "Error: Cannot specify a configuration name ('name') when '--version' is used.");
}

auto tests::test_argument_parsing() -> void
{
    std::println("Running `prase_arguments` tests.");

    test_valid_arguments();
    test_defaults();
    test_valid_configuration_1();
    test_valid_configuration_2();
    test_valid_configuration_3();
    test_valid_configuration_4();
    test_invalid_flag();
    test_duplicate_flag_1();
    test_duplicate_flag_2();
    test_conflicting_flags_1();
    test_conflicting_flags_2();
    test_conflicting_flags_3();
    test_no_configuration_name_with_clean_all();
    test_no_configuration_name_with_print_version();

    std::println("Done.");
}

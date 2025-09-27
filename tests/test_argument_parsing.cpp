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
    assert(argument_info->clean_object_files);
}

static auto test_defaults() -> void
{
    const std::vector arguments = {"./easy-make"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(argument_info->configuration_name == "default");
    assert(!argument_info->clean_object_files);
    assert(!argument_info->print_version);
}

static auto test_valid_configuration() -> void
{
    const std::vector arguments = {"./easy-make", "--version", "release"};
    const auto argument_info    = parse_arguments(arguments);

    assert(argument_info.has_value());
    assert(argument_info->configuration_name == "release");
    assert(!argument_info->clean_object_files);
    assert(argument_info->print_version);
}

static auto test_invalid_flag() -> void
{
    const std::vector arguments = {"./easy-make", "debug", "--nonexistent"};
    const auto argument_info    = parse_arguments(arguments);

    assert(!argument_info.has_value() && argument_info.error().contains("--nonexistent"));
}

auto tests::test_argument_parsing() -> void
{
    std::println("Running `prase_arguments` tests.");

    test_valid_arguments();
    test_defaults();
    test_valid_configuration();
    test_invalid_flag();

    std::println("Done.");
}

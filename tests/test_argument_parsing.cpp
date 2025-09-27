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
}

auto tests::test_argument_parsing() -> void
{
    std::println("Running `prase_arguments` tests.");

    test_valid_arguments();
    test_defaults();

    std::println("Done.");
}

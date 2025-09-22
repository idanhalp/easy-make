#include "tests/tests.hpp"

#include <filesystem>
#include <print>

auto main() -> int
{
    std::println("Running tests.");

    tests::test_check_if_configurations_file_exists();
    tests::test_get_chosen_configuration();
    tests::test_utils();

    std::println("All tests passed.");
}

#include "tests/tests.hpp"

#include <filesystem>
#include <print>

auto main() -> int
{
    std::println("Running tests.");

    tests::test_check_if_configurations_file_exists();
    tests::test_configuration_parsing();
    tests::test_get_chosen_configuration();
    tests::test_utils();
    tests::test_executable_creation();
    tests::test_build_caching();

    std::println("All tests passed.");
}

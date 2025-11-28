#include <cstdlib>
#include <format>

#include "third_party/doctest/doctest.hpp"

#include "tests/parameters.hpp"
#include "tests/regression_tests/utils/test_environment_guard.hpp"

TEST_SUITE("Regression tests - bug #1" * doctest::test_suite(test_type::regression))
{
    TEST_CASE_FIXTURE(TestEnvironmentGuard<1>, "Building without any flag")
    {
        const auto result = std::system(std::format("{} build config-1", get_path_to_exe().native()).c_str());
        CHECK_EQ(result, EXIT_SUCCESS);
    }
}

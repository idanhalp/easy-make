#include <cstdlib>
#include <format>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/build/build.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/parameters.hpp"
#include "tests/regression_tests/utils/test_environment_guard.hpp"

TEST_CASE_FIXTURE(TestEnvironmentGuard<1>, "Building without any flag [regression]")
{
    /*
    The configuration does not have any flags (no warnings, no optimization et cetera).
    This test checks the program does not crash because of the lack of flags.
    */

    const auto root_path = std::filesystem::current_path();
    const auto info      = BuildCommandInfo{
             .configuration_name       = "config-1",
             .build_all_configurations = false,
             .is_quiet                 = true,
             .use_parallel_compilation = false,
    };

    const auto configurations = parse_configurations(root_path);
    REQUIRE(configurations.has_value());

    const auto exit_status = commands::build(info, *configurations, root_path).exit_status;
    REQUIRE_EQ(exit_status, EXIT_SUCCESS);
}

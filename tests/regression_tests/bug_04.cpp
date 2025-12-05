#include "third_party/doctest/doctest.hpp"

#include "source/commands/build/build.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/parameters.hpp"
#include "tests/regression_tests/utils/test_environment_guard.hpp"

TEST_CASE_FIXTURE(TestEnvironmentGuard<4>, "Empty dependency graph [regression]")
{
    const auto root_path = std::filesystem::current_path();
    const auto info      = BuildCommandInfo{
             .configuration_name       = "config",
             .is_quiet                 = true,
             .use_parallel_compilation = false,
    };
    const auto configurations = parse_configurations(root_path);
    REQUIRE(configurations.has_value());
    REQUIRE_EQ(commands::build(info, *configurations, root_path).exit_status, EXIT_SUCCESS);
}

#include "tests/tests.hpp"

#include <cassert>
#include <print>

#include "tests/utils/utils.hpp"

static auto test_get_path_to_resources_project() -> void
{
    assert(tests::utils::get_path_to_resources_project(0).string().ends_with("easy-make/tests/resources/project_00"));
    assert(tests::utils::get_path_to_resources_project(7).string().ends_with("easy-make/tests/resources/project_07"));
    assert(tests::utils::get_path_to_resources_project(10).string().ends_with("easy-make/tests/resources/project_10"));
    assert(tests::utils::get_path_to_resources_project(16).string().ends_with("easy-make/tests/resources/project_16"));
}

auto tests::test_utils() -> void
{
    std::println("Running `utils` tests.");

    test_get_path_to_resources_project();

    std::println("Done.");
}

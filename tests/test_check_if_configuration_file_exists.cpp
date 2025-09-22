#include "tests/tests.hpp"

#include <cassert>
#include <print>

#include "source/utils/utils.hpp"
#include "tests/utils/utils.hpp"

auto tests::test_check_if_configurations_file_exists() -> void
{
    std::println("Running `check_if_configurations_file_exists` tests.");

    // Config file does not exists.
    const auto project_1_path = tests::utils::get_path_to_resources_project(1);
    assert(!::utils::check_if_configurations_file_exists(project_1_path));

    // Config file exists.
    const auto project_2_path = tests::utils::get_path_to_resources_project(2);
    assert(::utils::check_if_configurations_file_exists(project_2_path));

    // Config file is not at the root directory.
    const auto project_3_path = tests::utils::get_path_to_resources_project(3);
    assert(!::utils::check_if_configurations_file_exists(project_3_path));

    std::println("Done.");
}

#include <print>

#include "third_party/doctest/doctest.hpp"

#include "source/utils/utils.hpp"
#include "tests/utils/utils.hpp"

TEST_SUITE("check_if_configurations_file_exists")
{
    TEST_CASE("Checks if configuration file exists correctly")
    {
        // Config file does not exist.
        const auto project_1_path = tests::utils::get_path_to_resources_project(1);
        CHECK_FALSE(::utils::check_if_configurations_file_exists(project_1_path));

        // Config file exists.
        const auto project_2_path = tests::utils::get_path_to_resources_project(2);
        CHECK(::utils::check_if_configurations_file_exists(project_2_path));

        // Config file is not at the root directory.
        const auto project_3_path = tests::utils::get_path_to_resources_project(3);
        CHECK_FALSE(::utils::check_if_configurations_file_exists(project_3_path));
    }
}

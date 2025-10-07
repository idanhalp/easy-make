#include <filesystem>

#include "third_party/doctest/doctest.hpp"

#include "source/utils/utils.hpp"
#include "tests/utils/utils.hpp"

TEST_SUITE("Basic utils")
{
    TEST_CASE("'get_path_to_resources_project' works.")
    {
        CHECK(
            tests::utils::get_path_to_resources_project(0).string().ends_with("easy-make/tests/resources/project_00"));
        CHECK(
            tests::utils::get_path_to_resources_project(7).string().ends_with("easy-make/tests/resources/project_07"));
        CHECK(
            tests::utils::get_path_to_resources_project(10).string().ends_with("easy-make/tests/resources/project_10"));
        CHECK(
            tests::utils::get_path_to_resources_project(16).string().ends_with("easy-make/tests/resources/project_16"));
    }

    TEST_CASE("'get_object_file_name' works.")
    {
        CHECK_EQ(utils::get_object_file_name(std::filesystem::path("a") / "b" / "c"), "a-b-c.o");
        CHECK_EQ(utils::get_object_file_name(std::filesystem::path("a") / "b" / "c.cpp"), "a-b-c.cpp.o");
    }
}

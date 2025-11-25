#include <filesystem>
#include <limits>

#include "third_party/doctest/doctest.hpp"

#include "source/utils/utils.hpp"
#include "tests/unit_tests/utils/utils.hpp"

TEST_SUITE("Basic utils")
{
    TEST_CASE("'get_path_to_resources_project' works.")
    {
        CHECK(tests::utils::get_path_to_resources_project(0).native().ends_with(
            "easy-make/tests/unit_tests/resources/project_00"));
        CHECK(tests::utils::get_path_to_resources_project(7).native().ends_with(
            "easy-make/tests/unit_tests/resources/project_07"));
        CHECK(tests::utils::get_path_to_resources_project(10).native().ends_with(
            "easy-make/tests/unit_tests/resources/project_10"));
        CHECK(tests::utils::get_path_to_resources_project(16).native().ends_with(
            "easy-make/tests/unit_tests/resources/project_16"));
    }

    TEST_CASE("'get_object_file_name' works.")
    {
        CHECK_EQ(utils::get_object_file_name(std::filesystem::path("a") / "b" / "c"), "a-b-c.o");
        CHECK_EQ(utils::get_object_file_name(std::filesystem::path("a") / "b" / "c.cpp"), "a-b-c.cpp.o");
    }

    TEST_CASE("utils::count_digits")
    {
        SUBCASE("Zero")
        {
            CHECK_EQ(utils::count_digits(0), 1);
        }

        SUBCASE("Single-digit numbers")
        {
            CHECK_EQ(utils::count_digits(1), 1);
            CHECK_EQ(utils::count_digits(9), 1);
            CHECK_EQ(utils::count_digits(-5), 1);
        }

        SUBCASE("Multi-digit numbers")
        {
            CHECK_EQ(utils::count_digits(10), 2);
            CHECK_EQ(utils::count_digits(99), 2);
            CHECK_EQ(utils::count_digits(100), 3);
            CHECK_EQ(utils::count_digits(12345), 5);
        }

        SUBCASE("Negative numbers")
        {
            CHECK_EQ(utils::count_digits(-1), 1);
            CHECK_EQ(utils::count_digits(-10), 2);
            CHECK_EQ(utils::count_digits(-99999), 5);
        }

        SUBCASE("Powers of ten (possible rounding issues)")
        {
            CHECK_EQ(utils::count_digits(1000), 4);
            CHECK_EQ(utils::count_digits(1000000), 7);
            CHECK_EQ(utils::count_digits(-1000000), 7);
        }

        SUBCASE("Large numbers")
        {
            CHECK_EQ(utils::count_digits(std::numeric_limits<int>::max()), 10);
            CHECK_EQ(utils::count_digits(std::numeric_limits<int>::min()), 10);
        }
    }
}

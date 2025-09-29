#include "tests/tests.hpp"

#include <cassert>
#include <filesystem>
#include <print>

#include "source/utils/utils.hpp"
#include "tests/utils/utils.hpp"

static auto test_get_path_to_resources_project() -> void
{
    assert(tests::utils::get_path_to_resources_project(0).string().ends_with("easy-make/tests/resources/project_00"));
    assert(tests::utils::get_path_to_resources_project(7).string().ends_with("easy-make/tests/resources/project_07"));
    assert(tests::utils::get_path_to_resources_project(10).string().ends_with("easy-make/tests/resources/project_10"));
    assert(tests::utils::get_path_to_resources_project(16).string().ends_with("easy-make/tests/resources/project_16"));
}

static auto test_get_object_file_name() -> void
{
    assert(utils::get_object_file_name(std::filesystem::path("a") / "b" / "c") == "a-b-c.o");
    assert(utils::get_object_file_name(std::filesystem::path("a") / "b" / "c.cpp") == "a-b-c.cpp.o");
}

auto tests::test_utils() -> void
{
    std::println("Running `utils` tests.");

    test_get_path_to_resources_project();
    test_get_object_file_name();

    std::println("Done.");
}

#include "tests/unit_tests/utils/utils.hpp"

#include <cassert>
#include <filesystem>
#include <format>

auto tests::utils::get_path_to_resources_project(const int index) -> std::filesystem::path
{
    assert(0 <= index && index <= MAX_PROJECT_INDEX);

    return std::filesystem::current_path() / "tests" / "unit_tests" / "resources" / std::format("project_{:02}", index);
}

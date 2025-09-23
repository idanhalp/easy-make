#include "tests/utils/utils.hpp"

#include <filesystem>
#include <format>
#include <source_location>

auto tests::utils::get_path_to_resources_project(const int index) -> std::filesystem::path
{
    const auto utils_directory_path     = std::filesystem::absolute(std::source_location::current().file_name());
    const auto tests_directory_path     = utils_directory_path.parent_path().parent_path();
    const auto resources_directory_path = tests_directory_path / "resources";
    const auto project_path             = resources_directory_path / std::format("project_{:02}", index);

    return project_path;
}

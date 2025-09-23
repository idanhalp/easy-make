#ifndef TESTS_UTILS_UTILS_HPP
#define TESTS_UTILS_UTILS_HPP

#include <filesystem>
#include <string>

namespace tests::utils
{
    auto get_path_to_resources_project(int index) -> std::filesystem::path;
}

#endif // TESTS_UTILS_UTILS_HPP

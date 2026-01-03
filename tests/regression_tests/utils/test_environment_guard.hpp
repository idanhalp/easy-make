#ifndef TESTS_REGRESSION_UTILS_TEST_ENVIRONMENT_GUARD_HPP
#define TESTS_REGRESSION_UTILS_TEST_ENVIRONMENT_GUARD_HPP

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <format>

#include "source/parameters/parameters.hpp"

template <int Index>
class TestEnvironmentGuard
{
  public:
    static_assert(Index > 0, "Index must be a positive number.");

    TestEnvironmentGuard();  // Called before the test.
    ~TestEnvironmentGuard(); // Called after the test.

  private:
    const std::filesystem::path original_path;
};

template <int Index>
TestEnvironmentGuard<Index>::TestEnvironmentGuard() : original_path(std::filesystem::current_path())
{
    std::system("easy-make build debug --quiet");

    const auto path_to_project =
        original_path / "tests" / "regression_tests" / "resources" / std::format("project_{:02}", Index);

    assert(std::filesystem::exists(path_to_project));
    std::filesystem::current_path(path_to_project);
}

template <int Index>
TestEnvironmentGuard<Index>::~TestEnvironmentGuard()
{
    std::filesystem::remove(std::filesystem::current_path() / "output.exe");
    std::filesystem::remove_all(std::filesystem::current_path() / params::BUILD_DIRECTORY_NAME);
    std::filesystem::current_path(original_path);
}

#endif // TESTS_REGRESSION_UTILS_TEST_ENVIRONMENT_GUARD_HPP

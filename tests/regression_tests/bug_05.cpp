#include <filesystem>
#include <fstream>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/build/build.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/parameters.hpp"
#include "tests/regression_tests/utils/test_environment_guard.hpp"

namespace
{
    class FileCreator
    {
      public:
        FileCreator();
        ~FileCreator();

        auto create_file() -> void;
        auto change_file_name() -> void;

      private:
        const std::filesystem::path before = "f_1.cpp";
        const std::filesystem::path after  = "f_2.cpp";
    };
}

FileCreator::FileCreator()
{
    create_file();
}

FileCreator::~FileCreator()
{
    std::filesystem::remove(before);
    std::filesystem::remove(after);
}

auto FileCreator::create_file() -> void
{
    auto file          = std::ofstream(before);
    const auto content = "auto foo() -> void {}";

    file << content;
}

auto FileCreator::change_file_name() -> void
{
    std::filesystem::rename(before, after);
}

TEST_SUITE("Regression tests - bug #5" * doctest::test_suite(test_type::regression))
{
    TEST_CASE_FIXTURE(TestEnvironmentGuard<5>, "Delete object files of removed files")
    {
        const auto root_path      = std::filesystem::current_path();
        const auto info           = BuildCommandInfo{.configuration_name = "config", .is_quiet = true};
        const auto configurations = parse_configurations(root_path);
        REQUIRE(configurations.has_value());

        // Create a file which defines `void foo()`.
        // Everything should compile without any problem.
        FileCreator file_creator{};
        REQUIRE_EQ(commands::build(info, *configurations, root_path).exit_status, EXIT_SUCCESS);

        // Rename the file. The object file for the old path must be removed;
        // otherwise the build would contain two definitions of `foo()` and fail due to an ODR violation.
        // The build should still succeed if the issue is fixed.
        file_creator.change_file_name();
        REQUIRE_EQ(commands::build(info, *configurations, root_path).exit_status, EXIT_SUCCESS);
    }
}

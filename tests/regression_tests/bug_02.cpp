#include <filesystem>
#include <fstream>
#include <string_view>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/build/build.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "tests/parameters.hpp"
#include "tests/regression_tests/utils/test_environment_guard.hpp"

using namespace std::literals;

static const auto valid_content = R"(
    auto main() -> int
    {
        return 0;    
    }
)"sv;

static const auto invalid_content = R"(
    auto main() -> int
    {
        return 0 // Missing a semicolon.
    }
)"sv;

namespace
{
    class FileCreator
    {
      public:
        FileCreator();
        ~FileCreator();

        auto write_content(std::string_view content) -> void;

      private:
        const std::filesystem::path main_file = "main.cpp";
    };
}

FileCreator::FileCreator()
{
    write_content(valid_content);
}

FileCreator::~FileCreator()
{
    std::filesystem::remove(main_file);
}

auto FileCreator::write_content(const std::string_view content) -> void
{
    auto main = std::ofstream(main_file, std::ios::trunc);
    main << content;
}

TEST_SUITE("Regression tests - bug #2" * doctest::test_suite(test_type::regression))
{
    TEST_CASE_FIXTURE(TestEnvironmentGuard<2>, "Recompilation after failure")
    {
        FileCreator file_creator{}; // Creates main.cpp; can be modified; removed on destruction.

        const auto info           = BuildCommandInfo{.configuration_name = "config", .is_quiet = true};
        const auto configurations = parse_configurations(std::filesystem::current_path());

        REQUIRE(configurations.has_value());

        // Currently the contents of the file are valid.
        const auto before_failure = commands::build(info, *configurations, std::filesystem::current_path()).exit_status;
        REQUIRE_EQ(before_failure, EXIT_SUCCESS);

        // Change contents of the file to be invalid.
        file_creator.write_content(invalid_content);

        // Compilation should fail now.
        const auto first_failure = commands::build(info, *configurations, std::filesystem::current_path()).exit_status;
        REQUIRE_NE(first_failure, EXIT_SUCCESS);

        // Make sure that even though the compilation was initially successful and the fact that
        // the contents of the file did not change from last time, compilation still fails.
        const auto second_failure = commands::build(info, *configurations, std::filesystem::current_path()).exit_status;
        REQUIRE_NE(second_failure, EXIT_SUCCESS);
    }
}

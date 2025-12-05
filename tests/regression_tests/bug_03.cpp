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
        FileCreator() = default;
        ~FileCreator();

        auto create() -> void;
        auto remove() -> void;

      private:
        const std::filesystem::path file_name = "f_3.hh";
    };
}

FileCreator::~FileCreator() // To make sure file is removed in case of a crash.
{
    remove();
}

auto FileCreator::create() -> void
{
    std::ofstream file(file_name);
}

auto FileCreator::remove() -> void
{
    std::filesystem::remove(file_name);
}

TEST_CASE_FIXTURE(TestEnvironmentGuard<3>, "Recompilation after removing a file [regression]")
{
    const auto root_path = std::filesystem::current_path();
    FileCreator file_creator;

    const auto info = BuildCommandInfo{
        .configuration_name       = "config",
        .is_quiet                 = true,
        .use_parallel_compilation = false,
    };
    const auto configurations = parse_configurations(root_path);
    REQUIRE(configurations.has_value());

    /*
    The project contains 3 code files:
    - main.cpp (includes 'f_3.hh')
    - f_1.cpp (includes 'f_3.hh')
    - f_2.cpp (doesn't include 'f_3.hh')

    We create 'f_3.hh' and compile.
    Compilation is expected to be successful.
    */
    file_creator.create();
    REQUIRE_EQ(commands::build(info, *configurations, root_path).num_of_compilation_failures, 0);

    /*
    Now we remove 'f_3.hh'.
    Only the 2 affected files should be recompiled.
    */
    file_creator.remove();
    REQUIRE_EQ(commands::build(info, *configurations, root_path).num_of_compilation_failures, 2);
}

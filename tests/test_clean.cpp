#include <filesystem>
#include <fstream>
#include <vector>

#include "third_party/doctest/doctest.hpp"

#include "source/commands/clean/clean.hpp"
#include "source/commands/clean_all/clean_all.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "source/parameters/parameters.hpp"
#include "tests/utils/utils.hpp"

namespace
{
    struct TestSetUp
    {
        TestSetUp();
        ~TestSetUp();
    };
}

static const auto path_to_project = tests::utils::get_path_to_resources_project(tests::utils::MAX_PROJECT_INDEX);

static auto get_configurations() -> std::vector<Configuration>
{
    Configuration conf_1;
    conf_1.name        = "conf_1";
    conf_1.compiler    = "g++";
    conf_1.output_name = "conf_1.exe";

    Configuration conf_2;
    conf_2.name        = "conf_2";
    conf_2.compiler    = "g++";
    conf_2.output_path = "dir_1";
    conf_2.output_name = "conf_2.exe";

    Configuration conf_3;
    conf_3.name        = "conf_3";
    conf_3.compiler    = "g++";
    conf_3.output_path = "dir_1";
    conf_3.output_name = "conf_3.exe";

    Configuration conf_4;
    conf_4.name   = "conf_4";
    conf_4.parent = "conf_3";

    Configuration conf_5;
    conf_5.name        = "conf_5";
    conf_5.compiler    = "g++";
    conf_5.output_name = "conf_5.exe";

    return {conf_1, conf_2, conf_3, conf_4, conf_5};
}

// Called before all tests.
TestSetUp::TestSetUp()
{
    std::filesystem::create_directories(path_to_project / "dir_1");
    std::ofstream(path_to_project / "conf_1.exe");
    std::ofstream(path_to_project / "dir_1" / "conf_2.exe");
    std::ofstream(path_to_project / "dir_1" / "conf_3.exe");

    std::filesystem::create_directories(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_1");
    std::filesystem::create_directories(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_2");
    std::filesystem::create_directories(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_3");
    std::filesystem::create_directories(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_4");
}

// Called after all tests.
TestSetUp::~TestSetUp()
{
    std::filesystem::remove_all(path_to_project);
}

TEST_SUITE("commands::clean")
{
    TEST_CASE_FIXTURE(TestSetUp, "Basic functionality")
    {
        REQUIRE(std::filesystem::exists(path_to_project / "conf_1.exe")); // Exists before clean.

        const CleanCommandInfo info = {.configuration_name = "conf_1", .is_quiet = true};
        const auto result           = commands::clean(info, get_configurations(), path_to_project);

        CHECK(result == EXIT_SUCCESS);
        REQUIRE(!std::filesystem::exists(path_to_project / "conf_1.exe"));
        REQUIRE(!std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_1"));

        // Make sure other executables were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_2.exe"));
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_3.exe"));

        // Make sure other build subdirectories were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_2"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_3"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_4"));
    }

    TEST_CASE_FIXTURE(TestSetUp, "Executable inside a directory")
    {
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_2.exe")); // Exists before clean.

        const CleanCommandInfo info = {.configuration_name = "conf_2", .is_quiet = true};
        const auto result           = commands::clean(info, get_configurations(), path_to_project);

        CHECK(result == EXIT_SUCCESS);
        REQUIRE(!std::filesystem::exists(path_to_project / "dir_1" / "conf_2.exe"));
        REQUIRE(!std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_2"));

        // Make sure other executables were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / "conf_1.exe"));
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_3.exe"));

        // Make sure other build subdirectories were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_1"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_3"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_4"));
    }

    TEST_CASE_FIXTURE(TestSetUp, "Configuration inheriting exe name and path")
    {
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_3.exe")); // Exists before clean.

        const CleanCommandInfo info = {.configuration_name = "conf_4", .is_quiet = true};
        const auto result           = commands::clean(info, get_configurations(), path_to_project);

        CHECK(result == EXIT_SUCCESS);
        REQUIRE(!std::filesystem::exists(path_to_project / "dir_1" / "conf_3.exe"));
        REQUIRE(!std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_4"));

        // Make sure other executables were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / "conf_1.exe"));
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_2.exe"));

        // Make sure other build subdirectories were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_1"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_2"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_3"));
    }

    TEST_CASE_FIXTURE(TestSetUp, "Configuration without exe")
    {
        REQUIRE(!std::filesystem::exists(path_to_project / "conf_5.exe")); // Does not exists before clean.

        const CleanCommandInfo info = {.configuration_name = "conf_5", .is_quiet = true};
        const auto result           = commands::clean(info, get_configurations(), path_to_project);

        CHECK(result == EXIT_FAILURE);

        // Make sure other executables were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / "conf_1.exe"));
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_2.exe"));
        REQUIRE(std::filesystem::exists(path_to_project / "dir_1" / "conf_3.exe"));

        // Make sure other build subdirectories were not deleted.
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_1"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_2"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_3"));
        REQUIRE(std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME / "conf_4"));
    }
}

TEST_SUITE("commands::clean_all")
{
    TEST_CASE_FIXTURE(TestSetUp, "Basic functionality")
    {
        const auto result_1 = commands::clean_all({}, get_configurations(), path_to_project);

        CHECK(result_1 == EXIT_SUCCESS);
        REQUIRE(!std::filesystem::exists(path_to_project / params::BUILD_DIRECTORY_NAME));
        REQUIRE(!std::filesystem::exists(path_to_project / "conf_1.exe"));
        REQUIRE(!std::filesystem::exists(path_to_project / "dir_1" / "conf_2.exe"));
        REQUIRE(!std::filesystem::exists(path_to_project / "dir_1" / "conf_3.exe"));

        const auto result_2 = commands::clean_all({}, get_configurations(), path_to_project);
        CHECK(result_2 == EXIT_FAILURE); // Nothing is deleted.
    }
}

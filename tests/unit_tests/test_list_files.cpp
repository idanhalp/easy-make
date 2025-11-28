#include <string>

#include "third_party/doctest/doctest.hpp"

#include "source/argument_parsing/command_info.hpp"
#include "source/commands/list_files/list_files.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "tests/parameters.hpp"
#include "tests/unit_tests/utils/temporary_file_creator.hpp"
#include "tests/unit_tests/utils/utils.hpp" // `utils::get_path_to_resources_project`

using namespace std::literals;

TEST_SUITE("'list-files' command" * doctest::test_suite(test_type::unit))
{
    TEST_CASE_FIXTURE(TemporaryFileCreator, "No flags")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        // Note the space before the single-digit indices.
        const auto expected = "There are 20 files in the 'name' configuration:\n"
                              " 1 source/file_01.cpp\n"
                              " 2 source/file_01.hpp\n"
                              " 3 source/file_02.cpp\n"
                              " 4 source/file_02.hpp\n"
                              " 5 source/file_03.cpp\n"
                              " 6 source/file_03.hpp\n"
                              " 7 source/file_04.cpp\n"
                              " 8 source/file_04.hpp\n"
                              " 9 source/file_05.cpp\n"
                              "10 source/file_05.hpp\n"
                              "11 test/file_01.cpp\n"
                              "12 test/file_01.hpp\n"
                              "13 test/file_02.cpp\n"
                              "14 test/file_02.hpp\n"
                              "15 test/file_03.cpp\n"
                              "16 test/file_03.hpp\n"
                              "17 test/file_04.cpp\n"
                              "18 test/file_04.hpp\n"
                              "19 test/file_05.cpp\n"
                              "20 test/file_05.hpp\n"s;

        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "'--count' flag")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.count              = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        const auto expected = "20\n"s;
        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "'--count' and '--header-only' flags together")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.count              = true;
        info.header_only        = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        const auto expected = "10\n"s;
        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "'--header-only' flag")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.header_only        = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        // Note the space before the single-digit indices.
        const auto expected = "There are 10 header files in the 'name' configuration:\n"
                              " 1 source/file_01.hpp\n"
                              " 2 source/file_02.hpp\n"
                              " 3 source/file_03.hpp\n"
                              " 4 source/file_04.hpp\n"
                              " 5 source/file_05.hpp\n"
                              " 6 test/file_01.hpp\n"
                              " 7 test/file_02.hpp\n"
                              " 8 test/file_03.hpp\n"
                              " 9 test/file_04.hpp\n"
                              "10 test/file_05.hpp\n"s;

        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "'--porcelain' flag")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.porcelain_output   = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        const auto expected = "source/file_01.cpp\n"
                              "source/file_01.hpp\n"
                              "source/file_02.cpp\n"
                              "source/file_02.hpp\n"
                              "source/file_03.cpp\n"
                              "source/file_03.hpp\n"
                              "source/file_04.cpp\n"
                              "source/file_04.hpp\n"
                              "source/file_05.cpp\n"
                              "source/file_05.hpp\n"
                              "test/file_01.cpp\n"
                              "test/file_01.hpp\n"
                              "test/file_02.cpp\n"
                              "test/file_02.hpp\n"
                              "test/file_03.cpp\n"
                              "test/file_03.hpp\n"
                              "test/file_04.cpp\n"
                              "test/file_04.hpp\n"
                              "test/file_05.cpp\n"
                              "test/file_05.hpp\n"s;

        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "'--porcelain' and '--source-only' flags together")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.porcelain_output   = true;
        info.source_only        = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        const auto expected = "source/file_01.cpp\n"
                              "source/file_02.cpp\n"
                              "source/file_03.cpp\n"
                              "source/file_04.cpp\n"
                              "source/file_05.cpp\n"
                              "test/file_01.cpp\n"
                              "test/file_02.cpp\n"
                              "test/file_03.cpp\n"
                              "test/file_04.cpp\n"
                              "test/file_05.cpp\n"s;

        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "'--source-only' flag")
    {
        Configuration configuration{};
        configuration.name               = "name";
        configuration.compiler           = "g++";
        configuration.output_name        = "output";
        configuration.source_directories = {"."}; // Include everything

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.source_only        = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        // Note the space before the single-digit indices.
        const auto expected = "There are 10 source files in the 'name' configuration:\n"
                              " 1 source/file_01.cpp\n"
                              " 2 source/file_02.cpp\n"
                              " 3 source/file_03.cpp\n"
                              " 4 source/file_04.cpp\n"
                              " 5 source/file_05.cpp\n"
                              " 6 test/file_01.cpp\n"
                              " 7 test/file_02.cpp\n"
                              " 8 test/file_03.cpp\n"
                              " 9 test/file_04.cpp\n"
                              "10 test/file_05.cpp\n"s;

        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "No files in the configuration")
    {
        Configuration configuration{};
        configuration.name        = "name";
        configuration.compiler    = "g++";
        configuration.output_name = "output";

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        const auto expected = "There are 0 files in the 'name' configuration.\n"s;
        CHECK_EQ(get_content(), expected);
    }

    TEST_CASE_FIXTURE(TemporaryFileCreator, "One file in configuration")
    {
        Configuration configuration{};
        configuration.name         = "name";
        configuration.compiler     = "g++";
        configuration.output_name  = "output";
        configuration.source_files = {"source/file_01.cpp", "source/file_01.hpp"};

        ListFilesCommandInfo info{};
        info.configuration_name = *configuration.name;
        info.source_only        = true;

        const auto path_to_root = tests::utils::get_path_to_resources_project(25);

        const auto result = commands::list_files(info, {configuration}, path_to_root, get_ofstream());
        REQUIRE_EQ(result, EXIT_SUCCESS);

        const auto expected = "There is 1 source file in the 'name' configuration:\n"
                              "1 source/file_01.cpp\n"s;

        CHECK_EQ(get_content(), expected);
    }
}

#include <algorithm>
#include <filesystem>
#include <unordered_map>

#include "third_party/doctest/doctest.hpp"

#include "source/build_caching/build_caching.hpp"
#include "tests/utils/utils.hpp"

TEST_SUITE("build_caching")
{
    TEST_CASE("'hash_file_contents' works.")
    {
        const auto path_to_file_1 = tests::utils::get_path_to_resources_project(6) / "f_1.cpp";
        const auto path_to_file_2 = tests::utils::get_path_to_resources_project(6) / "f_2.cpp";
        const auto path_to_file_3 = tests::utils::get_path_to_resources_project(6) / "f_3.cpp";
        const auto path_to_file_4 = tests::utils::get_path_to_resources_project(6) / "f_4.cpp";

        const auto hash_1 = build_caching::hash_file_contents(path_to_file_1);
        const auto hash_2 = build_caching::hash_file_contents(path_to_file_2);
        const auto hash_3 = build_caching::hash_file_contents(path_to_file_3);
        const auto hash_4 = build_caching::hash_file_contents(path_to_file_4);

        CHECK_EQ(hash_1, hash_2);
        CHECK_NE(hash_1, hash_3);
        CHECK_NE(hash_1, hash_4);
        CHECK_NE(hash_3, hash_4);
    }

    TEST_CASE("'get_old_file_hashes' returns expected results for existing config.")
    {
        const auto path_to_project_7 = tests::utils::get_path_to_resources_project(7);
        const auto hashes            = build_caching::get_old_file_hashes("default", path_to_project_7);
        const std::unordered_map<std::filesystem::path, std::uint64_t> expected = {
            {"f_1.cpp", 1234}, {"f_2.cpp", 4321}, {"dir/f_3.cpp", 1357}};

        CHECK_EQ(hashes, expected);
    }

    TEST_CASE("'get_old_file_hashes' returns empty map when configuration does not exist.")
    {
        const auto path_to_project_7 = tests::utils::get_path_to_resources_project(7);
        const auto hashes            = build_caching::get_old_file_hashes("nonexistant", path_to_project_7);
        CHECK(hashes.empty());
    }

    TEST_CASE("'get_old_file_hashes' returns empty map for empty JSON.")
    {
        const auto path_to_project_7 = tests::utils::get_path_to_resources_project(7);
        const auto hashes            = build_caching::get_old_file_hashes("empty", path_to_project_7);
        CHECK(hashes.empty());
    }

    TEST_CASE("'get_files_to_delete' works correctly.")
    {
        const std::unordered_map<std::filesystem::path, std::uint64_t> old_file_hashes{
            {"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}};

        const std::unordered_map<std::filesystem::path, std::uint64_t> new_file_hashes{{"a", 1}, {"c", 3}, {"e", 5}};

        const auto files_to_delete = build_caching::get_files_to_delete(old_file_hashes, new_file_hashes);

        CHECK_EQ(files_to_delete.size(), 2);
        CHECK(std::ranges::contains(files_to_delete, "b"));
        CHECK(std::ranges::contains(files_to_delete, "d"));
    }

    TEST_CASE("'get_changed_files' works correctly.")
    {
        const std::unordered_map<std::filesystem::path, std::uint64_t> old_file_hashes{
            {"a.cpp", 1}, {"b.cpp", 2}, {"c.cpp", 3}, {"d.cpp", 4}, {"aa.cpp", 11}, {"bb.cpp", 22}, {"f.hpp", 4}};

        const std::unordered_map<std::filesystem::path, std::uint64_t> new_file_hashes{
            {"a.cpp", 1}, {"b.cpp", 2}, {"c.cpp", 4}, {"e.cpp", 5}, {"aa.cpp", 12}, {"bb.cpp", 22}, {"f.hpp", 4}};

        const auto path_to_project_8 = tests::utils::get_path_to_resources_project(8);
        const auto changed_files =
            build_caching::get_changed_files("conf", path_to_project_8, old_file_hashes, new_file_hashes);

        CHECK_EQ(changed_files.size(), 4);
        CHECK(std::ranges::contains(changed_files, "a.cpp"));
        CHECK_FALSE(std::ranges::contains(changed_files, "b.cpp"));
        CHECK(std::ranges::contains(changed_files, "c.cpp"));
        CHECK(std::ranges::contains(changed_files, "e.cpp"));
        CHECK(std::ranges::contains(changed_files, "aa.cpp"));
        CHECK_FALSE(std::ranges::contains(changed_files, "bb.cpp"));
        CHECK_FALSE(std::ranges::contains(changed_files, "f.hpp"));
    }
}

#include "tests/tests.hpp"

#include <algorithm>
#include <cassert>
#include <print>

#include "source/build_caching/build_caching.hpp"
#include "tests/utils/utils.hpp"

static auto test_hash_file_contents() -> void
{
    // Files 1 and 2 have the same contents.
    // File 3 uses `std::print` instead of `println`.
    // File 4 is empty.

    const auto path_to_file_1 = tests::utils::get_path_to_resources_project(6) / "f_1.cpp";
    const auto path_to_file_2 = tests::utils::get_path_to_resources_project(6) / "f_2.cpp";
    const auto path_to_file_3 = tests::utils::get_path_to_resources_project(6) / "f_3.cpp";
    const auto path_to_file_4 = tests::utils::get_path_to_resources_project(6) / "f_4.cpp";

    const auto hash_1 = build_caching::hash_file_contents(path_to_file_1);
    const auto hash_2 = build_caching::hash_file_contents(path_to_file_2);
    const auto hash_3 = build_caching::hash_file_contents(path_to_file_3);
    const auto hash_4 = build_caching::hash_file_contents(path_to_file_4);

    assert(hash_1 == hash_2);
    assert(hash_1 != hash_3);
    assert(hash_1 != hash_4);
    assert(hash_3 != hash_4);
}

static auto test_get_old_file_hashes_1() -> void
{
    const auto path_to_project_7 = tests::utils::get_path_to_resources_project(7);
    const auto hashes            = build_caching::get_old_file_hashes("default", path_to_project_7);
    const std::unordered_map<std::filesystem::path, std::uint64_t> expected = {
        {"f_1.cpp", 1234}, {"f_2.cpp", 4321}, {"dir/f_3.cpp", 1357}};

    assert(hashes == expected);
}

static auto test_get_old_file_hashes_2() -> void
{
    const auto path_to_project_7 = tests::utils::get_path_to_resources_project(7);
    const auto hashes            = build_caching::get_old_file_hashes("nonexistant", path_to_project_7);

    assert(hashes.empty()); // Configuration does not exist.
}

static auto test_get_old_file_hashes_3() -> void
{
    const auto path_to_project_7 = tests::utils::get_path_to_resources_project(7);
    const auto hashes            = build_caching::get_old_file_hashes("empty", path_to_project_7);

    assert(hashes.empty()); // JSON is an empty array
}

static auto test_get_files_to_delete() -> void
{
    const std::unordered_map<std::filesystem::path, std::uint64_t> old_file_hashes{
        {"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}};

    const std::unordered_map<std::filesystem::path, std::uint64_t> new_file_hashes{{"a", 1}, {"c", 3}, {"e", 5}};
    const auto files_to_delete = build_caching::get_files_to_delete(old_file_hashes, new_file_hashes);

    assert(files_to_delete.size() == 2);
    assert(std::ranges::contains(files_to_delete, "b"));
    assert(std::ranges::contains(files_to_delete, "d"));
}

static auto test_get_files_to_compile() -> void
{
    const std::unordered_map<std::filesystem::path, std::uint64_t> old_file_hashes{{"a", 1}, {"b", 2},   {"c", 3},
                                                                                   {"d", 4}, {"aa", 11}, {"bb", 22}};

    const std::unordered_map<std::filesystem::path, std::uint64_t> new_file_hashes{{"a", 1}, {"b", 2},   {"c", 4},
                                                                                   {"e", 5}, {"aa", 12}, {"bb", 22}};

    const auto path_to_project_8 = tests::utils::get_path_to_resources_project(8);
    const auto files_to_compile =
        build_caching::get_files_to_compile("conf", path_to_project_8, old_file_hashes, new_file_hashes);

    assert(files_to_compile.size() == 4);
    assert(std::ranges::contains(files_to_compile, "a"));   // Hash didn't change, no object file.
    assert(!std::ranges::contains(files_to_compile, "b"));  // Hash didn't change,  object file exists.
    assert(std::ranges::contains(files_to_compile, "c"));   // Hash changed, object file exists.
    assert(std::ranges::contains(files_to_compile, "e"));   // No previous hash, object file exists.
    assert(std::ranges::contains(files_to_compile, "aa"));  // Hash changed, object file exists.
    assert(!std::ranges::contains(files_to_compile, "bb")); // Hash didn't change,  object file exists.

    assert(std::ranges::is_sorted(files_to_compile));
}

auto tests::test_build_caching() -> void
{
    std::println("Running `build_caching` tests.");

    test_hash_file_contents();
    test_get_old_file_hashes_1();
    test_get_old_file_hashes_2();
    test_get_old_file_hashes_3();
    test_get_files_to_delete();
    test_get_files_to_compile();

    std::println("Done.");
}

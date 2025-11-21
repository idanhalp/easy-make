#include <string>

#include "third_party/doctest/doctest.hpp"

#include "source/utils/find_closest_word.hpp"

using namespace std::literals;

TEST_SUITE("find_closest_word")
{
    const auto candidates = {
        "release"s,
        "test"s,
        "easy-make"s,
    };

    TEST_CASE("Exact match")
    {
        const auto result = utils::find_closest_word("release", candidates);
        REQUIRE(result.has_value());
        CHECK_EQ(*result, "release");
    }

    TEST_CASE("Close match")
    {
        const auto result = utils::find_closest_word("relea", candidates);
        REQUIRE(result.has_value());
        CHECK_EQ(*result, "release");
    }

    TEST_CASE("Too far match")
    {
        const auto result = utils::find_closest_word("t", candidates);
        REQUIRE_FALSE(result.has_value());
    }

    TEST_CASE("No candidates")
    {
        const std::initializer_list<std::string> empty_candidates = {};
        const auto result = utils::find_closest_word("releas", empty_candidates);
        REQUIRE_FALSE(result.has_value());
    }
}

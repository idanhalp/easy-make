#include "third_party/doctest/doctest.hpp"

#include "source/utils/find_closest_word.hpp"

TEST_SUITE("find_closest_word")
{
    TEST_CASE("Exact match")
    {
        const auto result = utils::find_closest_word("release", {"release", "test", "easy-make"});
        REQUIRE(result.has_value());
        CHECK_EQ(*result, "release");
    }

    TEST_CASE("Close match")
    {
        const auto result = utils::find_closest_word("relea", {"release", "test", "easy-make"});
        REQUIRE(result.has_value());
        CHECK_EQ(*result, "release");
    }

    TEST_CASE("Too far match")
    {
        const auto result = utils::find_closest_word("t", {"release", "test", "easy-make"});
        REQUIRE_FALSE(result.has_value());
    }
}

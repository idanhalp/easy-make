#include "tests/tests.hpp"

#include <cassert>
#include <print>

#include "source/utils/find_closest_word.hpp"

static auto test_find_closest_word() -> void
{
    {
        // Exact match.
        const auto result = utils::find_closest_word("release", {"release", "test", "easy-make"});
        assert(result.has_value() && *result == "release");
    }

    {
        // Close match.
        const auto result = utils::find_closest_word("relea", {"release", "test", "easy-make"});
        assert(result.has_value() && *result == "release");
    }

    {
        // Too far.
        const auto result = utils::find_closest_word("t", {"release", "test", "easy-make"});
        assert(!result.has_value());
    }
}

auto tests::test_find_closest_word() -> void
{
    std::println("Running `find_closest_word` tests.");

    ::test_find_closest_word();

    std::println("Done.");
}

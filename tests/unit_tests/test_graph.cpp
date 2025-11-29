#include <algorithm>
#include <filesystem>

#include "third_party/doctest/doctest.hpp"

#include "source/utils/graph.hpp"
#include "tests/parameters.hpp"

TEST_SUITE("DirectedGraph class" * doctest::test_suite(test_type::unit))
{
    TEST_CASE("get_reachable_nodes")
    {
        SUBCASE("Empty initial nodes returns empty vector")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");
            graph.add_edge("b", "c");

            const auto reachable = graph.get_reachable_nodes({});

            CHECK(reachable.empty());
        }

        SUBCASE("Single node with no edges returns only that node")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");

            const auto reachable = graph.get_reachable_nodes({"c"});

            CHECK(reachable.size() == 1);
            CHECK(std::ranges::contains(reachable, "c"));
        }

        SUBCASE("Linear chain of nodes")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("c", "d");

            const auto reachable = graph.get_reachable_nodes({"a"});

            CHECK(reachable.size() == 4);
            CHECK(std::ranges::contains(reachable, "a"));
            CHECK(std::ranges::contains(reachable, "b"));
            CHECK(std::ranges::contains(reachable, "c"));
            CHECK(std::ranges::contains(reachable, "d"));
        }

        SUBCASE("Branching graph")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");
            graph.add_edge("a", "c");
            graph.add_edge("b", "d");
            graph.add_edge("c", "d");

            const auto reachable = graph.get_reachable_nodes({"a"});

            CHECK(reachable.size() == 4);
            CHECK(std::ranges::contains(reachable, "a"));
            CHECK(std::ranges::contains(reachable, "b"));
            CHECK(std::ranges::contains(reachable, "c"));
            CHECK(std::ranges::contains(reachable, "d"));
        }

        SUBCASE("Graph with unreachable nodes")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("d", "e");
            graph.add_edge("e", "f");

            const auto reachable = graph.get_reachable_nodes({"a"});

            CHECK(reachable.size() == 3);
            CHECK(std::ranges::contains(reachable, "a"));
            CHECK(std::ranges::contains(reachable, "b"));
            CHECK(std::ranges::contains(reachable, "c"));
            CHECK_FALSE(std::ranges::contains(reachable, "d"));
            CHECK_FALSE(std::ranges::contains(reachable, "e"));
            CHECK_FALSE(std::ranges::contains(reachable, "f"));
        }

        SUBCASE("Multiple initial nodes")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");
            graph.add_edge("c", "d");
            graph.add_edge("b", "e");
            graph.add_edge("d", "e");

            const auto reachable = graph.get_reachable_nodes({"a", "c"});

            CHECK(reachable.size() == 5);
            CHECK(std::ranges::contains(reachable, "a"));
            CHECK(std::ranges::contains(reachable, "b"));
            CHECK(std::ranges::contains(reachable, "c"));
            CHECK(std::ranges::contains(reachable, "d"));
            CHECK(std::ranges::contains(reachable, "e"));
        }

        SUBCASE("Graph with cycle")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("c", "a");
            graph.add_edge("b", "d");

            const auto reachable = graph.get_reachable_nodes({"a"});

            CHECK(reachable.size() == 4);
            CHECK(std::ranges::contains(reachable, "a"));
            CHECK(std::ranges::contains(reachable, "b"));
            CHECK(std::ranges::contains(reachable, "c"));
            CHECK(std::ranges::contains(reachable, "d"));
        }

        SUBCASE("Node pointing to itself")
        {
            utils::DirectedGraph<std::string> graph;
            graph.add_edge("a", "a");
            graph.add_edge("a", "b");

            const auto reachable = graph.get_reachable_nodes({"a"});

            CHECK(reachable.size() == 2);
            CHECK(std::ranges::contains(reachable, "a"));
            CHECK(std::ranges::contains(reachable, "b"));
        }
    }

    TEST_CASE("check_for_cycle")
    {
        SUBCASE("Simple cycle")
        {
            utils::DirectedGraph<std::filesystem::path> graph;

            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("c", "a");

            const auto cycle = graph.check_for_cycle();

            REQUIRE(cycle.has_value());
            CHECK_EQ(*cycle, "a -> b -> c -> a");
        }

        SUBCASE("No cycle")
        {
            utils::DirectedGraph<std::filesystem::path> graph;

            graph.add_edge("a", "b");
            graph.add_edge("b", "c");

            const auto cycle = graph.check_for_cycle();

            REQUIRE(!cycle.has_value());
        }

        SUBCASE("Not so simple cycle")
        {
            utils::DirectedGraph<std::filesystem::path> graph;

            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("b", "d");
            graph.add_edge("d", "a");

            const auto cycle = graph.check_for_cycle();

            REQUIRE(cycle.has_value());
            CHECK_EQ(*cycle, "a -> b -> d -> a");
        }

        SUBCASE("Node points to itself")
        {
            utils::DirectedGraph<std::filesystem::path> graph;

            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("b", "d");
            graph.add_edge("b", "b");

            const auto cycle = graph.check_for_cycle();

            REQUIRE(cycle.has_value());
            CHECK_EQ(*cycle, "b -> b");
        }

        SUBCASE("Two cycles.")
        {
            utils::DirectedGraph<std::filesystem::path> graph;

            graph.add_edge("a", "b");
            graph.add_edge("b", "c");
            graph.add_edge("c", "d");
            graph.add_edge("d", "b");
            graph.add_edge("c", "a");

            const auto cycle = graph.check_for_cycle();

            REQUIRE(cycle.has_value());

            const auto expected_cycle_found = (*cycle == "b -> c -> d") || (*cycle == "a -> b -> c -> a");
            CHECK(expected_cycle_found);
        }

        SUBCASE("Cycle with a tail")
        {
            utils::DirectedGraph<std::filesystem::path> graph;

            // This order is important because it causes "a" to precede "b" in the iteration order,
            // But "a" is not part of the cycle.
            graph.add_edge("b", "c");
            graph.add_edge("a", "b");
            graph.add_edge("c", "b");

            const auto cycle = graph.check_for_cycle();

            REQUIRE(cycle.has_value());
            CHECK_EQ(*cycle, "b -> c -> b");
        }
    }
}

#include "third_party/doctest/doctest.hpp"

#include "source/build_caching/dependency_graph.hpp"
#include "tests/utils/utils.hpp"

TEST_SUITE("Dependency graph")
{
    TEST_CASE("build_caching::get_included_files")
    {
        {
            const auto path_to_project_18 = tests::utils::get_path_to_resources_project(18);
            const auto path_to_f_1        = path_to_project_18 / "f_1.cpp";
            const auto includes           = build_caching::get_included_files(path_to_f_1);
            const std::vector<std::filesystem::path> expected{"f2.h", "f3.hpp", "f4.hh"};

            CHECK_EQ(includes, expected);
        }

        {
            const auto path_to_project_18        = tests::utils::get_path_to_resources_project(18);
            const auto path_to_non_existent_file = path_to_project_18 / "f_2.cpp";
            const auto includes                  = build_caching::get_included_files(path_to_non_existent_file);

            CHECK_EQ(includes, std::vector<std::filesystem::path>{});
        }
    }

    TEST_CASE("build_caching::get_dependency_graph")
    {
        const auto path_to_project_19 = tests::utils::get_path_to_resources_project(19);
        const auto f_1_cpp            = std::filesystem::path("f_1.cpp");
        const auto f_1_hpp            = std::filesystem::path("f_1.hpp");
        const auto f_2_hpp            = std::filesystem::path("f_2.hpp");
        const auto f_3_cpp            = std::filesystem::path("f_3.cpp");
        const auto f_3_hpp            = std::filesystem::path("f_3.hpp");
        const auto files              = {f_1_cpp, f_1_hpp, f_2_hpp, f_3_cpp, f_3_hpp};

        utils::DirectedGraph<std::filesystem::path> expected;
        expected.add_edge(f_1_hpp, f_1_cpp);
        expected.add_edge(f_2_hpp, f_1_hpp);
        expected.add_edge(f_3_hpp, f_3_cpp);

        const auto graph = build_caching::get_dependency_graph(path_to_project_19, files);

        CHECK_EQ(graph, expected);
    }
}

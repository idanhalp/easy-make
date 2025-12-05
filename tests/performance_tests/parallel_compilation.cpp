#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>

#include "third_party/doctest/doctest.hpp"

#include "source/argument_parsing/command_info.hpp"
#include "source/commands/build/compilation.hpp"
#include "source/configuration_parsing/configuration.hpp"
#include "source/parameters/parameters.hpp"

namespace
{
    class ProjectBuilder
    {
      public:
        ProjectBuilder();
        ~ProjectBuilder();

      private:
        auto create_file_content(int index) -> std::string;

        const std::filesystem::path old_path;
        const std::filesystem::path new_path;
    };
}

static const auto NUM_OF_FILES = 35;

// Build the project.
ProjectBuilder::ProjectBuilder()
    : old_path(std::filesystem::current_path()), new_path(old_path / "tests" / "performance_tests" / "resources")
{
    std::filesystem::create_directories(new_path);
    std::filesystem::current_path(new_path);

    for (auto index = 1; index <= NUM_OF_FILES; ++index)
    {
        const auto filename = std::format("file_{:02}.cpp", index);
        auto file           = std::ofstream(new_path / filename);
        file << create_file_content(index);
    }
}

// Remove the project
ProjectBuilder::~ProjectBuilder()
{
    std::filesystem::current_path(old_path);
    std::filesystem::remove_all(new_path);
}

auto ProjectBuilder::create_file_content(const int index) -> std::string
{
    constexpr auto content = R"(   
    #include <vector>
    #include <map>
    #include <string>
    #include <set>
    #include <unordered_map>
    #include <list>
    #include <deque>
    #include <algorithm>
    #include <numeric>
    #include <memory>
    
    int foo_{:02}() {{
        [[maybe_unused]]std::vector<int> v1;
        [[maybe_unused]]std::vector<double> v2;
        [[maybe_unused]]std::vector<std::string> v3;
        [[maybe_unused]]std::map<int, std::string> m1;
        [[maybe_unused]]std::map<std::string, double> m2;
        [[maybe_unused]]std::unordered_map<int, std::vector<std::string>> um1;
        [[maybe_unused]]std::set<int> s1;
        [[maybe_unused]]std::list<double> l1;
        [[maybe_unused]]std::deque<std::string> d1;
        [[maybe_unused]]std::shared_ptr<std::vector<int>> sp1;
        
        return 0;
    }})";

    return std::format(content, index);
}

static auto get_average_compilation_duration(const bool use_parallel_compilation) -> std::int64_t
{
    std::vector<std::filesystem::path> files;

    for (auto index = 1; index <= NUM_OF_FILES; ++index)
    {
        files.push_back(std::format("file_{:02}.cpp", index));
    }

    std::ranges::sort(files);

    Configuration configuration;
    configuration.name         = "config";
    configuration.compiler     = "g++";
    configuration.output_name  = "output.exe";
    configuration.optimization = "3";

    const auto num_of_warm_up_runs  = 1;
    const auto num_of_measured_runs = 3;
    auto total                      = 0LL;

    for (auto i = 1; i <= (num_of_warm_up_runs + num_of_measured_runs); ++i)
    {
        std::filesystem::remove_all(params::BUILD_DIRECTORY_NAME);
        std::filesystem::create_directories(params::BUILD_DIRECTORY_NAME / "config");

        const auto start_time = std::chrono::high_resolution_clock::now();
        compile_files(configuration, std::filesystem::current_path(), files, true, use_parallel_compilation);
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto runtime  = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        const auto should_measure_run = i > num_of_warm_up_runs;

        if (should_measure_run)
        {
            total += runtime;
        }
    }

    return total / num_of_measured_runs;
}

static auto get_average_sequential_compilation_duration() -> std::int64_t
{
    return get_average_compilation_duration(false);
}

static auto get_average_parallel_compilation_duration() -> std::int64_t
{
    return get_average_compilation_duration(true);
}

TEST_CASE_FIXTURE(ProjectBuilder, "Make sure parallel compilation is faster [performance]")
{
    const auto average_sequential_compilation_duration = get_average_sequential_compilation_duration();
    const auto average_parallel_compilation_duration   = get_average_parallel_compilation_duration();
    CHECK_GE(average_sequential_compilation_duration, average_parallel_compilation_duration);
}

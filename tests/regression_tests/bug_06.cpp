#include <filesystem>
#include <fstream>

#include "third_party/doctest/doctest.hpp"
#include "third_party/nlohmann/json.hpp"

#include "source/commands/build/build.hpp"
#include "source/configuration_parsing/configuration_parsing.hpp"
#include "source/parameters/parameters.hpp"
#include "tests/parameters.hpp"
#include "tests/regression_tests/utils/test_environment_guard.hpp"

namespace
{
    class ConfigurationFileCreator
    {
      public:
        ConfigurationFileCreator() = default;
        ~ConfigurationFileCreator();

        auto write_to_configuration_file(const nlohmann::json& json) -> void;
    };
}

ConfigurationFileCreator::~ConfigurationFileCreator()
{
    std::filesystem::remove(params::CONFIGURATIONS_FILE_NAME);
}

auto ConfigurationFileCreator::write_to_configuration_file(const nlohmann::json& json) -> void
{
    auto file = std::ofstream(params::CONFIGURATIONS_FILE_NAME, std::ios::trunc);
    file << json.dump();
    file.flush();
}

TEST_CASE_FIXTURE(TestEnvironmentGuard<6>, "Change in configuration file [regression]")
{
    using namespace nlohmann::json_literals;

    const auto root_path = std::filesystem::current_path();
    const auto info      = BuildCommandInfo{
             .configuration_name       = "config",
             .is_quiet                 = true,
             .use_parallel_compilation = false,
    };
    auto json = R"(
        [
            {
                "compiler": "g++",
                "name": "config",
                "output": { 
                    "name": "output.exe" 
                },
                "sources": { 
                    "files": ["main.cpp", "f_1.cpp", "f_2.cpp"] 
                }
            }
        ])"_json;

    // There are currently 3 files included: `main.cpp`, `f_1.cpp` and `f_2.cpp`.
    // all of them should be compiled.
    ConfigurationFileCreator configuration_file_creator{};
    configuration_file_creator.write_to_configuration_file(json);

    const auto configurations_1 = parse_configurations(root_path);
    REQUIRE(configurations_1.has_value());
    REQUIRE_EQ(commands::build(info, *configurations_1, root_path).num_of_files_compiled, 3);

    // Make a critical change (change optimization level) that will force all the files to be recompiled.
    json[0]["optimization"] = "2";
    configuration_file_creator.write_to_configuration_file(json);

    const auto configurations_2 = parse_configurations(root_path);
    REQUIRE(configurations_2.has_value());
    REQUIRE_EQ(commands::build(info, *configurations_2, root_path).num_of_files_compiled, 3);

    // Make a non-critical change (add a new file), which should cause only the new file to be compiled.
    json[0]["sources"]["files"].push_back("f_3.cpp");
    configuration_file_creator.write_to_configuration_file(json);

    const auto configurations_3 = parse_configurations(root_path);
    REQUIRE(configurations_3.has_value());
    REQUIRE_EQ(commands::build(info, *configurations_3, root_path).num_of_files_compiled, 1);
}

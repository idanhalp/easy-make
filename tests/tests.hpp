#ifndef TESTS_TESTS_HPP
#define TESTS_TESTS_HPP

#include <string>

namespace tests
{
    auto test_utils() -> void;
    auto test_find_closest_word() -> void;
    auto test_argument_parsing() -> void;
    auto test_check_if_configurations_file_exists() -> void;
    auto test_configuration_parsing() -> void;
    auto test_executable_creation() -> void;
    auto test_build_caching() -> void;
}

#endif // TESTS_TESTS_HPP

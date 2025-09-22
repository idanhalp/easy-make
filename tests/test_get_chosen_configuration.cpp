#include "tests/tests.hpp"

#include <cassert>
#include <print>

#include "source/utils/utils.hpp"

auto tests::test_get_chosen_configuration() -> void
{
    std::println("Running `check_if_configurations_file_exists` tests.");

	assert(::utils::get_chosen_configuration({"./easy-make", "release"}) == "release");
	assert(::utils::get_chosen_configuration({"./easy-make", "debug"}) == "debug");
	assert(::utils::get_chosen_configuration({"./easy-make"}) == "default");

    std::println("Done.");
}

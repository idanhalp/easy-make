#include "source/commands/print_version/print_version.hpp"

#include <print>

#include "source/parameters/version.hpp"

auto commands::print_version([[maybe_unused]] const PrintVersionCommandInfo& info) -> int
{
    // Currently `info` is unused.
    // Adding it as a parameter enables us to easily
    // add arguments and flags in the future.

    std::println("Current easy-make version: {}", params::VERSION);

    return EXIT_SUCCESS;
}

#include "source/commands/print_version/print_version.hpp"

#include <print>

#include "source/parameters/version.hpp"

auto commands::print_version() -> int
{
    std::println("Current easy-make version: {}", params::VERSION);

    return EXIT_SUCCESS;
}

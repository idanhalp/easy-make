#include "source/commands/print_version/print_version.hpp"

#include <format>
#include <print>
#include <string>

namespace
{
    struct VersionInfo
    {
        int major;
        int minor;
        int patch;

        auto to_string() const -> std::string
        {
            return std::format("{}.{}.{}", major, minor, patch);
        }
    };
}

auto commands::print_version([[maybe_unused]] const PrintVersionCommandInfo& info) -> int
{
    // Currently `info` is unused.
    // Adding it as a parameter enables us to easily
    // add arguments and flags in the future.

    const auto current_version = VersionInfo{
        .major = 1,
        .minor = 0,
        .patch = 0,
    };

    std::println("Current easy-make version: {}", current_version.to_string());

    return EXIT_SUCCESS;
}

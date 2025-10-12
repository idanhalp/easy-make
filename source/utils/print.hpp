#ifndef SOURCE_UTILS_PRINT_HPP
#define SOURCE_UTILS_PRINT_HPP

#include <print>

namespace utils
{
    // Prints text in red.
    template <class... Args> auto print_error(std::format_string<Args...> fmt, Args&&... args) -> void
    {
        constexpr auto ANSI_red   = "\033[31m";
        constexpr auto ANSI_reset = "\033[0m";

        std::println("{}{}{}", ANSI_red, std::format(fmt, std::forward<Args>(args)...), ANSI_reset);
    }

    // Prints text in green.
    template <class... Args> auto print_success(std::format_string<Args...> fmt, Args&&... args) -> void
    {
        constexpr auto ANSI_green = "\033[32m";
        constexpr auto ANSI_reset = "\033[0m";

        std::println("{}{}{}", ANSI_green, std::format(fmt, std::forward<Args>(args)...), ANSI_reset);
    }
}

#endif // SOURCE_UTILS_PRINT_HPP

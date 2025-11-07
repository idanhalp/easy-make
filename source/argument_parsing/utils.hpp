#ifndef SOURCE_ARGUMENT_PARSING_UTILS_HPP
#define SOURCE_ARGUMENT_PARSING_UTILS_HPP

#include <optional>
#include <span>
#include <string_view>

namespace utils
{
    auto is_flag(std::string_view argument) -> bool;

    auto check_for_duplicate_flags(std::span<const char* const> arguments) -> std::optional<std::string_view>;
}

#endif // SOURCE_ARGUMENT_PARSING_UTILS_HPP

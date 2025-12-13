#include "source/argument_parsing/utils.hpp"

#include <unordered_set>

auto utils::is_flag(const std::string_view argument) -> bool
{
    return argument.starts_with("--");
}

auto utils::check_for_duplicate_flags(const std::span<const char* const> arguments) -> std::optional<std::string_view>
{
    std::unordered_set<std::string_view> seen;

    for (const std::string_view argument : arguments)
    {
        const auto found_duplicate_flag = utils::is_flag(argument) && !seen.insert(argument).second;

        if (found_duplicate_flag)
        {
            return argument;
        }
    }

    return std::nullopt;
}

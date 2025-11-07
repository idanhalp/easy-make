#include "source/argument_parsing/utils.hpp"

#include <unordered_set>

auto utils::is_flag(const std::string_view argument) -> bool
{
    return argument.starts_with("--");
}

auto utils::check_for_duplicate_flags(const std::span<const char* const> arguments) -> std::optional<std::string_view>
{
    std::unordered_set<std::string_view> seen;

    for (const std::string_view flag : arguments)
    {
        const auto flag_already_seen = !seen.insert(flag).second;

        if (flag_already_seen)
        {
            return flag;
        }
    }

    return std::nullopt;
}

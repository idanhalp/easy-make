#ifndef SOURCE_UTILS_FIND_CLOSEST_WORD_HPP
#define SOURCE_UTILS_FIND_CLOSEST_WORD_HPP

#include <optional>
#include <span>
#include <string>
#include <string_view>

namespace utils
{
    auto find_closest_word(std::string_view target_word,
                           std::span<const std::string> candidates) -> std::optional<std::string>;

    auto find_closest_word(std::string_view target_word,
                           std::span<const std::string_view> candidates) -> std::optional<std::string>;
}

#endif // SOURCE_UTILS_FIND_CLOSEST_WORD_HPP

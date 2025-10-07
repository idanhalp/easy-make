#ifndef SOURCE_UTILS_FIND_CLOSEST_WORD_HPP
#define SOURCE_UTILS_FIND_CLOSEST_WORD_HPP

#include <optional>
#include <string>
#include <vector>

namespace utils
{
    auto find_closest_word(const std::string& word,
                           const std::vector<std::string>& candidates) -> std::optional<std::string>;
}

#endif // SOURCE_UTILS_FIND_CLOSEST_WORD_HPP

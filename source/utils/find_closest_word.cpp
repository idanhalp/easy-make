#include "source/utils/find_closest_word.hpp"

#include <algorithm>
#include <limits>
#include <ranges>

static auto get_weighted_distance_between_words(const std::string& word_1,
                                                const std::string& word_2,
                                                const int swap_penalty,
                                                const int substitution_penalty,
                                                const int insertion_penalty,
                                                const int deletion_penalty) -> int
{
    // We want that the auxiliary arrays to use O(min(`word_1.size()`, `word_2.size()`) space.
    // Since we iterate over `word_1` in the outer loop and `word_2` in the inner one,
    // the algorithm uses O(`word_2.size()`) space.
    // Therefore swap `word_1` and `word_2` if `word_2` is longer.
    if (word_1.size() < word_2.size())
    {
        return get_weighted_distance_between_words(word_2, word_1, swap_penalty, substitution_penalty,
                                                   insertion_penalty, deletion_penalty);
    }

    std::vector<int> before_previous(word_2.size() + 1);
    std::vector<int> previous(word_2.size() + 1);
    std::vector<int> current(word_2.size() + 1);

    for (auto index = 0UZ; index < previous.size(); ++index)
    {
        previous[index] = insertion_penalty * index;
    }

    for (auto i = 0UZ; i < word_1.size(); ++i)
    {
        current[0] = deletion_penalty * (i + 1);

        for (auto j = 0UZ; j < word_2.size(); ++j)
        {
            const auto cost_after_substitution = previous[j] + ((word_1[i] == word_2[j]) ? 0 : substitution_penalty);
            const auto cost_after_deletion     = previous[j + 1] + deletion_penalty;
            const auto cost_after_insertion    = current[j] + insertion_penalty;

            const auto swap_is_possible =
                (i > 0) && (j > 0) && (word_1[i - 1] == word_2[j]) && (word_1[i] == word_2[j - 1]);

            const auto cost_after_swap =
                swap_is_possible ? (before_previous[j - 1] + swap_penalty) : std::numeric_limits<int>::max();

            current[j + 1] =
                std::min({cost_after_substitution, cost_after_deletion, cost_after_insertion, cost_after_swap});
        }

        std::swap(before_previous, previous);
        std::swap(previous, current);
    }

    return previous[word_2.size()];
}

static auto get_distance_between_words(const std::string& word_1, const std::string& word_2) -> int
{
    // Git-style penalty values for weighted Damerau–Levenshtein distance.
    const auto swap_penalty         = 0;
    const auto substitution_penalty = 2;
    const auto insertion_penalty    = 1;
    const auto deletion_penalty     = 3;

    return get_weighted_distance_between_words(word_1, word_2, swap_penalty, substitution_penalty, insertion_penalty,
                                               deletion_penalty);
}

auto utils::find_closest_word(const std::string& target_word,
                              const std::vector<std::string>& candidates) -> std::optional<std::string>
{
    const auto distance_from_target = [&](const std::string& s) { return get_distance_between_words(s, target_word); };
    const auto closest_word         = std::ranges::min_element(candidates, std::ranges::less{}, distance_from_target);

    const auto MAX_SIMILARITY_DIFFERENCE = 7;
    const auto candidate_is_close_enough = distance_from_target(*closest_word) <= MAX_SIMILARITY_DIFFERENCE;

    if (candidate_is_close_enough)
    {
        return *closest_word;
    }
    else
    {
        return std::nullopt;
    }
}

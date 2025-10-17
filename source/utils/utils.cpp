#include "source/utils/utils.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <string_view>

#include "source/parameters/parameters.hpp"

auto utils::check_if_configurations_file_exists(const std::filesystem::path& path) -> bool
{
    const auto path_is_valid = std::filesystem::exists(path) && std::filesystem::is_directory(path);

    if (!path_is_valid)
    {
        return false;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        const auto is_configurations_file =
            entry.is_regular_file() && entry.path().filename() == params::CONFIGURATIONS_FILE_NAME;

        if (is_configurations_file)
        {
            return true;
        }
    }

    return false;
}

auto utils::get_object_file_name(const std::filesystem::path& path) -> std::string
{
    auto result = std::format("{}.o", path.string());
    std::ranges::replace(result, std::filesystem::path::preferred_separator, '-');

    return result;
}

auto utils::get_ordinal_indicator(int index) -> const char*
{
    // Special cases.
    if (index == 11 || index == 12 || index == 13)
    {
        return "th";
    }

    switch (index % 10)
    {
    case 1:
        return "st";

    case 2:
        return "nd";

    case 3:
        return "rd";

    default:
        return "th";
    }
}

auto utils::is_header_file(const std::filesystem::path& path) -> bool
{
    const auto extension = path.extension();

    return extension == ".h" || extension == ".hpp" || extension == ".hh" || extension == ".hxx";
}

auto utils::is_source_file(const std::filesystem::path& path) -> bool
{
    const auto extension = path.extension();

    return extension == ".cpp" || extension == ".cc" || extension == ".cxx";
}

auto utils::is_code_file(const std::filesystem::path& path) -> bool
{
    return is_header_file(path) || is_source_file(path);
}

auto utils::count_digits(const int x) -> int
{
    if (x == 0)
    {
        return 1;
    }
    else
    {
        return static_cast<int>(std::log10(std::abs(1.0 * x))) + 1;
    }
}

#include "source/utils/utils.hpp"

#include <algorithm>
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

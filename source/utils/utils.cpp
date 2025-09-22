#include "source/utils/utils.hpp"

#include <filesystem>
#include <string_view>

auto utils::check_if_configurations_file_exists(const std::string_view path) -> bool
{
    const auto path_is_valid = std::filesystem::exists(path) && std::filesystem::is_directory(path);

    if (!path_is_valid)
    {
        return false;
    }

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        const auto is_configurations_file =
            entry.is_regular_file() && entry.path().filename() == CONFIGURATIONS_FILE_NAME;

        if (is_configurations_file)
        {
            return true;
        }
    }

    return false;
}

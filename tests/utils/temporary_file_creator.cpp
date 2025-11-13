#include "tests/utils/temporary_file_creator.hpp"

#include <format>

static auto get_unique_file_name() -> std::filesystem::path
{
    for (auto index = 0;; ++index)
    {
        const auto file_name = std::format("temporary-file-creator_{}.txt", index);
        const auto path      = std::filesystem::temp_directory_path() / file_name;

        if (!std::filesystem::exists(path))
        {
            return path;
        }
    }
}

TemporaryFileCreator::TemporaryFileCreator() : path(get_unique_file_name()), ofstream(path)
{
}

TemporaryFileCreator::~TemporaryFileCreator()
{
    std::filesystem::remove(path);
}

auto TemporaryFileCreator::get_ofstream() -> std::ostream&
{
    return ofstream;
}

auto TemporaryFileCreator::get_content() -> std::string
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error(std::format("Failed to open file: {}", path.native()));
    }

    ofstream.flush();

    std::ostringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

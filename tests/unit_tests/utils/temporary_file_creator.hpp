#ifndef TESTS_UTILS_TEMPORARY_FILE_CREATOR_HPP
#define TESTS_UTILS_TEMPORARY_FILE_CREATOR_HPP

#include <filesystem>
#include <fstream>
#include <string>

class TemporaryFileCreator
{
  public:
    // Called before each test case.
    TemporaryFileCreator();

    // Called after each test case.
    ~TemporaryFileCreator();

    auto get_ofstream() -> std::ostream&;

    auto get_content() -> std::string;

  private:
    const std::filesystem::path path;
    std::ofstream ofstream;
};

#endif // TESTS_UTILS_TEMPORARY_FILE_CREATOR_HPP

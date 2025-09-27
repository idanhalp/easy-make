#ifndef SOURCE_ARGUMENT_PARSING_ARGUMENT_PARSING_HPP
#define SOURCE_ARGUMENT_PARSING_ARGUMENT_PARSING_HPP

#include <expected>
#include <span>
#include <string>

struct ArgumentInfo
{
    ArgumentInfo();

    std::string configuration_name;
    bool clean_object_files;
    bool print_version;
};

auto parse_arguments(std::span<const char* const> arguments) -> std::expected<ArgumentInfo, std::string>;

#endif // SOURCE_ARGUMENT_PARSING_ARGUMENT_PARSING_HPP

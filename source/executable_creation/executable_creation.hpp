#ifndef SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP
#define SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP

#include <optional>
#include <string_view>
#include <vector>

#include "source/configuration_parsing/configuration_parsing.hpp"

// Returns a message in case of a failure.
auto create_executable(std::string_view configuration_name,
                       const std::vector<Configuration> &configurations) -> std::optional<std::string>;

#endif // SOURCE_EXECUTABLE_CREATION_EXECUTABLE_CREATION_HPP

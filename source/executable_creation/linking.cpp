#include "source/executable_creation/linking.hpp"

#include <cstdlib>
#include <format>

#include "source/parameters/parameters.hpp"
#include "source/utils/print.hpp"

auto link_object_files(const Configuration& configuration, const std::filesystem::path& path_to_root) -> bool
{
    const auto object_files_path = path_to_root / params::BUILD_DIRECTORY_NAME / *configuration.name;
    const auto output_file_path =
        std::filesystem::path(configuration.output_path.value_or(".")) / *configuration.output_name;

    if (configuration.output_path.has_value())
    {
        std::filesystem::create_directories(*configuration.output_path);
    }

    const auto link_command =
        std::format("{} {}/*.o -o {}", *configuration.compiler, object_files_path.native(), output_file_path.native());

    std::println("Linking...");

    const auto linking_successful = std::system(link_command.c_str()) == EXIT_SUCCESS;

    if (linking_successful)
    {
        utils::print_success("Linking complete. Executable located at '{}'.", output_file_path.native());
    }
    else
    {
        utils::print_error("Linking failed.");
    }

    return linking_successful;
}

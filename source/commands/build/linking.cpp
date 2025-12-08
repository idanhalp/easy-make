#include "source/commands/build/linking.hpp"

#include <cstdlib>
#include <format>
#include <ranges>
#include <string_view>

#include "source/parameters/parameters.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/print.hpp"

using namespace std::literals;

static auto print_linking_result(const bool linking_successful, const std::string_view output_path) -> void
{
    if (linking_successful)
    {
        utils::print_success("Linking complete. Executable located at '{}'.", output_path);
    }
    else
    {
        utils::print_error("Linking failed.");
    }
}

auto link_object_files(const Configuration& configuration,
                       const std::filesystem::path& path_to_root,
                       const std::vector<std::string>& flags,
                       const bool is_quiet) -> bool
{
    ASSERT(configuration.name.has_value());
    ASSERT(configuration.compiler.has_value());
    ASSERT(configuration.output_name.has_value());

    const auto object_files_path = (path_to_root / params::BUILD_DIRECTORY_NAME / *configuration.name).string();
    const auto output_path =
        (std::filesystem::path(configuration.output_path.value_or(".")) / *configuration.output_name).string();

    if (configuration.output_path.has_value())
    {
        std::filesystem::create_directories(*configuration.output_path);
    }

    if (!is_quiet)
    {
        std::println("Linking...");
    }

    const auto flag_string = flags | std::views::join_with(" "sv) | std::ranges::to<std::string>();
    const auto link_command =
        std::format("{} {} {}/*.o -o {}", *configuration.compiler, flag_string, object_files_path, output_path);

    const auto linking_successful = std::system(link_command.c_str()) == EXIT_SUCCESS;

    if (!is_quiet)
    {
        print_linking_result(linking_successful, output_path);
    }

    return linking_successful;
}

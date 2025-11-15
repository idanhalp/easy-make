#include "source/commands/list_files/list_files.hpp"

#include <algorithm>
#include <cstdlib> // `EXIT_SUCCESS`, `EXIT_FAILURE`
#include <print>
#include <ranges>
#include <vector>

#include "source/commands/build/build.hpp"
#include "source/utils/macros/assert.hpp"
#include "source/utils/print.hpp"
#include "source/utils/utils.hpp"

static auto print_porcelain_output(const std::vector<std::filesystem::path>& files, std::ostream& output) -> void
{
    for (const auto& file : files)
    {
        std::println(output, "{}", file.native());
    }
}

static auto print_verbose_output(const ListFilesCommandInfo& info,
                                 const std::vector<std::filesystem::path>& files,
                                 std::ostream& output) -> void
{
    // If both flags are set then all files are irrelevant.
    ASSERT(!info.header_only || !info.source_only);

    const auto description = info.header_only ? "header " : info.source_only ? "source " : "";

    switch (files.size())
    {
    case 0:
        std::println(output, "There are 0 {}files in the '{}' configuration.", description, info.configuration_name);
        break;

    case 1:
        std::println(output, "There is 1 {}file in the '{}' configuration:", description, info.configuration_name);
        break;

    default:
        std::println(output,
                     "There are {} {}files in the '{}' configuration:",
                     files.size(),
                     description,
                     info.configuration_name);
        break;
    }

    const auto max_index_width = utils::count_digits(files.size()); // For formatting.

    for (const auto [index, name] : std::views::enumerate(files) | std::views::as_const)
    {
        std::println(output, "{0:>{2}} {1}", index + 1, name.native(), max_index_width);
    }
}

static auto get_relevant_files(const ListFilesCommandInfo& info,
                               const Configuration& configuration,
                               const std::filesystem::path& path_to_root) -> std::vector<std::filesystem::path>
{
    const auto files = get_code_files(configuration, path_to_root);

    if (info.header_only)
    {
        return files | std::views::filter(&utils::is_header_file) | std::ranges::to<std::vector>();
    }
    if (info.source_only)
    {
        return files | std::views::filter(&utils::is_source_file) | std::ranges::to<std::vector>();
    }
    else
    {
        return files;
    }
}

auto commands::list_files(const ListFilesCommandInfo& info,
                          const std::vector<Configuration>& configurations,
                          const std::filesystem::path& path_to_root,
                          std::ostream& output) -> int
{
    const auto configuration                  = get_actual_configuration(info.configuration_name, configurations);
    const auto found_error_with_configuration = !configuration.has_value();

    if (found_error_with_configuration)
    {
        utils::print_error(output, "{}", configuration.error());

        return EXIT_FAILURE;
    }

    auto files = get_relevant_files(info, *configuration, path_to_root);
    std::ranges::sort(files);

    if (info.count)
    {
        std::println(output, "{}", files.size());
    }
    else if (info.porcelain_output)
    {
        print_porcelain_output(files, output);
    }
    else
    {
        print_verbose_output(info, files, output);
    }

    return EXIT_SUCCESS;
}

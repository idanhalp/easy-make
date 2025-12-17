#include "source/commands/list_configurations/list_configurations.hpp"

#include <algorithm>
#include <print>
#include <ranges>
#include <string>

#include "source/commands/build/configuration_resolution.hpp"
#include "source/parameters/parameters.hpp"
#include "source/utils/macros/assert.hpp"

static auto
get_relevant_configuration_names(const ListConfigurationsCommandInfo& info,
                                 const std::vector<Configuration>& configurations) -> std::vector<std::string>
{
    std::vector<std::string> relevant_configuration_names;
    const auto configuration_type = info.complete_configurations_only     ? ConfigurationType::COMPLETE
                                    : info.incomplete_configurations_only ? ConfigurationType::INCOMPLETE
                                                                          : ConfigurationType::ALL;

    for (auto& configuration : get_resolved_configurations(configurations, configuration_type))
    {
        relevant_configuration_names.push_back(std::move(*configuration.name));
    }

    return relevant_configuration_names;
}

static auto print_porcelain_output(const std::vector<std::string>& configuration_names, std::ostream& output) -> void
{
    for (const auto& configuration_name : configuration_names)
    {
        std::println(output, "{}", configuration_name);
    }
}

static auto print_verbose_output(const ListConfigurationsCommandInfo& info,
                                 const std::vector<std::string>& configuration_names,
                                 std::ostream& output) -> void
{
    const auto description = info.complete_configurations_only     ? "complete "
                             : info.incomplete_configurations_only ? "incomplete "
                                                                   : "";

    switch (configuration_names.size())
    {
    case 0:
        std::println(output,
                     "There are no {}configurations in the '{}' file.",
                     description,
                     params::CONFIGURATIONS_FILE_NAME.native());
        break;

    case 1:
        std::println(output,
                     "There is 1 {}configuration in the '{}' file:",
                     description,
                     params::CONFIGURATIONS_FILE_NAME.native());
        break;

    default:
        std::println(output,
                     "There are {} {}configurations in the '{}' file:",
                     configuration_names.size(),
                     description,
                     params::CONFIGURATIONS_FILE_NAME.native());
        break;
    }

    for (const auto [index, name] : std::views::enumerate(configuration_names) | std::views::as_const)
    {
        std::println(output, "{} {}", index + 1, name);
    }
}

auto commands::list_configurations(const ListConfigurationsCommandInfo& info,
                                   const std::vector<Configuration>& configurations,
                                   std::ostream& output) -> int
{
    // If both flags are set then all configurations are irrelevant.
    ASSERT(!info.complete_configurations_only || !info.incomplete_configurations_only);

    // If only the configuration count is printed,
    // formatting the output using porcelain and sorting is meaningless.
    ASSERT(!info.count || !info.porcelain_output);
    ASSERT(!info.count || !info.sorted_output);

    auto relevant_configuration_names = get_relevant_configuration_names(info, configurations);

    if (info.sorted_output)
    {
        std::ranges::sort(relevant_configuration_names);
    }

    if (info.count)
    {
        std::println(output, "{}", relevant_configuration_names.size());
    }
    else if (info.porcelain_output)
    {
        print_porcelain_output(relevant_configuration_names, output);
    }
    else
    {
        print_verbose_output(info, relevant_configuration_names, output);
    }

    return EXIT_SUCCESS;
}

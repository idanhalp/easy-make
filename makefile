CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3 -I.

# Executables
TARGETS = easy-make test

# Source files
SOURCE_FILES = \
    source/argument_parsing/argument_parsing.cpp \
    source/argument_parsing/error_formatting.cpp \
    source/argument_parsing/commands/build.cpp \
    source/argument_parsing/commands/clean.cpp \
    source/argument_parsing/commands/clean_all.cpp \
    source/argument_parsing/commands/init.cpp \
    source/argument_parsing/commands/list_configurations.cpp \
    source/argument_parsing/commands/list_files.cpp \
    source/argument_parsing/commands/print_version.cpp \
    source/argument_parsing/utils.cpp \
    source/commands/build/build_caching/build_caching.cpp \
    source/commands/build/build_caching/dependency_graph.cpp \
    source/commands/build/compilation/compilation.cpp \
    source/commands/build/build.cpp \
    source/commands/build/linking.cpp \
    source/commands/list_configurations/list_configurations.cpp \
    source/commands/list_files/list_files.cpp \
    source/commands/clean/clean.cpp \
    source/commands/clean_all/clean_all.cpp \
    source/commands/init/init.cpp \
    source/commands/print_version/print_version.cpp \
    source/configuration_parsing/configuration.cpp \
    source/configuration_parsing/configuration_parsing.cpp \
    source/configuration_parsing/json_keys.cpp \
    source/main.cpp \
    source/utils/find_closest_word.cpp \
    source/utils/utils.cpp

TEST_FILES = \
    tests/main.cpp \
    tests/test_argument_parsing.cpp \
    tests/test_build_caching.cpp \
    tests/test_check_if_configuration_file_exists.cpp \
    tests/test_clean.cpp \
    tests/test_configuration_parsing.cpp \
    tests/test_dependency_graph.cpp \
    tests/test_build.cpp \
    tests/test_find_closest_word.cpp \
    tests/test_graph.cpp \
    tests/test_utils.cpp \
    tests/utils/utils.cpp

# Object files
EASY_MAKE_OBJS = $(SOURCE_FILES:.cpp=.o)
# Exclude source/main.o from test build (tests/main.cpp is used instead)
TESTS_OBJS = $(filter-out source/main.o, $(SOURCE_FILES:.cpp=.o)) $(TEST_FILES:.cpp=.o)

# Default target
all: $(TARGETS)

# Build easy-make
easy-make: $(EASY_MAKE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build test executable
test: $(TESTS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Generic rule for compiling .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean objects and binaries
clean:
	rm -f $(EASY_MAKE_OBJS) $(TESTS_OBJS) $(TARGETS)

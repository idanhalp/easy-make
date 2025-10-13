CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O0 -I.

# Executables
TARGETS = tests easy-make

# Sources
TESTS_SRCS = \
	tests/main.cpp\
	tests/test_argument_parsing.cpp\
	tests/test_build_caching.cpp\
	tests/test_check_if_configuration_file_exists.cpp\
	tests/test_clean.cpp\
	tests/test_configuration_parsing.cpp\
	tests/test_executable_creation.cpp\
	tests/test_find_closest_word.cpp\
	tests/test_graph.cpp\
	tests/test_utils.cpp\
	tests/utils/utils.cpp\
	source/argument_parsing/argument_parsing.cpp\
	source/build_caching/build_caching.cpp\
	source/build_caching/dependency_graph.cpp\
	source/commands/clean/clean.cpp\
	source/commands/clean_all/clean_all.cpp\
	source/commands/print_version/print_version.cpp\
	source/configuration_parsing/configuration.cpp\
	source/configuration_parsing/configuration_parsing.cpp\
	source/configuration_parsing/json_keys.cpp\
	source/executable_creation/executable_creation.cpp\
	source/utils/find_closest_word.cpp\
	source/utils/utils.cpp

EASYMAKE_SRCS = \
	source/main.cpp \
	source/argument_parsing/argument_parsing.cpp\
	source/build_caching/build_caching.cpp\
	source/build_caching/dependency_graph.cpp\
	source/commands/clean/clean.cpp\
	source/commands/clean_all/clean_all.cpp\
	source/commands/print_version/print_version.cpp\
	source/configuration_parsing/configuration.cpp\
	source/configuration_parsing/configuration_parsing.cpp\
	source/configuration_parsing/json_keys.cpp\
	source/executable_creation/executable_creation.cpp\
	source/utils/find_closest_word.cpp\
	source/utils/utils.cpp

# Objects
TESTS_OBJS = $(TESTS_SRCS:.cpp=.o)
EASYMAKE_OBJS = $(EASYMAKE_SRCS:.cpp=.o)

all: $(TARGETS)

run-tests: $(TESTS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

easy-make: $(EASYMAKE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TESTS_OBJS) $(EASYMAKE_OBJS) $(TARGETS)

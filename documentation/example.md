# Example

This section demonstrates using **easy-make** to compile **easy-make** itself.

## Listing Complete Configurations

You can look at the [easy-make-configurations.json](/easy-make-configurations.json) file to see how configurations are defined.

We can check which configurations are available using the `list-configs` command with the `--complete-only` flag, as incomplete configurations cannot be compiled:

```
$ easy-make list-configs --complete-only

There are 4 complete configurations in the 'easy-make-configurations.json' file:
1 test
2 easy-make
3 debug
4 release
```

## Counting Files

Let us build the `debug` configuration. We can check how many files it contains using the `list-files` command with the `--count` flag:

```
$ easy-make list-files debug --count

53
```

Since we don't need to compile header files, let us check how many source files must be compiled using the `--source-only` flag:

```
$ easy-make list-files debug --count --source-only

24
```

## Cleaning Previous Outputs

We can ensure no old object files are used by running:

```
$ easy-make clean-all

Removed 'easy-make-build'.
Removed '/home/idanhalp/projects/easy-make/easy-make'.
```

Both the object files and the executable were removed.

## Building the Project

Now we can compile the project using the `debug` configuration:

```
$ easy-make build debug

Compiling 24 files...
 1/24 [  4%] source/argument_parsing/argument_parsing.cpp
 2/24 [  8%] source/argument_parsing/commands/build.cpp
 3/24 [ 12%] source/argument_parsing/commands/clean.cpp
 4/24 [ 16%] source/argument_parsing/commands/clean_all.cpp
 5/24 [ 20%] source/argument_parsing/commands/list_configurations.cpp
 6/24 [ 25%] source/argument_parsing/commands/list_files.cpp
 7/24 [ 29%] source/argument_parsing/commands/print_version.cpp
 8/24 [ 33%] source/argument_parsing/utils.cpp
 9/24 [ 37%] source/commands/build/build.cpp
10/24 [ 41%] source/commands/build/build_caching/build_caching.cpp
11/24 [ 45%] source/commands/build/build_caching/dependency_graph.cpp
12/24 [ 50%] source/commands/build/compilation.cpp
13/24 [ 54%] source/commands/build/linking.cpp
14/24 [ 58%] source/commands/clean/clean.cpp
15/24 [ 62%] source/commands/clean_all/clean_all.cpp
16/24 [ 66%] source/commands/list_configurations/list_configurations.cpp
17/24 [ 70%] source/commands/list_files/list_files.cpp
18/24 [ 75%] source/commands/print_version/print_version.cpp
19/24 [ 79%] source/configuration_parsing/configuration.cpp
20/24 [ 83%] source/configuration_parsing/configuration_parsing.cpp
21/24 [ 87%] source/configuration_parsing/json_keys.cpp
22/24 [ 91%] source/main.cpp
23/24 [ 95%] source/utils/find_closest_word.cpp
24/24 [100%] source/utils/utils.cpp
Compilation complete.
Linking...
Linking complete. Executable located at './easy-make'.
```

## Incremental Compilation

Suppose we modify `source/parameters/parameters.hpp` to add a new value in the `params` namespace. Since **easy-make** tracks dependencies, only affected files will be recompiled:

```
$ easy-make build debug

Compiling 11 files...
 1/11 [  9%] source/commands/build/build.cpp
 2/11 [ 18%] source/commands/build/build_caching/build_caching.cpp
 3/11 [ 27%] source/commands/build/compilation.cpp
 4/11 [ 36%] source/commands/build/linking.cpp
 5/11 [ 45%] source/commands/clean/clean.cpp
 6/11 [ 54%] source/commands/clean_all/clean_all.cpp
 7/11 [ 63%] source/commands/list_configurations/list_configurations.cpp
 8/11 [ 72%] source/configuration_parsing/configuration.cpp
 9/11 [ 81%] source/configuration_parsing/configuration_parsing.cpp
10/11 [ 90%] source/main.cpp
11/11 [100%] source/utils/utils.cpp
Compilation complete.
Linking...
Linking complete. Executable located at './easy-make'.
```

## Summary

This example shows:

- Listing configurations
- Counting files
- Cleaning builds
- Full compilation
- Incremental recompilation based on dependency tracking

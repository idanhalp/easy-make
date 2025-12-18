# Example

This section demonstrates using **easy-make** to compile **easy-make** itself.

## Listing Configurations

You can look at the [easy-make-configurations.json](/easy-make-configurations.json) file to see how configurations are defined.

We can check which configurations are available using the `list-configs` command:

```
$ easy-make list-configs

There are 5 configurations in the 'easy-make-configurations.json' file:
1 default
2 test
3 easy-make
4 debug
5 release
```

Since we only care about configurations that we can compile, let us rerun the same command with the `--complete-only` flag:

```
$ easy-make list-configs --complete-only

There are 3 complete configurations in the 'easy-make-configurations.json' file:
1 test
2 debug
3 release
```

## Counting Files

Let us build the `debug` configuration. We can check how many files it contains using the `list-files` command with the `--count` flag:

```
$ easy-make list-files debug --count

62
```

Since we don't need to compile header files, let us check how many source files must be compiled using the `--source-only` flag:

```
$ easy-make list-files debug --count --source-only

28
```

Now we know that we will have to compile 28 source files.

## Cleaning Previous Outputs

Let us remove the old object files and executable (if they exist).

```
$ easy-make clean debug
```

## Building the Project

Now we can compile the project by using the `build` command.
In order to hasten the process let us use the `--parallel` flag to use several threads for the compilation:

```
$ easy-make build debug --parallel

Compiling 28 files for configuration 'debug'...
 1/28 [  3%] source/argument_parsing/argument_parsing.cpp
 2/28 [  7%] source/argument_parsing/commands/build.cpp
 3/28 [ 10%] source/argument_parsing/commands/clean.cpp
 4/28 [ 14%] source/argument_parsing/commands/clean_all.cpp
 5/28 [ 17%] source/argument_parsing/commands/init.cpp
 6/28 [ 21%] source/argument_parsing/commands/list_configurations.cpp
 7/28 [ 25%] source/argument_parsing/commands/list_files.cpp
 8/28 [ 28%] source/argument_parsing/commands/print_version.cpp
 9/28 [ 32%] source/argument_parsing/error_formatting.cpp
10/28 [ 35%] source/argument_parsing/utils.cpp
11/28 [ 39%] source/commands/build/build.cpp
12/28 [ 42%] source/commands/build/build_caching/build_caching.cpp
13/28 [ 46%] source/commands/build/build_caching/dependency_graph.cpp
14/28 [ 50%] source/commands/build/compilation/compilation.cpp
15/28 [ 53%] source/commands/build/linking.cpp
16/28 [ 57%] source/commands/clean/clean.cpp
17/28 [ 60%] source/commands/clean_all/clean_all.cpp
18/28 [ 64%] source/commands/init/init.cpp
19/28 [ 67%] source/commands/list_configurations/list_configurations.cpp
20/28 [ 71%] source/commands/list_files/list_files.cpp
21/28 [ 75%] source/commands/print_version/print_version.cpp
22/28 [ 78%] source/configuration_parsing/configuration_parsing.cpp
23/28 [ 82%] source/configuration_parsing/json_keys.cpp
24/28 [ 85%] source/configuration_parsing/structure_validation.cpp
25/28 [ 89%] source/configuration_parsing/value_validation.cpp
26/28 [ 92%] source/main.cpp
27/28 [ 96%] source/utils/find_closest_word.cpp
28/28 [100%] source/utils/utils.cpp
Compilation complete.
Linking...
Linking complete. Executable located at './easy-make-debug'.
```

## Incremental Compilation

Suppose we modify `source/parameters/parameters.hpp` to add a new value in the `params` namespace. Since **easy-make** tracks dependencies, only affected files will be recompiled:

```
$ easy-make build debug

Compiling 12 files for configuration 'debug'...
 1/12 [  8%] source/commands/build/build.cpp
 2/12 [ 16%] source/commands/build/build_caching/build_caching.cpp
 3/12 [ 25%] source/commands/build/compilation/compilation.cpp
 4/12 [ 33%] source/commands/build/linking.cpp
 5/12 [ 41%] source/commands/clean/clean.cpp
 6/12 [ 50%] source/commands/clean_all/clean_all.cpp
 7/12 [ 58%] source/commands/init/init.cpp
 8/12 [ 66%] source/commands/list_configurations/list_configurations.cpp
 9/12 [ 75%] source/configuration_parsing/configuration_parsing.cpp
10/12 [ 83%] source/configuration_parsing/value_validation.cpp
11/12 [ 91%] source/main.cpp
12/12 [100%] source/utils/utils.cpp
Compilation complete.
Linking...
Linking complete. Executable located at './easy-make-debug'.
```

Instead of compiling all the 28 files, only the relevant 12 were recompiled.

## Summary

This example shows:

- Listing configurations
- Counting files
- Cleaning builds
- Full compilation
- Incremental recompilation based on dependency tracking

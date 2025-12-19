# `build` Command Documentation

## Summary

Creates executables for one or more configurations.

## Usage

    easy-make build <configuration-name> [options]
    easy-make build --all [options]

## Behavior

- `easy-make build <configuration-name>`  
  Creates the executable defined by the given configuration.  
  The configuration must be complete (see
  [definition](../easy-make-configurations-reference.md#note---complete-configurations)).

- `easy-make build --all`  
  Builds executables for all complete configurations.

## Options

- `--all`  
  Build all complete configurations.

- `--parallel`  
  Enable parallel compilation of source files.  
  The number of threads is chosen automatically.

- `--quiet`  
  Suppress non-essential output such as:

  - current configuration name
  - compilation progress
  - success messages

  Compiler warnings and errors are still printed.

## Exit Status

- `0`  
  The build completed successfully.

- `1`  
  The build failed. Possible reasons include:
  - Invalid arguments were supplied.
  - Invalid or incomplete configuration.
  - Circular include dependencies.
  - Compilation errors.
  - Linker errors.

## Examples

```
easy-make build debug
easy-make build release --quiet --parallel
easy-make build --all --parallel
```

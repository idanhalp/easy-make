# `clean-all` Command Documentation

## Summary

Removes all build artifacts, including the build directory and generated executables for all configurations.

## Usage

```
easy-make clean-all [options]
```

## Behavior

- Removes the global build directory (e.g. `build/`) from the project root.
- Iterates over all configurations and removes their generated executables, if present.
- Executables are removed only for complete configurations ([see definition](../easy-make-configurations-reference.md#note---complete-configurations)).
- Status messages are printed for each removed item unless `--quiet` is specified.
- If the build directory does not exist, the command fails.

## Options

- `--quiet`  
  Suppress informational output.

## Exit Status

- `0`  
  The build directory existed and was successfully removed.

- `1`  
  The command failed because of one of the following reasons:
  - Invalid arguments were supplied.
  - The build directory did not exist.

## Examples

```
easy-make clean-all
easy-make clean-all --quiet
```

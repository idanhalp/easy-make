# `clean` Command Documentation

## Summary

Removes the executable and object files of a specified configuration.

## Usage

```
easy-make clean <configuration-name> [options]
```

## Behavior

- Resolves the specified configuration by name.
- If the configuration does not exist or is invalid, the command fails.
- If the configuration is valid:
  - Removes the directory containing its object files, if it exists.
  - Removes the generated executable, if it exists.
- If neither object files nor the executable exist, the command fails.

## Options

- `--quiet`  
  Suppress error messages and informational output.

## Exit Status

- `0`  
  At least one object file directory or executable was successfully removed.

- `1`  
  The command failed due to one of the following reasons:
  - Invalid arguments were supplied.
  - The specified configuration does not exist or is invalid.
  - There were no object files or executable to remove.
  - No files were removed while running in quiet mode.

## Examples

```
easy-make clean debug
easy-make clean debug --quiet
```
